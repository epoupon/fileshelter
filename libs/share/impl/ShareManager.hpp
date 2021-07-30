#pragma once

#include "share/IShareManager.hpp"

#include "Db.hpp"

namespace Share
{

	class ShareManager : public IShareManager
	{
		public:
			ShareManager(const std::filesystem::path& dbFile);

		private:
			FileSize				getMaxShareSize() const override;
			FileSize				getMaxFileSize() const override;
			std::chrono::seconds	getMaxValidatityDuration() const override;
			std::chrono::seconds	getDefaultValidatityDuration() const override;
			std::size_t				getMaxValidatityHits() const override;
			bool					canValidatityDurationBeSet() const override;

			ShareEditUUID	createShare(const ShareCreateParameters& share, const std::vector<FileCreateParameters>& files, bool transferFileOwnership) override;
			void			destroyShare(const ShareEditUUID& shareId) override;
			bool			shareHasPassword(const ShareUUID& shareId) override;
			ShareDesc		getShareDesc(const ShareUUID& shareId, std::optional<std::string_view> password) override;
			ShareDesc		getShareDesc(const ShareEditUUID& shareId) override;
			std::unique_ptr<Zip::Zipper> getShareZipper(const ShareUUID& shareId, std::optional<std::string_view> password) override;

			Db _db;
	};

} // namespace Share
