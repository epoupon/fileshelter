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
#include <string_view>

#include <Wt/Auth/PasswordHash.h>
#include <Wt/WString.h>

#include "database/Types.hpp"

namespace Database
{
	class Share;
}

namespace ShareUtils
{
	Database::FileSize		getMaxShareSize();
	std::chrono::seconds	getMaxValidatityDuration();
	std::chrono::seconds	getDefaultValidatityDuration();
	std::size_t				getMaxValidatityHits();
	bool					canValidatityDurationBeSet();

	void destroyShare(Wt::Dbo::Session& session, Database::IdType shareId);
	void destroyExpiredShares(Wt::Dbo::Session& session);

	// A transaction must be active
	bool moveFileToShare(Wt::Dbo::Session& session,
								Database::IdType shareId,
								const std::filesystem::path& filePath,
								std::string_view fileName);
	Wt::Auth::PasswordHash	computePasswordHash(const Wt::WString& password);
	bool					checkPassword(const Wt::Auth::PasswordHash& hash, const Wt::WString& password);
	bool		isShareAvailableForDownload(const Wt::Dbo::ptr<Database::Share>& share);
	std::string	computeFileName(const Wt::Dbo::ptr<Database::Share>& share);
}

