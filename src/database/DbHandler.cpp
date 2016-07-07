#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/Dbo/backend/Sqlite3>

#include "utils/Logger.hpp"

#include "DbHandler.hpp"

#include "Share.hpp"

namespace Database {

Handler::Handler(Wt::Dbo::SqlConnectionPool& connectionPool)
{
	_session.setConnectionPool(connectionPool);

	_session.mapClass<Database::Share>("share");

	try {
		Wt::Dbo::Transaction transaction(_session);

	        _session.createTables();
	}
	catch(std::exception& e) {
		FS_LOG(DB, ERROR) << "Cannot create tables: " << e.what();
	}

	{
		Wt::Dbo::Transaction transaction(_session);

		// Indexes
		_session.execute("PRAGMA journal_mode=WAL");
		_session.execute("CREATE INDEX IF NOT EXISTS share_download_uid_idx ON share(download_UUID)");
		_session.execute("CREATE INDEX IF NOT EXISTS share_edit_uid_idx ON share(edit_UUID)");
	}
}

Wt::Dbo::SqlConnectionPool*
Handler::createConnectionPool(boost::filesystem::path p)
{
	FS_LOG(DB, INFO) << "Creating connection pool on file " << p;

	Wt::Dbo::backend::Sqlite3 *connection = new Wt::Dbo::backend::Sqlite3(p.string());

	connection->executeSql("pragma journal_mode=WAL");

	//  connection->setProperty("show-queries", "true");
	return new Wt::Dbo::FixedSqlConnectionPool(connection, 1);
}

} // namespace Database

