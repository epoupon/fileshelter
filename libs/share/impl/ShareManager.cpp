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
	{

	}

	FileSize
	ShareManager::getMaxShareSize() const
	{
		return Service<IConfig>::get()->getULong("max-share-size", 100) * 1024 * 1024;
	}

	FileSize
	ShareManager::getMaxFileSize() const
	{
		return Service<IConfig>::get()->getULong("max-file-size", 100) * 1024 * 1024;
	}

	std::chrono::seconds
	ShareManager::getMaxValidatityDuration() const
	{
		return std::chrono::hours {24} * Service<IConfig>::get()->getULong("max-validity-days", 100);
	}

	std::chrono::seconds
	ShareManager::getDefaultValidatityDuration() const
	{
		return {};
	}

	std::size_t
	ShareManager::getMaxValidatityHits() const
	{
		return 0;
	}

	bool
	ShareManager::canValidatityDurationBeSet() const
	{
		return true;
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
	ShareManager::destroyShare(const ShareEditUUID& shareId)
	{
		throw Exception {"Not implemented"};
	}

	ShareDesc
	ShareManager::getShareDesc(const ShareUUID& shareId, std::optional<std::string_view> password)
	{
		Wt::Dbo::Session& session {_db.getTLSSession()};
		Wt::Dbo::Transaction transaction {session};

		const Share::pointer share {getShareByUUIDAndPassword(session, shareId, password)};
		return shareToDesc(*share.get());
	}

	std::unique_ptr<Zip::Zipper>
	ShareManager::getShareZipper(const ShareUUID& shareId, std::optional<std::string_view> password)
	{
		try
		{
			Wt::Dbo::Session& session {_db.getTLSSession()};
			Wt::Dbo::Transaction transaction {session};

			Share::pointer share {getShareByUUIDAndPassword(session, shareId, password)};

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
