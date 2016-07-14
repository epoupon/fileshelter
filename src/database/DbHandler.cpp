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

	try
	{
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

