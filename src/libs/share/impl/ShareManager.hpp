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

#pragma once

#include <Wt/Auth/PasswordVerifier.h>

#include "Db.hpp"
#include "share/IShareManager.hpp"

namespace Share
{
    class ShareCleaner;

    class ShareManager : public IShareManager
    {
    public:
        ShareManager(bool enableCleaner);
        ~ShareManager();

        ShareManager(const ShareManager&) = delete;
        ShareManager(ShareManager&&) = delete;
        ShareManager& operator=(const ShareManager&) = delete;
        ShareManager& operator=(ShareManager&&) = delete;

    private:
        FileSize getMaxShareSize() const override { return _maxShareSize; }
        std::chrono::seconds getMaxValidityPeriod() const override { return _maxValidityPeriod; }
        std::chrono::seconds getDefaultValidityPeriod() const override { return _defaultValidityPeriod; }
        std::size_t getMaxValidatityHits() const override { return _maxValidityHits; }
        bool canValidityPeriodBeSet() const override { return _canValidityPeriodBeSet; }

        ShareDesc createShare(const ShareCreateParameters& share, const std::vector<FileCreateParameters>& files, bool transferFileOwnership) override;
        void destroyShare(const ShareEditUUID& shareUUID) override;
        bool shareHasPassword(const ShareUUID& shareUUID) override;
        ShareDesc getShareDesc(const ShareUUID& shareUUID, std::optional<std::string_view> password) override;
        ShareDesc getShareDesc(const ShareEditUUID& shareUUID) override;
        void visitShares(std::function<void(const ShareDesc&)>) override;
        void incrementReadCount(const ShareUUID& shareUUID) override;
        void removeOrphanFiles(const std::filesystem::path& directory) override;

        void validateShareSizes(const std::vector<FileCreateParameters>& files, const std::vector<FileSize>& fileSizes);

        const std::filesystem::path _workingDirectory;
        Db _db;

        std::unique_ptr<ShareCleaner> _shareCleaner;
        Wt::Auth::PasswordVerifier _passwordVerifier;

        const FileSize _maxShareSize{};
        const std::chrono::seconds _maxValidityPeriod{};
        const std::chrono::seconds _defaultValidityPeriod{};
        const std::size_t _maxValidityHits{};
        const bool _canValidityPeriodBeSet{};
    };

} // namespace Share
