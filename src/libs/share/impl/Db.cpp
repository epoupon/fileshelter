/*
 * Copyright (C) 2020 Emeric Poupon
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

#include "Db.hpp"

#include <unordered_map>

#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

#include "File.hpp"
#include "Share.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"

namespace Share
{

	using Version = int;
	static constexpr Version FS_DATABASE_VERSION {2};

	class VersionInfo
	{
		public:
			using pointer = Wt::Dbo::ptr<VersionInfo>;

			static VersionInfo::pointer getOrCreate(Wt::Dbo::Session& session)
			{
				pointer versionInfo {session.find<VersionInfo>()};
				if (!versionInfo)
					return session.add(std::make_unique<VersionInfo>());

				return versionInfo;
			}

			static VersionInfo::pointer get(Wt::Dbo::Session& session)
			{
				return session.find<VersionInfo>();
			}

			Version getVersion() const { return _version; }
			void setVersion(Version version) { _version = version; }

			template<class Action>
				void persist(Action& a)
				{
					Wt::Dbo::field(a, _version, "db_version");
				}

		private:
			Version _version {FS_DATABASE_VERSION};
	};

	Db::Db(const std::filesystem::path& db)
	{
		FS_LOG(DB, DEBUG) << "Creating connection pool on file '" << db.string() << "'";

		auto connection {std::make_unique<Wt::Dbo::backend::Sqlite3>(db.string())};
		// Caution: must have only 1 connection to create implicit locking on the database accesses
		_connectionPool = std::make_unique<Wt::Dbo::FixedSqlConnectionPool>(std::move(connection), 1);

		prepare();
	}

	Wt::Dbo::Session&
	Db::getTLSSession()
	{
		static thread_local std::unordered_map<Db*, Wt::Dbo::Session*> tlsSessions {};

		auto itSession {tlsSessions.find(this)};
		if (itSession != std::cend(tlsSessions))
			return *itSession->second;

		auto newSession {createSession()};
		Wt::Dbo::Session* tlsSession {newSession.get()};
		tlsSessions[this] = tlsSession;

		{
			std::scoped_lock lock {_tlsSessionsMutex};
			_tlsSessions.push_back(std::move(newSession));
		}

		return *tlsSession;
	}

	std::unique_ptr<Wt::Dbo::Session>
	Db::createSession()
	{
		auto session {std::make_unique<Wt::Dbo::Session>()};

		session->setConnectionPool(*_connectionPool);
		session->mapClass<VersionInfo>("version_info");
		session->mapClass<File>("file");
		session->mapClass<Share>("share");

		return session;
	}

	void
	Db::prepare()
	{
		auto session {createSession()};

		try
		{
			Wt::Dbo::Transaction transaction {*session};

			session->createTables();
		}
		catch (Wt::Dbo::Exception& e)
		{
			FS_LOG(DB, DEBUG) << "Dbo exception: '" << e.what() << "'";
			// HACK: get rid of the polluting 'already exists' error
			if (std::string_view{e.what()}.find("already exists") == std::string::npos)
			{
				FS_LOG(DB, ERROR) << "Cannot create tables: " << e.what();
				throw e;
			}
		}

		// Indexes
		{
			Wt::Dbo::Transaction transaction {*session};

			session->execute("CREATE INDEX IF NOT EXISTS share_uuid_idx ON share(uuid)");
			session->execute("CREATE INDEX IF NOT EXISTS share_edit_uuid_idx ON share(edit_uuid)");
			session->execute("CREATE INDEX IF NOT EXISTS file_uuid_idx ON file(uuid)");
		}

		doMigrationIfNeeded(*session);
	}

	void
	Db::doMigrationIfNeeded(Wt::Dbo::Session& session)
	{
		try
		{
			Wt::Dbo::Transaction transaction {session};

			VersionInfo::getOrCreate(session)->getVersion();
		}
		catch (Wt::Dbo::Exception& e)
		{
			throw FsException {"Database too old, migration not supported"};
		}
	}

} // namespace Share


