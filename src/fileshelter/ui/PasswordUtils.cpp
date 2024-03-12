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

#include <unordered_set>

#include "utils/IConfig.hpp"
#include "utils/Service.hpp"

namespace UserInterface::PasswordUtils
{

	bool
	isUploadPassordRequired()
	{
		bool hasStrings {};
		Service<IConfig>::get()->visitStrings("upload-passwords", [&](std::string_view str)
		{
			if (!str.empty())
				hasStrings = true;
		});

		return hasStrings;
	}

	bool
	checkUploadPassord(std::string_view uploadPassword)
	{
		bool res{};
		Service<IConfig>::get()->visitStrings("upload-passwords", [&](std::string_view password)
		{
			if (password == uploadPassword)
				res = true;
		});

		return res;
	}

	bool
	isListPasswordDefined()
	{
		bool hasStrings {};
		Service<IConfig>::get()->visitStrings("list-passwords", [&](std::string_view str)
		{
			if (!str.empty())
				hasStrings = true;
		});

		return hasStrings;
	}

	bool
	checkListPassword(std::string_view listPassword)
	{
		bool res{};
		Service<IConfig>::get()->visitStrings("list-passwords", [&](std::string_view password)
		{
			if (password == listPassword)
				res = true;
		});

		return res;
	}

}
