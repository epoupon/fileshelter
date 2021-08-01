#include "ShareManager.hpp"

#include <Wt/WLocalDateTime.h>
#include "share/Exception.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "utils/Zipper.hpp"
#include "File.hpp"
#include "Share.hpp"

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
			fileDesc.clientPath = file->getClientPath();
			fileDesc.size = file->getSize();

			desc.files.emplace_back(std::move(fileDesc));
		});

		return desc;
	}

	Share::Share::pointer
	getShareByUUIDAndPassword(Wt::Dbo::Session& session, const ShareUUID& shareUUID, std::optional<std::string_view> password)
	{
		const Share::Share::pointer share {Share::Share::getByUUID(session, shareUUID)};
		if (!share)
			throw ShareNotFoundException {};

		// TODO check password

		return share;
	}
}

namespace Share
{
	std::unique_ptr<IShareManager>
	createShareManager(const std::filesystem::path& dbFile)
	{
		return std::make_unique<ShareManager>(dbFile);
	}

	ShareManager::ShareManager(const std::filesystem::path& dbFile)
	: _db {dbFile}
	, _shareMaxSize {Service<IConfig>::get()->getULong("max-share-size", 100) * 1024 * 1024}
	, _fileMaxSize {Service<IConfig>::get()->getULong("max-file-size", 100) * 1024 * 1024}
	, _maxValidityDuration {std::chrono::hours {24} * Service<IConfig>::get()->getULong("max-validity-days", 100)}
	, _defaultValidityDuration {std::chrono::hours {24} * Service<IConfig>::get()->getULong("default-validity-days", 100)}
	, _canValidatityDurationBeSet {Service<IConfig>::get()->getBool("user-defined-validy-days", true)}
	{
		// config validation
		if (_shareMaxSize == 0)
			throw Exception {"max-share-size must be greater than 0"};
		if (_fileMaxSize == 0)
			throw Exception {"max-file-size must be greater than 0"};
		if (_maxValidityDuration.count() == 0)
			throw Exception {"max-validity-days must be greater than 0"};
		if (_defaultValidityDuration.count() == 0)
			throw Exception {"default-validity-days must be greater than 0"};

		if (_maxValidityDuration < _defaultValidityDuration)
			throw Exception {"max-validity-days must be greater than default-validity-days"};
	}

	ShareEditUUID
	ShareManager::createShare(const ShareCreateParameters& shareParameters, const std::vector<FileCreateParameters>& filesParameters, bool transferFileOwnership)
	{
		FS_LOG(SHARE, DEBUG) << "Creating share! nb files = " << filesParameters.size();

		// TODO validate file sizes

		// TODO hash password

		{
			Wt::Dbo::Session& session {_db.getTLSSession()};
			Wt::Dbo::Transaction transaction {session};

			Share::pointer share {Share::create(session, shareParameters)};
			share.modify()->setUUID(UUID::Generate {});
			share.modify()->setEditUUID(UUID::Generate {});

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
	ShareManager::destroyShare(const ShareEditUUID& shareUUID)
	{
		throw Exception {"Not implemented"};
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
		Wt::Dbo::Transaction transaction {session};

		const Share::pointer share {getShareByUUIDAndPassword(session, shareUUID, password)};
		return shareToDesc(*share.get());
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

	std::unique_ptr<Zip::Zipper>
	ShareManager::getShareZipper(const ShareUUID& shareUUID, std::optional<std::string_view> password)
	{
		try
		{
			Wt::Dbo::Session& session {_db.getTLSSession()};
			Wt::Dbo::Transaction transaction {session};

			Share::pointer share {getShareByUUIDAndPassword(session, shareUUID, password)};

			std::map<std::string, std::filesystem::path> zipFiles;
			share->visitFiles([&](const File::pointer& file)
			{
				zipFiles.emplace(file->getClientPath(), file->getPath());
			});

			share.modify()->incReadCount();

			// mask creation time
			return std::make_unique<Zip::Zipper>(zipFiles, Wt::WLocalDateTime::currentDateTime().toUTC());
		}
		catch (const Zip::ZipperException& e)
		{
			FS_LOG(UI, DEBUG) << "Zip exception: " << e.what();
			throw Exception {"zip exception: " + std::string {e.what()}};
		}
	}

} // namespace Share
