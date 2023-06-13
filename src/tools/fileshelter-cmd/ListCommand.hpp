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

#include <boost/program_options.hpp>
#include "ICommand.hpp"

class ListCommand : public ICommand
{
	public:
		ListCommand(std::string_view processArg);

	private:
		std::string_view getName() const { return "list"; }
		std::string_view getDescription() const { return "List available shares"; }

		void displayHelp(std::ostream& os) const override;
		int process(const std::vector<std::string>& args) const override;

		const std::string _processArg;
		boost::program_options::options_description _options;
};

