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
			FileSize				getMaxShareSize() const override { return _shareMaxSize; }
			FileSize				getMaxFileSize() const override { return _fileMaxSize; }
			std::chrono::seconds	getMaxValidatityDuration() const override { return _maxValidityDuration; }
			std::chrono::seconds	getDefaultValidatityDuration() const override { return _defaultValidityDuration; }
			std::size_t				getMaxValidatityHits() const override { return _maxValidityHits; }
			bool					canValidatityDurationBeSet() const override { return _canValidatityDurationBeSet; }

			ShareEditUUID	createShare(const ShareCreateParameters& share, const std::vector<FileCreateParameters>& files, bool transferFileOwnership) override;
			void			destroyShare(const ShareEditUUID& shareUUID) override;
			bool			shareHasPassword(const ShareUUID& shareUUID) override;
			ShareDesc		getShareDesc(const ShareUUID& shareUUID, std::optional<std::string_view> password) override;
			ShareDesc		getShareDesc(const ShareEditUUID& shareUUID) override;

			Db _db;

			const FileSize 				_shareMaxSize {};
			const FileSize 				_fileMaxSize {};
			const std::chrono::seconds	_maxValidityDuration {};
			const std::chrono::seconds	_defaultValidityDuration {};
			const std::size_t			_maxValidityHits {};
			const bool					_canValidatityDurationBeSet {};
	};

} // namespace Share
