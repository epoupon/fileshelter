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

#include <boost/asio/placeholders.hpp>
#include "utils/Logger.hpp"

#include "Share.hpp"
#include "DbCleaner.hpp"

namespace Database {

boost::gregorian::date
getNextDay(const boost::gregorian::date& current)
{
	boost::gregorian::day_iterator it(current);
	return *(++it);
}

Cleaner::Cleaner(Wt::Dbo::SqlConnectionPool& connectionPool)
: _scheduleTimer(_ioService),
 _db(connectionPool)
{
	_ioService.setThreadCount(1);
}

void
Cleaner::start(void)
{
	schedule();
	_ioService.start();
}

void
Cleaner::schedule(void)
{
	boost::gregorian::date nextScanDate = getNextDay(boost::gregorian::day_clock::universal_day());

	auto nextScanTime = boost::posix_time::ptime( nextScanDate, boost::posix_time::seconds(0));

	_scheduleTimer.expires_at(nextScanTime);
	_scheduleTimer.async_wait( boost::bind( &Cleaner::process, this, boost::asio::placeholders::error));

	FS_LOG(DB, INFO) << "Scheduled next scan at UTC " << nextScanTime;
}

void
Cleaner::stop(void)
{
	_scheduleTimer.cancel();

	_ioService.stop();
}

void
Cleaner::process(boost::system::error_code err)
{
	if (err)
		return;

	boost::gregorian::date currentDate = boost::gregorian::day_clock::universal_day();
	FS_LOG(DB, INFO) << "Cleaning expired shares...";
	Wt::Dbo::Transaction transaction(_db.getSession());

	auto shares = Database::Share::getAll(_db.getSession());
	for (auto share : shares)
	{
		// In order not to delete a share that is being downloaded,
		// really remove the share at least a day after it has expired
		if (share->hasExpired() && share->getExpiracyDate() != currentDate)
		{
			FS_LOG(DB, INFO) << "Deleting expired share " << share->getDownloadUUID();

			boost::filesystem::remove(share->getPath(), err);
			if (err)
				FS_LOG(DB, ERROR) << "Cannot remove file " << share->getPath() << ": " << err.message();

			share.remove();
		}
	}

	FS_LOG(DB, INFO) << "Cleaning expired shares: complete!";

	schedule();
}


} // namespace Database

