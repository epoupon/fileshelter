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

#include <Wt/WEnvironment>
#include <Wt/WBootstrapTheme>
#include <Wt/WStackedWidget>
#include <Wt/WNavigationBar>
#include <Wt/WAnchor>
#include <Wt/WTemplate>

#include "utils/Logger.hpp"

#include "Home.hpp"
#include "ShareCreate.hpp"
#include "ShareCreated.hpp"
#include "ShareDownload.hpp"
#include "ShareEdit.hpp"

#include "FileShelterApplication.hpp"

namespace UserInterface {

Wt::WApplication*
FileShelterApplication::create(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool)
{
	/*
	 * You could read information from the environment to decide whether
	 * the user has permission to start a new application
	 */
	return new FileShelterApplication(env, connectionPool);
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
};

void
handlePathChange(Wt::WStackedWidget* stack)
{
	static std::map<std::string, int> indexes =
	{
		{ "/home",		IdxHome },
		{ "/share-create",	IdxShareCreate },
		{ "/share-created",	IdxShareCreated },
		{ "/share-download",	IdxShareDownload },
		{ "/share-edit",	IdxShareEdit },
	};

	for (auto index : indexes)
	{
		if (wApp->internalPathMatches(index.first))
		{
			stack->setCurrentIndex(index.second);
			return;
		}
	}

	// Redirect bad path to the home
	wApp->setInternalPath("/home", true);
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
	Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(this);
	bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version3);
	bootstrapTheme->setResponsive(true);
	setTheme(bootstrapTheme);

	useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

	// Resouce bundles
	messageResourceBundle().use(appRoot() + "templates");

	FS_LOG(UI, INFO) << "Locale = " << env.locale().name();

	if (env.locale().name().find_first_of("fr") == 0)
		messageResourceBundle().use(appRoot() + "messages_fr");
	else
		messageResourceBundle().use(appRoot() + "messages_en");

	setTitle("FileShelter");
	addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");

	enableInternalPaths();

	auto navbar = new Wt::WNavigationBar(root());
	navbar->setResponsive(true);
	navbar->setTitle(Wt::WString::tr("msg-nav-title"), "/home");

	auto homeAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/home"), Wt::WString::tr("msg-nav-home"));
	navbar->addWidget(homeAnchor);

	auto createShareAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/share-create"), Wt::WString::tr("msg-nav-share-create"));
	navbar->addWidget(createShareAnchor);

	auto container = new Wt::WContainerWidget(root());
	container->addStyleClass("container");

	// Same order as Idx enum
	Wt::WStackedWidget* mainStack = new Wt::WStackedWidget(container);
	mainStack->addWidget(new Home());
	mainStack->addWidget(new ShareCreate());
	mainStack->addWidget(new ShareCreated());
	mainStack->addWidget(new ShareDownload());
	mainStack->addWidget(new ShareEdit());

	internalPathChanged().connect(std::bind([=]
	{
		handlePathChange(mainStack);
	}));

	handlePathChange(mainStack);
}


Database::Handler& DbHandler()
{
	return FileShelterApplication::instance()->getDbHandler();
}
Wt::Dbo::Session& DboSession()
{
	return DbHandler().getSession();
}

} // namespace UserInterface
