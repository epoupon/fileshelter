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

#include <optional>
#include <string>

#include "utils/String.hpp"

class UUID
{
	public:

		static UUID generate();
		static std::optional<UUID> fromString(std::string_view str);

		const std::string& getAsString() const { return _value; }

		bool operator==(const UUID& other) const
		{
			return _value == other._value;
		}

	private:
		UUID(std::string_view value);
		std::string _value;
};

namespace StringUtils
{
	template <>
	std::optional<UUID>
	readAs(const std::string& str);
}

