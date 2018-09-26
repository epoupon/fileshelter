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

#include "FileShelterApplication.hpp"

#include <Wt/WEnvironment.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WAnchor.h>
#include <Wt/WMenu.h>
#include <Wt/WTemplate.h>
#include <Wt/WPushButton.h>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "ShareCreate.hpp"
#include "ShareCreated.hpp"
#include "ShareDownload.hpp"
#include "ShareEdit.hpp"

namespace UserInterface {



std::unique_ptr<Wt::WApplication>
FileShelterApplication::create(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool)
{
	/*
	 * You could read information from the environment to decide whether
	 * the user has permission to start a new application
	 */
	return std::make_unique<FileShelterApplication>(env, connectionPool);
}

FileShelterApplication*
FileShelterApplication::instance()
{
	return reinterpret_cast<FileShelterApplication*>(Wt::WApplication::instance());
}

enum Idx
{
	IdxHome 		= 0,
	IdxShareCreate		= 1,
	IdxShareCreated		= 2,
	IdxShareDownload	= 3,
	IdxShareEdit		= 4,
	IdxToS			= 5,
};


static std::unique_ptr<Wt::WWebWidget> createHome()
{
	auto home = std::make_unique<Wt::WTemplate>(Wt::WString::tr("template-home"));
	home->addFunction("tr", &Wt::WTemplate::Functions::tr);
	home->addFunction("block", &Wt::WTemplate::Functions::block);

	Wt::WPushButton* createBtn = home->bindNew<Wt::WPushButton>("share-create-btn", "<i class=\"fa fa-upload\"></i> " + Wt::WString::tr("msg-share-create"), Wt::TextFormat::XHTML);

	createBtn->addStyleClass("btn-primary");
	createBtn->setLink( Wt::WLink(Wt::LinkType::InternalPath, "/share-create") );

	return home;
}

static std::unique_ptr<Wt::WWebWidget> createToS(void)
{
	auto tos = std::make_unique<Wt::WTemplate>();

	// Override the ToS with a custom version is specified
	auto path = Config::instance().getOptPath("tos-custom");
	if (path)
	{
		std::ifstream ifs(path->string().c_str());
		std::stringstream buffer;
		buffer << ifs.rdbuf();

		tos->setTemplateText(buffer.str());
	}
	else
	{
		tos->setTemplateText(Wt::WString::tr("template-tos"));
		tos->addFunction("tr", &Wt::WTemplate::Functions::tr);
	}

	return tos;
}

static void
handlePathChange(Wt::WStackedWidget* stack)
{
	static std::map<std::string, int> indexes =
	{
		{ "/home",		IdxHome },
		{ "/share-create",	IdxShareCreate },
		{ "/share-created",	IdxShareCreated },
		{ "/share-download",	IdxShareDownload },
		{ "/share-edit",	IdxShareEdit },
		{ "/tos",		IdxToS },
	};

	FS_LOG(UI, DEBUG) << "Internal path changed to '" << wApp->internalPath() << "'";

	for (auto index : indexes)
	{
		if (wApp->internalPathMatches(index.first))
		{
			stack->setCurrentIndex(index.second);
			return;
		}
	}

	// Redirect bad path to the home
	wApp->setInternalPath("/home", false);
}

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the Wt::WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
*/
FileShelterApplication::FileShelterApplication(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool)
: Wt::WApplication(env),
  _db(connectionPool)
{
	auto  bootstrapTheme = std::make_unique<Wt::WBootstrapTheme>();
	bootstrapTheme->setVersion(Wt::BootstrapVersion::v3);
	bootstrapTheme->setResponsive(true);
	setTheme(std::move(bootstrapTheme));

	useStyleSheet("css/fileshelter.css");
	useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

	addMetaHeader(Wt::MetaHeaderType::Meta, "viewport", "width=device-width, user-scalable=no");

	// Resouce bundles
	messageResourceBundle().use(appRoot() + "templates");

	FS_LOG(UI, INFO) << "Client address = " << env.clientAddress() << ", UserAgent = '" << env.userAgent() << "', Locale = " << env.locale().name() << ", path = '" << env.internalPath() << "'";

	messageResourceBundle().use(appRoot() + "messages");
	messageResourceBundle().use((Config::instance().getPath("working-dir") / "user_messages").string());
	if (!Config::instance().getOptPath("tos-custom"))
		messageResourceBundle().use(appRoot() + "tos");

	setTitle(Wt::WString::tr("msg-app-name"));

	enableInternalPaths();

	Wt::WTemplate* main = root()->addNew<Wt::WTemplate>(Wt::WString::tr("template-main"));

	Wt::WNavigationBar* navbar = main->bindNew<Wt::WNavigationBar>("navbar-top");
	navbar->setResponsive(true);
	navbar->setTitle("<i class=\"fa fa-external-link\"></i> " + Wt::WString::tr("msg-app-name"), Wt::WLink(Wt::LinkType::InternalPath, "/home"));

	Wt::WMenu* menu = navbar->addMenu(std::make_unique<Wt::WMenu>());
	{
		auto menuItem = menu->insertItem(0, Wt::WString::tr("msg-home"));
		menuItem->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/home"));
		menuItem->setSelectable(false);
	}
	{
		auto menuItem = menu->insertItem(1, Wt::WString::tr("msg-share-create"));
		menuItem->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/share-create"));
		menuItem->setSelectable(false);
	}

	Wt::WContainerWidget* container = main->bindNew<Wt::WContainerWidget>("contents");

	main->bindNew<Wt::WAnchor>("tos", Wt::WLink(Wt::LinkType::InternalPath, "/tos"), Wt::WString::tr("msg-tos"));

	// Same order as Idx enum
	Wt::WStackedWidget* mainStack = container->addNew<Wt::WStackedWidget>();
	mainStack->addWidget(createHome());
	mainStack->addNew<ShareCreate>();
	mainStack->addNew<ShareCreated>();
	mainStack->addNew<ShareDownload>();
	mainStack->addNew<ShareEdit>();
	mainStack->addWidget(createToS());

	internalPathChanged().connect(std::bind([=]
	{
		handlePathChange(mainStack);
	}));

	handlePathChange(mainStack);
}

} // namespace UserInterface
