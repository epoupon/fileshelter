/*
 * Copyright (C) 2021 Emeric Poupon
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

#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "Db.hpp"
#include "Share.hpp"

namespace Share
{
	ShareCleaner::ShareCleaner(Db& db)
	: _db {db}
	, _timer {_ioService}
	{
		FS_LOG(SHARE, DEBUG) << "Started cleaner";
		checkExpiredShares();

		_ioService.start();

		scheduleNextCheck();
	}

	ShareCleaner::~ShareCleaner()
	{
		_timer.cancel();
		_ioService.stop();
		FS_LOG(SHARE, DEBUG) << "Stopped cleaner";
	}

	void
	ShareCleaner::scheduleNextCheck()
	{
		_timer.expires_after(_checkPeriod);

		_timer.async_wait([this](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			checkExpiredShares();
		});
	}

	void
	ShareCleaner::checkExpiredShares()
	{
		FS_LOG(SHARE, DEBUG) << "Checking expired shares...";

		Wt::Dbo::Session& session {_db.getTLSSession()};
		Wt::Dbo::Transaction transaction {session};

		Share::visitAll(session, [](Share::pointer& share)
		{
			if (share->isExpired())
			{
				FS_LOG(SHARE, INFO) << "Removing expired share '" << share->getUUID().toString() << "'";
				Share::destroy(share);
			}
		});
	}
} // namespace Share
