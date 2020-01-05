/*
 * Copyright (C) 2019 Emeric Poupon
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

#include "PasswordUtils.hpp"

#include <set>

#include "utils/Config.hpp"

bool
isUploadPassordRequired()
{
	return !Config::instance().getString("upload-password", "").empty()
		|| !Config::instance().getStrings("upload-passwords").empty();
}

bool
checkUploadPassord(const std::string& uploadPassword)
{
	std::set<std::string> configPasswords;

	{
		std::string password {Config::instance().getString("upload-password", "")};

		if (!password.empty())
			configPasswords.insert(std::move(password));
	}

	for (std::string& password : Config::instance().getStrings("upload-passwords"))
		configPasswords.insert(std::move(password));

	return configPasswords.find(uploadPassword) != std::cend(configPasswords);
}

