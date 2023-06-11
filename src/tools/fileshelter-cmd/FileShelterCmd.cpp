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

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include "share/CreateParameters.hpp"
#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "CreateCommand.hpp"
#include "HelpCommand.hpp"
#include "ListCommand.hpp"

using Commands = std::vector<std::unique_ptr<ICommand>>;

void displayGlobalUsage(std::string_view cmd, std::ostream& os, const Commands& commands)
{
	os << "Usage: " << cmd << " <command> [<args>]\n\n"
		"Available commands:" << std::endl;

	for (const auto& command : commands)
		os << "\t" << command->getName() << "\t\t" << command->getDescription() << std::endl;
}

int main(int argc, char* argv[])
{
	try
	{
		Commands commands;
		commands.push_back(std::make_unique<HelpCommand>(argv[0], commands));
		commands.push_back(std::make_unique<CreateCommand>(argv[0]));
		commands.push_back(std::make_unique<ListCommand>(argv[0]));

		if (argc <= 1)
		{
			displayGlobalUsage(argv[0], std::cout, commands);
			return EXIT_SUCCESS;
		}

		std::vector<std::string> args;
		for (int i {2}; i < argc; ++i)
			args.push_back(argv[i]);

		std::string_view commandName {argv[1]};
		if (auto itCommand {std::find_if(std::cbegin(commands), std::cend(commands), [=](const auto& command) { return command->getName() == commandName; })}; itCommand != std::cend(commands))
		{
			return (*itCommand)->process(args);
		}
		else
		{
			displayGlobalUsage(argv[0], std::cerr,commands);
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
