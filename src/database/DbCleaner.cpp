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

#include "DbCleaner.hpp"

#include <Wt/WLocalDateTime.h>

#include "utils/Logger.hpp"

#include "Share.hpp"

namespace Database {

Cleaner::Cleaner(Wt::Dbo::SqlConnectionPool& connectionPool)
: _scheduleTimer(_ioService),
 _db(connectionPool)
{
	_ioService.setThreadCount(1);
}

void
Cleaner::start()
{
	// Schedule an immediate cleanup
	schedule(std::chrono::seconds(0));

	_ioService.start();
}

void
Cleaner::schedule(std::chrono::seconds seconds)
{
	_scheduleTimer.expires_from_now(seconds);
	_scheduleTimer.async_wait([this](boost::system::error_code err)
			{
				if (!err)
					process();
			});
}

void
Cleaner::stop()
{
	_scheduleTimer.cancel();

	_ioService.stop();
}

void
Cleaner::process()
{
	auto now = Wt::WLocalDateTime::currentServerDateTime().toUTC();

	FS_LOG(DB, INFO) << "Cleaning expired shares...";
	Wt::Dbo::Transaction transaction(_db.getSession());

	auto shares = Database::Share::getAll(_db.getSession());
	FS_LOG(DB, INFO) << "Current number of shares: " << shares.size();
	for (auto share : shares)
	{
		// In order not to delete a share that is being downloaded,
		// really remove the share at least a day after it has expired
		if (share->hasExpired() && now > share->getExpiryTime().addDays(1))
		{
			FS_LOG(DB, INFO) << "[" << share->getDownloadUUID() << "] Deleting expired share";

			share.modify()->destroy();
			share.remove();
		}
	}

	FS_LOG(DB, INFO) << "Cleaning expired shares: complete!";

	schedule(std::chrono::hours(6));
}


} // namespace Database

