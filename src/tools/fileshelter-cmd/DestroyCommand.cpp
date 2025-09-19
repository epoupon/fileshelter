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

#include "DestroyCommand.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "share/CreateParameters.hpp"
#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Service.hpp"

static std::vector<Share::ShareEditUUID>
parseShareEditUUIDs(const std::vector<std::string>& shares)
{
    std::vector<Share::ShareEditUUID> res;
    res.reserve(shares.size());

    std::transform(std::cbegin(shares), std::cend(shares), std::back_inserter(res), [](std::string_view uuid) { return Share::ShareEditUUID{ uuid }; });
    return res;
}

static void
processDestroyCommand(Share::IShareManager& shareManager, const std::vector<Share::ShareEditUUID>& shares)
{
    using namespace Share;

    for (const Share::ShareEditUUID& shareEditUUID : shares)
    {
        shareManager.destroyShare(shareEditUUID);
        std::cout << "Share '" << shareEditUUID.toString() << "' destroyed" << std::endl;
    }
}

DestroyCommand::DestroyCommand(std::string_view processArg)
    : _processArg{ processArg }
{
    namespace po = boost::program_options;

    po::options_description options{ "Options" };
    options.add_options()("conf,c", po::value<std::string>()->default_value("/etc/fileshelter.conf"), "fileshelter config file");

    po::options_description hiddenOptions{ "Hidden options" };
    hiddenOptions.add_options()("EditUUID", po::value<std::vector<std::string>>()->composing(), "EditUUID");

    _allOptions.add(options).add(hiddenOptions);
    _visibleOptions.add(options);
}

void DestroyCommand::displayHelp(std::ostream& os) const
{
    os << "Usage: " << _processArg << " " << getName() << " [options] EditUUID...\n";
    os << _visibleOptions << std::endl;
}

int DestroyCommand::process(const std::vector<std::string>& args) const
{
    namespace po = boost::program_options;

    po::positional_options_description pos;
    pos.add("EditUUID", -1);

    po::variables_map vm;
    {
        po::parsed_options parsed{ po::command_line_parser(args)
                .options(_allOptions)
                .positional(pos)
                .run() };
        po::store(parsed, vm);
    }

    if (!vm.count("EditUUID"))
    {
        displayHelp(std::cerr);
        return EXIT_FAILURE;
    }

    Service<IConfig> config{ createConfig(vm["conf"].as<std::string>()) };
    Service<Share::IShareManager> shareManager{ Share::createShareManager(false /* enableCleaner */) };

    processDestroyCommand(*shareManager.get(), parseShareEditUUIDs(vm["EditUUID"].as<std::vector<std::string>>()));

    return EXIT_SUCCESS;
}
