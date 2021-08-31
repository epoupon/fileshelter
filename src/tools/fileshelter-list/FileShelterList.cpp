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

#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

static
void
processListCommand(Share::IShareManager& shareManager, bool details)
{
	std::cout.imbue(std::locale(""));

	std::size_t nbShares{};
	std::size_t totalShareSize{};

	shareManager.visitShares([&](const Share::ShareDesc& share)
	{
		nbShares++;
		totalShareSize += share.size;

		std::cout << "Share '" << share.uuid.toString()
				<< "', expires " << share.expiryTime.toString() << " UTC, "
				<< "created by '" << share.creatorAddress << "', "
				<< share.size << " bytes" << ", "
				<< share.files.size() << " file" << (share.files.size() == 1 ? "" : "s") << ", "
				<< share.readCount << " download" << (share.readCount > 1 ? "s" : "") << std::endl;
		if (!details)
			return;

		for (const Share::FileDesc& file : share.files)
		{
			std::cout << "\tFile '" << file.path.string() << "' " << (file.isOwned ? "(owned)" : "") << ", '" << file.clientPath.string() << "', " << file.size << " bytes" << std::endl;
		}
	});

	std::cout << std::endl << "Share count: " << nbShares << ", " << totalShareSize << " bytes" << std::endl;
}

int main(int argc, char* argv[])
{
	try
	{
		namespace po = boost::program_options;

		po::options_description options{"Options"};
		options.add_options()
			("help,h", "print usage message")
			("conf,c", po::value<std::string>()->default_value("/etc/fileshelter.conf"), "fileshelter config file")
			("details", "Show details");

		po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, options), vm);

		if (vm.count("help"))
		{
			std::cout << options << std::endl;
            return EXIT_SUCCESS;
        }

		Service<IConfig> config {createConfig(vm["conf"].as<std::string>())};
		Service<Share::IShareManager> shareManager {Share::createShareManager(Service<IConfig>::get()->getPath("working-dir") / "fileshelter.db", false /* enableCleaner */)};

		processListCommand(*shareManager.get(), vm.count("details"));
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
