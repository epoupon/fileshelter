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

#include <stdlib.h>
#include <filesystem>
#include <iostream>
#include <boost/program_options.hpp>
#include <stdexcept>

#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

static
void
processListCommand(Share::IShareManager& shareManager, bool details)
{
    std::cout.imbue(std::locale("")); // TODO

	std::size_t nbShares{};
	std::size_t totalShareSize{};

	shareManager.visitShares([&](const Share::ShareDesc& share)
	{
		nbShares++;
		totalShareSize += share.size;

		std::cout << "Share '" << share.uuid.toString() << "', expires " << share.expiryTime.toString() << " UTC, " << share.size << " bytes" << ", " << share.files.size() << " file" << (share.files.size() == 1 ? "" : "s") << std::endl;
		if (!details)
			return;

		for (const Share::FileDesc& file : share.files)
		{
			std::cout << "\tFile '" << file.clientPath.string() << "', " << file.size << " bytes" << std::endl;
		}
	});

	std::cout << std::endl << "Share count: " << nbShares << ", " << totalShareSize << " bytes" << std::endl;
}

// TODO: make this app actually controls the fileshelter daemon using a UNIX socket
int main(int argc, char* argv[])
{
	try
	{
		namespace po = boost::program_options;

		// log to stdout
//		Service<Logger> logger {std::make_unique<StreamLogger>(std::cout)};

		po::options_description allOptions;
		po::options_description visibleOptions;
		{
			po::options_description genericOptions{"Generic options"};
			genericOptions.add_options()
				("help,h", "print usage message")
				("conf,c", po::value<std::string>()->default_value("/etc/fileshelter.conf"), "fileshelter config file");

			po::options_description createOptions{"Create options"};
			createOptions.add_options()
				("desc", po::value<std::string>(), "description")
				("file,f", po::value<std::vector<std::filesystem::path>>()->composing(), "file")
				("password", po::value<std::string>(), "password");

			po::options_description listOptions("List options");
			listOptions.add_options()
				("details", "Show details");

			po::options_description hiddenOptions{"Hidden options"};
			hiddenOptions.add_options()
				("action", po::value<std::string>(), "action to perform");

			allOptions.add(genericOptions).add(createOptions).add(listOptions).add(hiddenOptions);
			visibleOptions.add(genericOptions).add(createOptions).add(listOptions);
		}

		po::positional_options_description pos;
		pos.add("action", 1);

		po::parsed_options parsed {po::command_line_parser(argc, argv).
		    options(allOptions).
		    positional(pos).
		    run()
		};

		po::variables_map vm;
		po::store(parsed, vm);

		if (vm.count("help") || !vm.count("action"))
		{
			std::cout << std::endl << argv[0] << " <list|create> [options]" << std::endl;
			std::cout << visibleOptions << std::endl;
			return EXIT_SUCCESS;
		}

		Service<IConfig> config {createConfig(vm["conf"].as<std::string>())};
		Service<Share::IShareManager> shareManager {Share::createShareManager(Service<IConfig>::get()->getPath("working-dir") / "fileshelter.db", false /* enableCleaner */)};

		const std::string action {vm["action"].as<std::string>()};
		if (action == "list")
		{
			processListCommand(*shareManager.get(), vm.count("details"));
		}
		else
		{
			std::cerr << "Unrecognized command '" << action << "'" << std::endl;
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
