/*
 * Copyright (C) 2023 Emeric Poupon
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

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

class ICommand
{
	public:
		virtual ~ICommand() = default;

		virtual std::string_view getName() const = 0;
		virtual std::string_view getDescription() const = 0;
		virtual void displayHelp(std::ostream& os) const = 0;
		virtual int process(const std::vector<std::string>& args) const = 0;
};
