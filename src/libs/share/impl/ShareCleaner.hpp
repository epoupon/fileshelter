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

#pragma once

#include <chrono>
#include <filesystem>
#include <boost/asio/steady_timer.hpp>
#include <Wt/WIOService.h>

namespace Share
{
	class Db;
	class Share;
	class ShareCleaner
	{
		public:
			ShareCleaner(Db& _db);
			~ShareCleaner();

			ShareCleaner(const ShareCleaner&) = delete;
			ShareCleaner(ShareCleaner&&) = delete;
			ShareCleaner& operator=(const ShareCleaner&) = delete;
			ShareCleaner& operator=(ShareCleaner&&) = delete;

			void removeOrphanFiles(const std::filesystem::path& directory);

		private:
			bool	isOrphanFile(const std::filesystem::path& file);
			void	scheduleNextCheck();
			void	checkExpiredShares();

			Db&							_db;
			const std::chrono::seconds	_checkPeriod {std::chrono::hours {1}};
			Wt::WIOService				_ioService;
			boost::asio::steady_timer	_timer;
	};

} // namespace Share
