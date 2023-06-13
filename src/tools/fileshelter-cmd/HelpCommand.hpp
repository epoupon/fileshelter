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

#include <memory>
#include <vector>
#include "ICommand.hpp"

class HelpCommand : public ICommand
{
	public:
		HelpCommand(std::string_view processArg, const std::vector<std::unique_ptr<ICommand>>& commands);

		std::string_view getName() const { return "help"; }
		std::string_view getDescription() const { return "Show this help or display command specific help"; }
		void displayHelp(std::ostream& os) const override;
		int process(const std::vector<std::string>& args) const override;

	private:
		std::string generateCommandDesc(const std::vector<std::unique_ptr<ICommand>>& commands) const;

		const std::string _processArg;
		const std::vector<std::unique_ptr<ICommand>>& _commands;
};

