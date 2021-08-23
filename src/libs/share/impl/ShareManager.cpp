/*
 * Copyright (C) 2021 Emeric Poupon
 *
 * This file is part of fileshelter.
 *
 * fileshelter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fileshelter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fileshelter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Share.hpp"

#include "ShareManager.hpp"

#include <Wt/Auth/HashFunction.h>
#include <Wt/WLocalDateTime.h>
#include "share/Exception.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "File.hpp"
#include "Share.hpp"
#include "ShareCleaner.hpp"

namespace
{
	using namespace Share;

	std::filesystem::path
	getFilesStorePath()
	{
		return Service<IConfig>::get()->getPath("working-dir") / "files";
	}

	ShareDesc
	shareToDesc(const Share::Share& share)
	{
		ShareDesc desc;
		desc.uuid = share.getUUID();
		desc.readCount = share.getReadCount();
		desc.size = share.getShareSize();
		desc.hasPassword = share.hasPassword();
		desc.description = share.getDescription();
		desc.expiryTime = share.getExpiryTime();
		desc.creatorAddress = share.getCreatorAddr();

		share.visitFiles([&](const File::pointer& file)
		{
			Share::FileDesc fileDesc;
			fileDesc.uuid = file->getUUID();
			fileDesc.path = file->getPath();
			fileDesc.clientPath = file->getClientPath();
			fileDesc.size = file->getSize();
			fileDesc.isOwned = file->isOwned();

			desc.files.emplace_back(std::move(fileDesc));
		});

		return desc;
	}
}

namespace Share
{
	std::unique_ptr<IShareManager>
	createShareManager(const std::filesystem::path& dbFile, bool enableCleaner)
	{
		return std::make_unique<ShareManager>(dbFile, enableCleaner);
	}

	ShareManager::ShareManager(const std::filesystem::path& dbFile, bool enableCleaner)
	: _db {dbFile}
	, _shareCleaner {enableCleaner ? std::make_unique<ShareCleaner>(_db) : nullptr}
	, _shareMaxSize {Service<IConfig>::get()->getULong("max-share-size", 100) * 1024 * 1024}
	, _fileMaxSize {Service<IConfig>::get()->getULong("max-file-size", 100) * 1024 * 1024}
	, _maxValidityPeriod {std::chrono::hours {24} * Service<IConfig>::get()->getULong("max-validity-days", 100)}
	, _defaultValidityPeriod {std::chrono::hours {24} * Service<IConfig>::get()->getULong("default-validity-days", 100)}
	, _canValidityPeriodBeSet {Service<IConfig>::get()->getBool("user-defined-validy-days", true)}
	{
		auto hashFunc {std::make_unique<Wt::Auth::BCryptHashFunction>(static_cast<int>(Service<IConfig>::get()->getULong("bcrypt-count", 12)))};
		_passwordVerifier.addHashFunction(std::move(hashFunc));

		// config validation
		if (_shareMaxSize == 0)
			throw Exception {"max-share-size must be greater than 0"};
		if (_fileMaxSize == 0)
			throw Exception {"max-file-size must be greater than 0"};
		if (_maxValidityPeriod.count() == 0)
			throw Exception {"max-validity-days must be greater than 0"};
		if (_defaultValidityPeriod.count() == 0)
			throw Exception {"default-validity-days must be greater than 0"};

		if (_maxValidityPeriod < _defaultValidityPeriod)
			throw Exception {"max-validity-days must be greater than default-validity-days"};

		FS_LOG(SHARE, DEBUG) << "Started share manager";
	}

	ShareManager::~ShareManager()
	{
		FS_LOG(SHARE, DEBUG) << "Stopped share manager";
	}

	ShareEditUUID
	ShareManager::createShare(const ShareCreateParameters& shareParameters, const std::vector<FileCreateParameters>& filesParameters, bool transferFileOwnership)
	{
		FS_LOG(SHARE, DEBUG) << "Creating share! nb files = " << filesParameters.size();

		// TODO validate file sizes

		// TODO validity period!!

		std::optional<Wt::Auth::PasswordHash> passwordHash;
		if (!shareParameters.password.empty())
			passwordHash = _passwordVerifier.hashPassword(shareParameters.password);

		{
			Wt::Dbo::Session& session {_db.getTLSSession()};
			Wt::Dbo::Transaction transaction {session};

			Share::pointer share {Share::create(session, shareParameters)};
			share.modify()->setUUID(UUID::Generate {});
			share.modify()->setEditUUID(UUID::Generate {});
			if (passwordHash)
				share.modify()->setPasswordHash(*passwordHash);

			for (const FileCreateParameters& fileParameters : filesParameters)
			{
				File::pointer file {File::create(session, fileParameters, share)};

				file.modify()->setIsOwned (transferFileOwnership);
				file.modify()->setUUID(UUID::Generate {});
			}

			return share->getEditUUID();
		}
	}

	void
	ShareManager::destroyShare(const ShareEditUUID& shareEditUUID)
	{
		Wt::Dbo::Session& session {_db.getTLSSession()};
		Wt::Dbo::Transaction transaction {session};

		Share::pointer share {Share::getByEditUUID(session, shareEditUUID)};
		if (!share)
			throw ShareNotFoundException {};

		Share::destroy(share);
	}

	bool
	ShareManager::shareHasPassword(const ShareUUID& shareUUID)
	{
		Wt::Dbo::Session& session {_db.getTLSSession()};
		Wt::Dbo::Transaction transaction {session};

		const Share::pointer share {Share::getByUUID(session, shareUUID)};
		if (!share)
			throw ShareNotFoundException {};

		return share->hasPassword();
	}

	ShareDesc
	ShareManager::getShareDesc(const ShareUUID& shareUUID, std::optional<std::string_view> password)
	{
		Wt::Dbo::Session& session {_db.getTLSSession()};

		ShareDesc shareDesc;
		std::optional<Wt::Auth::PasswordHash> passwordHash;

		{
			Wt::Dbo::Transaction transaction {session};

			const Share::Share::pointer share {Share::Share::getByUUID(session, shareUUID)};
			if (!share)
				throw ShareNotFoundException {};

			if ((!share->hasPassword() && password) || (share->hasPassword() && !password))
				throw ShareNotFoundException {};

			if (share->hasPassword())
				passwordHash = share->getPasswordHash();

			shareDesc = shareToDesc(*share.get());
		}

		if (passwordHash)
		{
			if (!_passwordVerifier.verify(std::string {*password}, *passwordHash))
				throw ShareNotFoundException {};
		}

		return shareDesc;
	}

	ShareDesc
	ShareManager::getShareDesc(const ShareEditUUID& shareEditUUID)
	{
		Wt::Dbo::Session& session {_db.getTLSSession()};
		Wt::Dbo::Transaction transaction {session};

		const Share::pointer share {Share::getByEditUUID(session, shareEditUUID)};
		if (!share)
			throw ShareNotFoundException {};

		return shareToDesc(*share.get());
	}

	void
	ShareManager::visitShares(std::function<void(const ShareDesc&)> visitor)
	{
		std::vector<ShareDesc> shares;

		{
			Wt::Dbo::Session& session {_db.getTLSSession()};
			Wt::Dbo::Transaction transaction {session};

			Share::visitAll(session, [&](const Share::pointer& share)
			{
				shares.push_back(shareToDesc(*share.get()));
			});
		}

		for (const ShareDesc& share : shares)
			visitor(share);
	}

} // namespace Share
