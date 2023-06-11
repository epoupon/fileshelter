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

#include "ListCommand.hpp"

#include <stdlib.h>
#include <filesystem>
#include <iostream>

#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "Common.hpp"

static
void
processListCommand(Share::IShareManager& shareManager, bool details, std::string_view deployURL)
{
	std::cout.imbue(std::locale {""});

	std::size_t nbShares{};
	std::size_t totalShareSize{};

	shareManager.visitShares([&](const Share::ShareDesc& share)
	{
		nbShares++;
		totalShareSize += share.size;
		displayShareDesc(share, details, deployURL);
	});

	std::cout << std::endl << "Share count: " << nbShares << ", " << totalShareSize << " bytes" << std::endl;
}

ListCommand::ListCommand(std::string_view processArg)
	: _processArg {processArg}
	, _options {"Options"}
{
	namespace po = boost::program_options;

	_options.add_options()
		("conf,c", po::value<std::string>()->default_value("/etc/fileshelter.conf"), "fileshelter config file")
		("url,u", po::value<std::string>()->default_value(""), "deploy URL")
		("details,d", "Show details");
}

void
ListCommand::displayHelp(std::ostream& os) const
{
	os << "Usage: " << _processArg << " " << getName() << " [options]\n\n";
	os << _options << std::endl;
}

int
ListCommand::process(const std::vector<std::string>& args) const
{
	namespace po = boost::program_options;

	po::variables_map vm;
	{
		po::parsed_options parsed {po::command_line_parser(args)
			.options(_options)
			.run()};
		po::store(parsed, vm);
	}

	Service<IConfig> config {createConfig(vm["conf"].as<std::string>())};
	Service<Share::IShareManager> shareManager {Share::createShareManager(false /* enableCleaner */)};

	processListCommand(*shareManager.get(), vm.count("details"), vm["url"].as<std::string>());

	return EXIT_SUCCESS;
}
