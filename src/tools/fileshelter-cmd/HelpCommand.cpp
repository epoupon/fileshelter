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

#include "HelpCommand.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

HelpCommand::HelpCommand(std::string_view processArg, const std::vector<std::unique_ptr<ICommand>>& commands)
    : _processArg{ processArg }
    , _commands{ commands }
{
}

void HelpCommand::displayHelp(std::ostream& os) const
{
    os << "Usage : " << _processArg << " " << getName() << " <command>\n\n"
       << generateCommandDesc(_commands) << std::endl;
    ;
}

int HelpCommand::process(const std::vector<std::string>& args) const
{
    if (args.size() == 0)
    {
        displayHelp(std::cout);
        return EXIT_SUCCESS;
    }
    else if (args.size() > 1)
    {
        displayHelp(std::cerr);
        return EXIT_FAILURE;
    }

    std::string_view commandName{ args[0] };
    if (auto itCommand{ std::find_if(std::cbegin(_commands), std::cend(_commands), [=](const auto& command) { return command->getName() == commandName; }) }; itCommand != std::cend(_commands) && itCommand->get() != this)
    {
        (*itCommand)->displayHelp(std::cout);
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr << "Unknown command name '" << commandName << "'" << std::endl;
        return EXIT_FAILURE;
    }
}

std::string
HelpCommand::generateCommandDesc(const std::vector<std::unique_ptr<ICommand>>& commands) const
{
    std::string commandList;
    for (const auto& command : commands)
    {
        if (command.get() == this)
            continue;

        if (!commandList.empty())
            commandList += ", ";
        commandList += command->getName();
    }

    return "Command name. Can be one of: " + commandList;
}
