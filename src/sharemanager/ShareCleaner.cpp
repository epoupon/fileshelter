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

#include "ShareCleaner.hpp"

#include <Wt/WLocalDateTime.h>

#include "database/Db.hpp"
#include "utils/Logger.hpp"
#include "share/ShareUtils.hpp"


ShareCleaner::ShareCleaner(Database::Db& db)
: _scheduleTimer {_ioService},
 _db {db}
{
	_ioService.setThreadCount(1);
	start();
}

ShareCleaner::~ShareCleaner()
{
	stop();
}

void
ShareCleaner::start()
{
	// Schedule an immediate cleanup
	schedule(std::chrono::seconds {0});

	_ioService.start();
}

void
ShareCleaner::schedule(std::chrono::seconds seconds)
{
	_scheduleTimer.expires_from_now(seconds);
	_scheduleTimer.async_wait([this](boost::system::error_code err)
	{
		if (!err)
			process();
	});
}

void
ShareCleaner::stop()
{
	_scheduleTimer.cancel();

	_ioService.stop();
}

void
ShareCleaner::process()
{
	ShareUtils::destroyExpiredShares(_db.getTLSSession());

	schedule(std::chrono::hours {6});
}


