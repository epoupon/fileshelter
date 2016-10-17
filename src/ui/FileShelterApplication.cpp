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
#include <Wt/WPushButton>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "ShareCreate.hpp"
#include "ShareCreated.hpp"
#include "ShareDownload.hpp"
#include "ShareEdit.hpp"

#include "FileShelterApplication.hpp"

namespace UserInterface {

// This theme is a workaround to correctly apply the viewport meta header
class FsTheme : public Wt::WBootstrapTheme
{
	public:
		FsTheme(Wt::WObject *parent = 0) : Wt::WBootstrapTheme(parent) {}

		std::vector< Wt::WCssStyleSheet > styleSheets(void) const
		{
			auto res = Wt::WBootstrapTheme::styleSheets();
			Wt::WApplication::instance()->addMetaHeader("viewport", "width=device-width, initial-scale=1.0, user-scalable=no");
			return res;
		}
};


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
	IdxToS			= 5,
};


static Wt::WWebWidget* createHome()
{
	Wt::WTemplate *home = new Wt::WTemplate(Wt::WString::tr("template-home"));
	home->addFunction("tr", &Wt::WTemplate::Functions::tr);

#if WT_VERSION >= 0X03030400
	Wt::WPushButton *createBtn = new Wt::WPushButton("<i class=\"fa fa-upload\"></i> " + Wt::WString::tr("msg-share-create"), Wt::XHTMLText);
#else
	Wt::WPushButton *createBtn = new Wt::WPushButton(Wt::WString::tr("msg-share-create"));
#endif

	createBtn->addStyleClass("btn-primary");
	createBtn->setLink( Wt::WLink(Wt::WLink::InternalPath, "/share-create") );

	home->bindWidget("share-create-btn", createBtn);

	return home;
}

static Wt::WWebWidget* createToS(void)
{
	Wt::WTemplate *tos = new Wt::WTemplate(Wt::WString::tr("template-tos"));
	tos->addFunction("tr", &Wt::WTemplate::Functions::tr);

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
	auto bootstrapTheme = new FsTheme(this);
	bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version3);
	bootstrapTheme->setResponsive(true);
	setTheme(bootstrapTheme);

	useStyleSheet("css/fileshelter.css");
	useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

	// Resouce bundles
	messageResourceBundle().use(appRoot() + "templates");

	FS_LOG(UI, INFO) << "Client address = " << env.clientAddress() << ", UserAgent = '" << env.userAgent() << "', Locale = " << env.locale().name() << ", path = '" << env.internalPath() << "'";

	messageResourceBundle().use(appRoot() + "messages");
	messageResourceBundle().use(appRoot() + "tos");
	messageResourceBundle().use((Config::instance().getPath("working-dir") / "tos_user").string());

	setTitle(Wt::WString::tr("msg-app-name"));

	enableInternalPaths();

	auto main = new Wt::WTemplate(Wt::WString::tr("template-main"), root());

	auto navbar = new Wt::WNavigationBar();
	main->bindWidget("navbar-top", navbar);
	navbar->setResponsive(true);
	navbar->setTitle("<i class=\"fa fa-external-link\"></i> " + Wt::WString::tr("msg-app-name"), Wt::WLink(Wt::WLink::InternalPath, "/home"));

	auto homeAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/home"), "<i class=\"fa fa-home fa-lg\"></i> " + Wt::WString::tr("msg-home"));
	homeAnchor->addStyleClass("fs-nav");
	navbar->addWidget(homeAnchor);

	auto createShareAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/share-create"), "<i class=\"fa fa-upload fa-lg\"></i> " + Wt::WString::tr("msg-share-create"));
	createShareAnchor->addStyleClass("fs-nav");
	navbar->addWidget(createShareAnchor);

	auto container = new Wt::WContainerWidget();
	main->bindWidget("contents", container);

	main->bindWidget("github-link", new Wt::WAnchor("https://github.com/epoupon/fileshelter", "<i class=\"fa fa-github\"> Github</i>"));
	main->bindWidget("tos", new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/tos"), Wt::WString::tr("msg-tos")));

	// Same order as Idx enum
	Wt::WStackedWidget* mainStack = new Wt::WStackedWidget(container);
	mainStack->addWidget(createHome());
	mainStack->addWidget(new ShareCreate());
	mainStack->addWidget(new ShareCreated());
	mainStack->addWidget(new ShareDownload());
	mainStack->addWidget(new ShareEdit());
	mainStack->addWidget(createToS());

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
