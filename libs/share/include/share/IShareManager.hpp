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

#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

#include "share/Types.hpp"

namespace Zip
{
	class Zipper;
}

namespace Share
{
	class IShare;

	struct FileCreateParameters;
	struct ShareCreateParameters;

	class IShareManager
	{
		public:
			virtual ~IShareManager() = default;

			virtual FileSize				getMaxShareSize() const = 0;
			virtual FileSize				getMaxFileSize() const = 0;

			virtual std::chrono::seconds	getMaxValidatityDuration() const = 0;
			virtual std::chrono::seconds	getDefaultValidatityDuration() const = 0;
			virtual std::size_t				getMaxValidatityHits() const = 0;
			virtual bool					canValidatityDurationBeSet() const = 0;

			virtual ShareEditUUID			createShare(const ShareCreateParameters& params, const std::vector<FileCreateParameters>& files, bool tranferFilesOwnership) = 0;
			virtual void					destroyShare(const ShareEditUUID& shareId) = 0;
			virtual bool					shareHasPassword(const ShareUUID& shareId) = 0;
			virtual ShareDesc				getShareDesc(const ShareUUID& shareId, std::optional<std::string_view> password = std::nullopt) = 0;
			virtual ShareDesc				getShareDesc(const ShareEditUUID& shareId) = 0;
			virtual	std::unique_ptr<Zip::Zipper> getShareZipper(const ShareUUID& shareId, std::optional<std::string_view> password) = 0;
	};

	std::unique_ptr<IShareManager> createShareManager(const std::filesystem::path& dbFile);

} // namespace Share
