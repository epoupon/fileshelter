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
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMenu.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WAnchor.h>
#include <Wt/WHBoxLayout.h>

#include "utils/IConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

#include "Exception.hpp"
#include "ShareCreate.hpp"
#include "ShareCreated.hpp"
#include "ShareDownload.hpp"
#include "ShareEdit.hpp"
#include "TermsOfService.hpp"

namespace UserInterface {

static const char * defaultPath{ "/share-create" };

std::filesystem::path
prepareUploadDirectory()
{
	return FileShelterApplication::prepareUploadDirectory();
}

std::unique_ptr<Wt::WApplication>
createFileShelterApplication(const Wt::WEnvironment& env)
{
	return std::make_unique<FileShelterApplication>(env);
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
            auto app = dynamic_cast<FileShelterApplication*>(Wt::WApplication::instance());
            app->updateMenuVisibility();
            
			stack->setCurrentIndex(index.second);

			return;
		}
	}
    
    wApp->setInternalPath(defaultPath, true);
}

FileShelterApplication::FileShelterApplication(const Wt::WEnvironment& env)
: Wt::WApplication {env}
{
	useStyleSheet("css/fileshelter.css");
	useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

	// Resouce bundles
	messageResourceBundle().use(appRoot() + "templates");
	messageResourceBundle().use(appRoot() + "messages");
	messageResourceBundle().use((Service<IConfig>::get()->getPath("working-dir") / "user_messages").string());
	if (Service<IConfig>::get()->getPath("tos-custom").empty())
		messageResourceBundle().use(appRoot() + "tos");

	auto bootstrapTheme {std::make_unique<Wt::WBootstrap5Theme>()};
	setTheme(std::move(bootstrapTheme));

	FS_LOG(UI, INFO) << "Client address = " << env.clientAddress() << ", UserAgent = '" << env.userAgent() << "', Locale = " << env.locale().name() << ", path = '" << env.internalPath() << "'";

	setTitle(Wt::WString::tr("msg-app-name"));

	enableInternalPaths();
}

std::filesystem::path
FileShelterApplication::prepareUploadDirectory()
{
	_workingDirectory = Service<IConfig>::get()->getPath("working-dir");

	std::filesystem::path uploadDirectory {_workingDirectory / "uploaded-files"};
	std::filesystem::create_directories (uploadDirectory);

	// Set the WT_TMP_DIR inside the working dir, used to upload files
	setenv("WT_TMP_DIR", uploadDirectory.string().c_str(), 1);

	return uploadDirectory;
}

FileShelterApplication*
FileShelterApplication::instance()
{
	return reinterpret_cast<FileShelterApplication*>(Wt::WApplication::instance());
}

void
FileShelterApplication::initialize()
{
	Wt::WTemplate* main {root()->addNew<Wt::WTemplate>(Wt::WString::tr("template-main"))};

	Wt::WNavigationBar* navbar {main->bindNew<Wt::WNavigationBar>("navbar-top")};
    
	navbar->setTitle(Wt::WString::tr("msg-app-name"));

	Wt::WMenu* menu {navbar->addMenu(std::make_unique<Wt::WMenu>())};
 
	_menuItemShareCreate = menu->addItem(Wt::WString::tr("msg-share-create"));
	_menuItemShareCreate->setLink(Wt::WLink {Wt::LinkType::InternalPath, "/share-create"});
	_menuItemShareCreate->setSelectable(true);

	Wt::WMenuItem* menuItemTos = menu->addItem(Wt::WString::tr("msg-tos"));
	menuItemTos->setLink(Wt::WLink {Wt::LinkType::InternalPath, "/tos"});
	menuItemTos->setSelectable(true);

	Wt::WContainerWidget* container {main->bindNew<Wt::WContainerWidget>("contents")};

	// Same order as Idx enum
	Wt::WStackedWidget* mainStack {container->addNew<Wt::WStackedWidget>()};
	mainStack->addNew<ShareCreate>(_workingDirectory);
	mainStack->addNew<ShareCreated>();
	mainStack->addNew<ShareDownload>();
	mainStack->addNew<ShareEdit>();
	mainStack->addWidget(createTermsOfService());

	internalPathChanged().connect(mainStack, [mainStack]
	{
		handlePathChange(mainStack);
	});

	handlePathChange(mainStack);
    
   FS_LOG(UI, INFO) << "end init";
}

void 
FileShelterApplication::updateMenuVisibility()
{
    if (!Service<IConfig>::get()->getBool("show-create-links-on-download", true))
    {
        if (wApp->internalPathMatches("/share-download"))
        {
            _menuItemShareCreate->hide();
        }
        else
        {
            _menuItemShareCreate->show();
        }
    }
}

void
FileShelterApplication::displayError(std::string_view error)
{
	root()->clear();

	Wt::WTemplate *t {root()->addNew<Wt::WTemplate>(Wt::WString::tr("template-error"))};
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
	t->bindString("error", std::string {error});
}

void
FileShelterApplication::notify(const Wt::WEvent& event)
{
	try
	{
		WApplication::notify(event);
	}
	catch (const Exception& e)
	{
		FS_LOG(UI, WARNING) << "Caught an UI exception: " << e.what();
		displayError(e.what());
	}
	catch (const std::exception& e)
	{
		FS_LOG(UI, ERROR) << "Caught exception: " << e.what();

		 // Do not put details or rethrow the exception here at it may appear on the user rendered html
		throw FsException {"Internal error"};
	}
}

} // namespace UserInterface
