
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
#include <stdexcept>
#include <vector>
#include <boost/program_options.hpp>

#include "share/CreateParameters.hpp"
#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

/* Function used to check that 'opt1' and 'opt2' are not specified
   at the same time. */
static
void
conflictingOptions(const boost::program_options::variables_map& vm, const char* opt1, const char* opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) && !vm[opt2].defaulted())
	{
        throw std::logic_error(std::string {"Conflicting options '"} + opt1 + "' and '" + opt2 + "'.");
	}
}

static
void
processCreateCommand(Share::IShareManager& shareManager,
		const std::vector<std::string>& files,
		std::string_view desc,
		std::chrono::seconds validityPeriod,
		std::string_view password)
{
	using namespace Share;

	std::cout.imbue(std::locale(""));

	Share::ShareCreateParameters shareParameters;
	shareParameters.validityPeriod = validityPeriod;
	shareParameters.description = desc;
	shareParameters.creatorAddress = "local";
	shareParameters.password = password;

	std::vector<FileCreateParameters> fileParameters;
	fileParameters.reserve(files.size());
	for (const std::string& strPath : files)
	{
		const std::filesystem::path p {strPath};
		fileParameters.emplace_back(FileCreateParameters {p, p.filename()});
	}

	const ShareEditUUID shareEditUUID {shareManager.createShare(shareParameters, fileParameters, false /* keep ownership */)};
	std::cout << "Successfuly created share. Edit UUID = '" << shareEditUUID.toString() << "'" << std::endl;
}

int main(int argc, char* argv[])
{
	try
	{
		namespace po = boost::program_options;

		po::options_description allOptions;
		po::options_description visibleOptions;
		{
			po::options_description options{"Options"};
			options.add_options()
				("help,h", "print usage message")
				("conf,c", po::value<std::string>()->default_value("/etc/fileshelter.conf"), "fileshelter config file")
				("desc", po::value<std::string>()->default_value(""), "description")
				("password", po::value<std::string>()->default_value(""), "password")
				("validity-hours", po::value<unsigned>(), "validity period in hours")
				("validity-days", po::value<unsigned>(), "Validity period in days");

			po::options_description hiddenOptions{"Hidden options"};
			hiddenOptions.add_options()
				("file,f", po::value<std::vector<std::string>>()->composing(), "file");

			allOptions.add(options).add(hiddenOptions);
			visibleOptions.add(options);
		}

		po::positional_options_description pos;
		pos.add("file", -1);

		po::variables_map vm;
		{
			po::parsed_options parsed {po::command_line_parser(argc, argv).
			    options(allOptions).
			    positional(pos).
			    run()
			};
			po::store(parsed, vm);
		}

		auto displayUsage {[&](std::ostream& os)
		{
			os << "Usage: " << argv[0] << " [options] file..." << std::endl;
			os << visibleOptions << std::endl;
		}};

		if (vm.count("help"))
		{
			displayUsage(std::cout);
			return EXIT_SUCCESS;
		}
		else if (!vm.count("file"))
		{
			displayUsage(std::cerr);
			return EXIT_FAILURE;
		}

		Service<IConfig> config {createConfig(vm["conf"].as<std::string>())};
		Service<Share::IShareManager> shareManager {Share::createShareManager(Service<IConfig>::get()->getPath("working-dir") / "fileshelter.db", false /* enableCleaner */)};

		conflictingOptions(vm, "validity-hours", "validity-days");

		std::chrono::seconds validityPeriod {};
		if (vm.count("validity-hours"))
			validityPeriod = std::chrono::seconds {vm["validity-hours"].as<unsigned>()};
		else if (vm.count("validity-days"))
			validityPeriod = std::chrono::hours {vm["validity-days"].as<unsigned>()} * 24;
		else
			validityPeriod = shareManager.get()->getDefaultValidityPeriod();

		processCreateCommand(*shareManager.get(), vm["file"].as<std::vector<std::string>>(), vm["desc"].as<std::string>(), validityPeriod, vm["password"].as<std::string>());
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
