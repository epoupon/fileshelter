#pragma once

#include <Wt/WIOService>

#include <boost/asio/deadline_timer.hpp>

#include "DbHandler.hpp"

namespace Database {

class Cleaner
{
	public:
		Cleaner(Wt::Dbo::SqlConnectionPool& connectionPool);

		void start();
		void stop();

	private:

		void schedule(void);
		void process(boost::system::error_code ec);

		Wt::WIOService _ioService;
		boost::asio::deadline_timer _scheduleTimer;
		Database::Handler _db;
};

} // namespace Database

