#pragma once

#include <boost/filesystem.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/SqlConnectionPool>

namespace Database {

class Handler
{
	public:

		Handler(Wt::Dbo::SqlConnectionPool& connectionPool);

		Wt::Dbo::Session& getSession() { return _session; }

		static Wt::Dbo::SqlConnectionPool*	createConnectionPool(boost::filesystem::path db);

	private:

		Wt::Dbo::Session		_session;
};

} // namespace Database


