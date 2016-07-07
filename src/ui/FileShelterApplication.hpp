#pragma once

#include <Wt/WApplication>

#include "database/DbHandler.hpp"

namespace UserInterface {

class FileShelterApplication : public Wt::WApplication
{
	public:
		static Wt::WApplication *create(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool);
		static FileShelterApplication* instance();

		FileShelterApplication(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool);

		// Session application data
		Database::Handler& getDbHandler() { return _db;}

	private:

		Database::Handler	_db;
};

// Helpers to get session data
#define LmsApp	FileShelterApplication::instance()

Database::Handler& DbHandler();
Wt::Dbo::Session& DboSession();

} // namespace UserInterface


