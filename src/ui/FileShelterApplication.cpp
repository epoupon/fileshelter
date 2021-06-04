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
#include <Wt/WMenu.h>
#include <Wt/WTemplate.h>

#include "database/Db.hpp"
#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "ShareCreate.hpp"
#include "ShareCreated.hpp"
#include "ShareDownload.hpp"
#include "ShareEdit.hpp"
#include "TermsOfService.hpp"

namespace UserInterface {

static const char * defaultPath{ "/share-create" };


std::unique_ptr<Wt::WApplication>
FileShelterApplication::create(const Wt::WEnvironment& env, Database::Db& db)
{
	return std::make_unique<FileShelterApplication>(env, db);
}

FileShelterApplication*
FileShelterApplication::instance()
{
	return reinterpret_cast<FileShelterApplication*>(Wt::WApplication::instance());
}

enum Idx
{
	IdxShareCreate		= 0,
	IdxShareCreated		= 1,
	IdxShareDownload	= 2,
	IdxShareEdit		= 3,
	IdxToS				= 4,
};

static
void
handlePathChange(Wt::WStackedWidget* stack)
{
	static std::map<std::string, int> indexes =
	{
		{ "/share-create",		IdxShareCreate },
		{ "/share-created",		IdxShareCreated },
		{ "/share-download",	IdxShareDownload },
		{ "/share-edit",		IdxShareEdit },
		{ "/tos",				IdxToS },
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

	wApp->setInternalPath(defaultPath, false);
}

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the Wt::WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
*/
FileShelterApplication::FileShelterApplication(const Wt::WEnvironment& env, Database::Db& db)
: Wt::WApplication {env},
  _db {db}
{
	auto bootstrapTheme {std::make_unique<Wt::WBootstrapTheme>()};
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
	navbar->setTitle("<i class=\"fa fa-external-link\"></i> " + Wt::WString::tr("msg-app-name"), Wt::WLink(Wt::LinkType::InternalPath, defaultPath));

	Wt::WMenu* menu = navbar->addMenu(std::make_unique<Wt::WMenu>());
	{
		auto menuItem = menu->addItem(Wt::WString::tr("msg-share-create"));
		menuItem->setLink(Wt::WLink {Wt::LinkType::InternalPath, "/share-create"});
		menuItem->setSelectable(true);
	}
	{
		auto menuItem = menu->addItem(Wt::WString::tr("msg-tos"));
		menuItem->setLink(Wt::WLink {Wt::LinkType::InternalPath, "/tos"});
		menuItem->setSelectable(true);
	}

	Wt::WContainerWidget* container {main->bindNew<Wt::WContainerWidget>("contents")};

	// Same order as Idx enum
	Wt::WStackedWidget* mainStack = container->addNew<Wt::WStackedWidget>();
	mainStack->addNew<ShareCreate>();
	mainStack->addNew<ShareCreated>();
	mainStack->addNew<ShareDownload>();
	mainStack->addNew<ShareEdit>();
	mainStack->addWidget(createTermsOfService());

	internalPathChanged().connect([=]
	{
		handlePathChange(mainStack);
	});

	handlePathChange(mainStack);
}

Wt::Dbo::Session&
FileShelterApplication::getDboSession()
{
	return _db.getTLSSession();
}


} // namespace UserInterface
