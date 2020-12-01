/*
 * Copyright (C) 2016 Emeric Poupon
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

#include <filesystem>
#include <iostream>

#include <boost/property_tree/xml_parser.hpp>

#include <Wt/WServer.h>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "database/Db.hpp"
#include "resources/ShareResource.hpp"
#include "share/ShareCleaner.hpp"

#include "ui/FileShelterApplication.hpp"

std::vector<std::string> generateWtConfig(std::string execPath)
{
	std::vector<std::string> args;

	const std::filesystem::path wtConfigPath {Config::instance().getPath("working-dir") / "wt_config.xml"};
	const std::filesystem::path wtLogFilePath {Config::instance().getPath("log-file", "")};
	const std::filesystem::path wtAccessLogFilePath {Config::instance().getPath("access-log-file", "")};
	const std::filesystem::path userMsgPath {Config::instance().getPath("working-dir") / "user_messages.xml"};

	args.push_back(execPath);
	args.push_back("--config=" + wtConfigPath.string());
	args.push_back("--docroot=" + Config::instance().getString("docroot"));
	args.push_back("--approot=" + Config::instance().getString("approot"));
	args.push_back("--deploy-path=" + Config::instance().getString("deploy-path", "/"));
	args.push_back("--resources-dir=" + Config::instance().getString("wt-resources"));

	if (!wtAccessLogFilePath.empty())
		args.push_back("--accesslog=" + wtAccessLogFilePath.string());

	if (Config::instance().getBool("tls-enable", false))
	{
		args.push_back("--https-port=" + std::to_string( Config::instance().getULong("listen-port", 5081)));
		args.push_back("--https-address=" + Config::instance().getString("listen-addr", "0.0.0.0"));
		args.push_back("--ssl-certificate=" + Config::instance().getString("tls-cert"));
		args.push_back("--ssl-private-key=" + Config::instance().getString("tls-key"));
		args.push_back("--ssl-tmp-dh=" + Config::instance().getString("tls-dh"));
	}
	else
	{
		args.push_back("--http-port=" + std::to_string( Config::instance().getULong("listen-port", 5081)));
		args.push_back("--http-address=" + Config::instance().getString("listen-addr", "0.0.0.0"));
	}

	// Generate the wt_config.xml file
	{
		boost::property_tree::ptree pt;

		pt.put("server.application-settings.<xmlattr>.location", "*");
		pt.put("server.application-settings.log-file", wtLogFilePath.string());
		pt.put("server.application-settings.log-config", Config::instance().getString("log-config", "* -debug -info:WebRequest"));
		pt.put("server.application-settings.max-request-size", Config::instance().getULong("max-file-size", 100) * 1024 /* kB */);
		pt.put("server.application-settings.behind-reverse-proxy", Config::instance().getBool("behind-reverse-proxy", false));
		pt.put("server.application-settings.progressive-bootstrap", true);

		std::ofstream oss {wtConfigPath.string().c_str(), std::ios::out};
		boost::property_tree::xml_parser::write_xml(oss, pt);
	}

	// Generate the user_messages.xml file
	{
		boost::property_tree::ptree pt;

		pt.put("messages.<xmlattr>.xmlns:if", "Wt.WTemplate.conditions");

		{
			boost::property_tree::ptree node;
			node.put("<xmlattr>.id", "msg-tos-org");
			node.put("", Config::instance().getString("tos-org", "**[ORG]**"));
			pt.add_child("messages.message", node);
		}

		{
			boost::property_tree::ptree node;
			node.put("<xmlattr>.id", "msg-tos-url");
			node.put("", Config::instance().getString("tos-url", "**[DEPLOY URL]**/tos"));
			pt.add_child("messages.message", node);
		}

		{
			boost::property_tree::ptree node;
			node.add("<xmlattr>.id", "msg-tos-support-email");
			node.put("", Config::instance().getString("tos-support-email", "**[SUPPORT EMAIL ADDRESS]**"));
			pt.add_child("messages.message", node);
		}

		{
			boost::property_tree::ptree node;
			node.add("<xmlattr>.id", "msg-app-name");
			node.put("", Config::instance().getString("app-name", "FileShelter"));
			pt.add_child("messages.message", node);
		}

		std::ofstream oss {userMsgPath.string().c_str(), std::ios::out};
		boost::property_tree::xml_parser::write_xml(oss, pt);
	}

	return args;
}

int main(int argc, char *argv[])
{

	std::filesystem::path configFilePath {"/etc/fileshelter.conf"};
	int res {EXIT_FAILURE};

	assert(argc > 0);
	assert(argv[0] != NULL);

	if (argc >= 2)
		configFilePath = std::string(argv[1], 0, 256);

	try
	{
		Config::instance().setFile(configFilePath);

		// Make sure the working directory exists
		// TODO check with boost::system::error_code ec;
		std::filesystem::create_directories(Config::instance().getPath("working-dir") / "files");

		// Recreate the tmp directory in order to flush it
		const auto tmpDir {Config::instance().getPath("working-dir") / "tmp"};
		std::filesystem::remove_all(tmpDir);
		std::filesystem::create_directories(tmpDir);

		// Set the WT_TMP_DIR inside the working dir, used to upload files
		setenv("WT_TMP_DIR", tmpDir.string().c_str(), 1);

		// Construct WT configuration and get the argc/argv back
		std::vector<std::string> wtServerArgs {generateWtConfig(argv[0])};

		const char* wtArgv[wtServerArgs.size()];
		for (std::size_t i = 0; i < wtServerArgs.size(); ++i)
		{
			std::cout << "ARG = " << wtServerArgs[i] << std::endl;
			wtArgv[i] = wtServerArgs[i].c_str();
		}

		Wt::WServer server {argv[0]};
		server.setServerConfiguration(wtServerArgs.size(), const_cast<char**>(wtArgv));

		Database::Db database {Config::instance().getPath("working-dir") / "fileshelter.db"};

		ShareCleaner shareCleaner {database};

		// bind static resources
		ShareResource shareResource {database};
		server.addResource(&shareResource, std::string {shareResource.getDeployPath()});

		// bind entry point
		server.addEntryPoint(Wt::EntryPointType::Application, [&](const Wt::WEnvironment &env)
		{
		    return UserInterface::FileShelterApplication::create(env, database);
		});

		FS_LOG(MAIN, INFO) << "Starting server...";
		server.start();

		FS_LOG(MAIN, INFO) << "Now running...";
		Wt::WServer::waitForShutdown();

		FS_LOG(MAIN, INFO) << "Stopping server...";
		server.stop();

		res = EXIT_SUCCESS;
	}
	catch (libconfig::FileIOException& e)
	{
		std::cerr << "Cannot open config file " << configFilePath << std::endl;
	}
	catch (libconfig::ParseException& e)
	{
		std::cerr << "Caught libconfig::ParseException! error='" << e.getError() << "', file = '" << e.getFile() << "', line = " << e.getLine() << std::endl;
	}
	catch (Wt::WServer::Exception& e)
	{
		std::cerr << "Caught a WServer::Exception: " << e.what() << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Caught std::exception: " << e.what() << std::endl;
	}

	return res;
}

