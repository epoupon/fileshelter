#include <Wt/WEnvironment>
#include <Wt/WBootstrapTheme>

#include "utils/Logger.hpp"

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

//	useStyleSheet("css/lms.css");
	useStyleSheet("resources/font-awesome/css/font-awesome.min.css");

	// Add a resource bundle
	messageResourceBundle().use(appRoot() + "templates");

	setTitle("FileShelter");

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
