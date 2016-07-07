#include <Wt/WEnvironment>
#include <Wt/WBootstrapTheme>
#include <Wt/WStackedWidget>


#include "utils/Logger.hpp"

#include "Home.hpp"
#include "ShareCreate.hpp"
#include "ShareCreated.hpp"
#include "ShareDownload.hpp"

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
	IdxShareEdit		= 5,
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
//		{ "/share-edit",	IdxShareEdit },
	};

	for (auto index : indexes)
	{
		if (wApp->internalPathMatches(index.first))
		{
			stack->setCurrentIndex(index.second);
			return;
		}
	}

	// Redirect bad paths to the home
	stack->setCurrentIndex(IdxHome);
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
	messageResourceBundle().use(appRoot() + "messages");

	setTitle("FileShelter");

	enableInternalPaths();

	root()->addStyleClass("container");

	// Same order as Idx enum
	Wt::WStackedWidget* mainStack = new Wt::WStackedWidget(root());
	mainStack->addWidget(new Home());
	mainStack->addWidget(new ShareCreate());
	mainStack->addWidget(new ShareCreated());
	mainStack->addWidget(new ShareDownload());

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
