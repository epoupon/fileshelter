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
#include <thread>

#include <boost/property_tree/xml_parser.hpp>

#include <Wt/WServer.h>

#include "share/IShareManager.hpp"
#include "utils/IConfig.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

#include "resources/ShareResource.hpp"

#include "ui/FileShelterApplication.hpp"

std::vector<std::string> generateWtConfig(std::string execPath)
{
	std::vector<std::string> args;

	const std::filesystem::path wtConfigPath {Service<IConfig>::get()->getPath("working-dir") / "wt_config.xml"};
	const std::filesystem::path wtLogFilePath {Service<IConfig>::get()->getPath("log-file", "")};
	const std::filesystem::path wtAccessLogFilePath {Service<IConfig>::get()->getPath("access-log-file", "")};
	const std::filesystem::path userMsgPath {Service<IConfig>::get()->getPath("working-dir") / "user_messages.xml"};
	const unsigned long configHttpServerThreadCount {Service<IConfig>::get()->getULong("http-server-thread-count", 0)};

	args.push_back(execPath);
	args.push_back("--config=" + wtConfigPath.string());
	args.push_back("--docroot=" + std::string {Service<IConfig>::get()->getString("docroot")});
	args.push_back("--approot=" + std::string {Service<IConfig>::get()->getString("approot")});
	args.push_back("--deploy-path=" + std::string {Service<IConfig>::get()->getString("deploy-path", "/")});
	args.push_back("--resources-dir=" + std::string {Service<IConfig>::get()->getString("wt-resources")});

	if (!wtAccessLogFilePath.empty())
		args.push_back("--accesslog=" + wtAccessLogFilePath.string());

	if (Service<IConfig>::get()->getBool("tls-enable", false))
	{
		args.push_back("--https-port=" + std::to_string( Service<IConfig>::get()->getULong("listen-port", 5081)));
		args.push_back("--https-address=" + std::string {Service<IConfig>::get()->getString("listen-addr", "0.0.0.0")});
		args.push_back("--ssl-certificate=" + std::string {Service<IConfig>::get()->getString("tls-cert")});
		args.push_back("--ssl-private-key=" + std::string {Service<IConfig>::get()->getString("tls-key")});
		args.push_back("--ssl-tmp-dh=" + std::string {Service<IConfig>::get()->getString("tls-dh")});
	}
	else
	{
		args.push_back("--http-port=" + std::to_string( Service<IConfig>::get()->getULong("listen-port", 5081)));
		args.push_back("--http-address=" + std::string {Service<IConfig>::get()->getString("listen-addr", "0.0.0.0")});
	}

	{
		// Reserve at least 2 threads since we still have some blocking IO (reading on disk)
		const unsigned long httpServerThreadCount {configHttpServerThreadCount ? configHttpServerThreadCount : std::max<unsigned long>(2, std::thread::hardware_concurrency())};
		args.push_back("--threads=" + std::to_string(httpServerThreadCount));
	}

	// Generate the wt_config.xml file
	{
		boost::property_tree::ptree pt;

		pt.put("server.application-settings.<xmlattr>.location", "*");
		pt.put("server.application-settings.log-file", wtLogFilePath.string());
		pt.put("server.application-settings.log-config", Service<IConfig>::get()->getString("log-config", "* -debug -info:WebRequest"));
		pt.put("server.application-settings.max-request-size", Service<IConfig>::get()->getULong("max-share-size", 100) * 1024 /* kB */);

		if (Service<IConfig>::get()->getBool("behind-reverse-proxy", false))
		{
			pt.put("server.application-settings.trusted-proxy-config.original-ip-header", Service<IConfig>::get()->getString("original-ip-header", "X-Forwared-For"));
			Service<IConfig>::get()->visitStrings("trusted-proxies", [&](std::string_view trustedProxy)
			{
				pt.add("server.application-settings.trusted-proxy-config.trusted-proxies.proxy", std::string {trustedProxy});
			}, {"127.0.0.1", "::1"});
		}

		{
			boost::property_tree::ptree viewport;
			viewport.put("<xmlattr>.name", "viewport");
			viewport.put("<xmlattr>.content", "width=device-width, initial-scale=1, user-scalable=no");
			pt.add_child("server.application-settings.head-matter.meta", viewport);
		}

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
			node.put("", Service<IConfig>::get()->getString("tos-org", "**[ORG]**"));
			pt.add_child("messages.message", node);
		}

		{
			boost::property_tree::ptree node;
			node.put("<xmlattr>.id", "msg-tos-url");
			node.put("", Service<IConfig>::get()->getString("tos-url", "**[DEPLOY URL]**/tos"));
			pt.add_child("messages.message", node);
		}

		{
			boost::property_tree::ptree node;
			node.add("<xmlattr>.id", "msg-tos-support-email");
			node.put("", Service<IConfig>::get()->getString("tos-support-email", "**[SUPPORT EMAIL ADDRESS]**"));
			pt.add_child("messages.message", node);
		}

		{
			boost::property_tree::ptree node;
			node.add("<xmlattr>.id", "msg-app-name");
			node.put("", Service<IConfig>::get()->getString("app-name", "FileShelter"));
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
		Service<IConfig> config {createConfig(configFilePath)};

		// Make sure the working directory exists
		std::filesystem::create_directories(Service<IConfig>::get()->getPath("working-dir"));

		const auto uploadedFilesPath {Service<IConfig>::get()->getPath("working-dir") / "uploaded-files"};
		std::filesystem::create_directories(uploadedFilesPath);

		// Set the WT_TMP_DIR inside the working dir, used to upload files
		setenv("WT_TMP_DIR", uploadedFilesPath.string().c_str(), 1);

		// Construct WT configuration and get the argc/argv back
		std::vector<std::string> wtServerArgs {generateWtConfig(argv[0])};

		const char* wtArgv[wtServerArgs.size()];
		for (std::size_t i = 0; i < wtServerArgs.size(); ++i)
		{
			std::cout << "ARG = " << wtServerArgs[i] << std::endl;
			wtArgv[i] = wtServerArgs[i].c_str();
		}

		// Create server first to handle log config etc.
		Wt::WServer server {argv[0]};
		server.setServerConfiguration(wtServerArgs.size(), const_cast<char**>(wtArgv));

		const std::string deployPath {Service<IConfig>::get()->getString("deploy-path", "/")};

		Service<Share::IShareManager> shareManager {Share::createShareManager(Service<IConfig>::get()->getPath("working-dir") / "fileshelter.db", true /* enableCleaner */)};
		shareManager->removeOrphanFiles(uploadedFilesPath);

		ShareResource shareResource;
		server.addResource(&shareResource, std::string {shareResource.getDeployPath()});
		server.addEntryPoint(Wt::EntryPointType::Application, [&](const Wt::WEnvironment& env)
		{
		    return UserInterface::FileShelterApplication::create(env);
		});

		FS_LOG(MAIN, INFO) << "Starting server...";
		server.start();

		FS_LOG(MAIN, INFO) << "Now running...";
		Wt::WServer::waitForShutdown();

		FS_LOG(MAIN, INFO) << "Stopping server...";
		server.stop();

		res = EXIT_SUCCESS;
	}
	catch (const FsException& e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
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

