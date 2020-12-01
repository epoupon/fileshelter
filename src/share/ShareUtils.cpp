/*
 * Copyright (C) 2020 Emeric Poupon
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

#include "ShareUtils.hpp"

#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/WLocalDateTime.h>

#include "database/File.hpp"
#include "database/Share.hpp"

#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"

using namespace Database;

namespace ShareUtils
{

	std::size_t
	getMaxShareSize()
	{
		return Config::instance().getULong("max-file-size", 100) * 1024 * 1024;
	}

	std::filesystem::path
	getFilesPath()
	{
		return Config::instance().getPath("working-dir") / "files";
	}

	std::chrono::seconds
	getMaxValidatityDuration()
	{
		const auto durationDay =  std::chrono::hours(24);
		auto maxDuration = durationDay * Config::instance().getULong("max-validity-days", 100);

		if (durationDay > maxDuration)
			maxDuration = durationDay;

		return maxDuration;
	}

	std::chrono::seconds
	getDefaultValidatityDuration()
	{
		const auto durationDay =  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(24));
		auto defaultDuration = std::chrono::duration_cast<std::chrono::seconds>(durationDay * Config::instance().getULong("default-validity-days", 7));
		auto maxDuration = getMaxValidatityDuration();

		if (defaultDuration < durationDay)
			defaultDuration = durationDay;

		if (defaultDuration > maxDuration)
			defaultDuration = maxDuration;

		return defaultDuration;
	}

	bool
	canValidatityDurationBeSet()
	{
		return Config::instance().getBool("user-defined-validy-days", true);
	}


	std::size_t
	getMaxValidatityHits()
	{
		return Config::instance().getULong("max-validity-hits", 0);
	}

	bool
	addFileToShare(Wt::Dbo::Session& session,
			IdType shareId,
			const std::filesystem::path& path,
			std::string_view fileName)
	{
		const std::string storeName {UUID::generate().getAsString()};
		const std::filesystem::path storePath {getFilesPath() / storeName};

		std::error_code ec;
		std::filesystem::rename(path, storePath, ec);
		if (ec)
		{
			FS_LOG(SHARE, ERROR) << "Move file failed from '" << path.string() << "' to '" << storePath.string() << "': " << ec.message();
			return false;
		}

		const auto fileSize {std::filesystem::file_size(storePath, ec)};
		if (ec)
		{
			FS_LOG(SHARE, ERROR) << "Cannot get file size for '" << storePath.string() << "': " << ec.message();
			return false;
		}

		{
			Wt::Dbo::Transaction transaction {session};

			Share::pointer share {Share::getById(session, shareId)};
			if (!share)
				return false;

			assert(share->getState() == Share::State::Init);

			Database::File::create(session,
									fileName,
									fileSize,
									storePath,
									UUID::generate(),
									share);
		}

		return true;
	}

	void
	destroyShare(Wt::Dbo::Session& session, IdType shareId)
	{
		std::vector<IdType> fileIds;

		{
			Wt::Dbo::Transaction transaction {session};
			Share::pointer share {Share::getById(session, shareId)};
			if (!share)
				return;

			fileIds = share->getFileIds();
			share.modify()->setState(Share::State::Deleting);
		}


		for (const IdType fileId : fileIds)
		{
			std::filesystem::path filePath;
			{
				Wt::Dbo::Transaction transaction(session);
				const File::pointer file {File::getById(session, fileId)};
				if (!file)
					continue;

				filePath = file->getPath();
			}


			std::error_code ec;
			std::filesystem::remove(filePath, ec);
			if (ec)
				FS_LOG(SHARE, ERROR) << "Cannot remove file: '" << filePath.string() << "' from share! Error:" << ec.message();
		}

		{
			Wt::Dbo::Transaction transaction {session};

			Share::pointer share {Share::getById(session, shareId)};
			if (!share)
				return;

			share.remove();
		}
	}

	void
	destroyExpiredShares(Wt::Dbo::Session& session)
	{
		const auto now {Wt::WLocalDateTime::currentServerDateTime().toUTC()};

		FS_LOG(SHARE, INFO) << "Cleaning expired shares...";

		std::vector<IdType> expiredShareIds;

		{
			Wt::Dbo::Transaction transaction {session};

			auto shares {Database::Share::getAll(session)};
			shares.erase(std::remove_if(std::begin(shares), std::end(shares), [&](const Share::pointer& share)
					{
						// In order not to delete a share that is being downloaded,
						// really remove the share at least a day after it has expired
						return now < share->getExpiryTime().addDays(1);
					}),
					std::cend(shares));

			std::transform(std::cbegin(shares), std::cend(shares), std::back_inserter(expiredShareIds),
					[&](const Share::pointer& share)
					{
						return share.id();
					});
		}

		for (const IdType expiredShareId : expiredShareIds)
			destroyShare(session, expiredShareId);

		FS_LOG(SHARE, INFO) << "Cleaned expired shares!";
	}

	std::string
	computeFileName(const Share::pointer& share)
	{
		const auto files {share->getFiles()};
		return files.size() == 1 ? files.front()->getName() : share->getDownloadUUID().getAsString() + ".zip";
	}

	Wt::Auth::PasswordHash
	computePasswordHash(const Wt::WString& password)
	{
		auto hashFunc {std::make_unique<Wt::Auth::BCryptHashFunction>(Config::instance().getULong("bcrypt-count", 12))};

		Wt::Auth::PasswordVerifier verifier;
		verifier.addHashFunction(std::move(hashFunc));

		return verifier.hashPassword(password);
	}

	bool
	checkPassword(const Wt::Auth::PasswordHash& hash, const Wt::WString& password)
	{
		auto hashFunc {std::make_unique<Wt::Auth::BCryptHashFunction>(Config::instance().getULong("bcrypt-count", 12))};
		Wt::Auth::PasswordVerifier verifier;
		verifier.addHashFunction(std::move(hashFunc));

		return verifier.verify(password, hash);
	}

	bool
	isShareAvailableForDownload(const Share::pointer& share)
	{
		const auto now {Wt::WLocalDateTime::currentServerDateTime().toUTC()};

		if (share->getState() != Share::State::Ready)
			return false;

		// Make also sure all files are present and have not changed (at least in size)
		for (const auto& file : share->getFiles())
		{
			std::error_code ec;
			bool isRegular {std::filesystem::is_regular_file(file->getPath(), ec)};
			if (ec)
			{
				FS_LOG(SHARE, ERROR) << "Cannot get info on file '" << file->getPath().string() << "': " << ec.message();
				return false;
			}
			if (!isRegular)
			{
				FS_LOG(SHARE, ERROR) << "File '" << file->getPath().string() << "' is not regular";
				return false;
			}
			const auto fileSize {std::filesystem::file_size(file->getPath(), ec)};
			if (ec)
			{
				FS_LOG(SHARE, ERROR) << "Cannot get file size on file '" << file->getPath().string() << "': " << ec.message();
				return false;
			}
			if (fileSize != file->getSize())
			{
				FS_LOG(SHARE, ERROR) << "File mismatch on file '" << file->getPath().string() << "'";
				return false;
			}
		}

		return now < share->getExpiryTime();
	}
}

