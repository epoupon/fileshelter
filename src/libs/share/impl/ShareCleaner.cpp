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

#include <Wt/WLocalDateTime.h>

#include "Db.hpp"
#include "File.hpp"
#include "Share.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

namespace Share
{
    ShareCleaner::ShareCleaner(Db& db, const std::filesystem::path& workingDirectory)
        : _db{ db }
        , _workingDirectory{ workingDirectory }
        , _timer{ _ioService }
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
    ShareCleaner::removeOrphanFiles(const std::filesystem::path& directory)
    {
        FS_LOG(SHARE, DEBUG) << "Removing orphan files in directory '" << directory.string() << "'";

        for (const std::filesystem::path& directoryEntry : std::filesystem::directory_iterator{ directory })
        {
            if (!std::filesystem::is_regular_file(directoryEntry))
            {
                FS_LOG(SHARE, DEBUG) << "Skipping '" << directoryEntry.string() << "': not regular";
                continue;
            }

            if (isOrphanFile(directoryEntry))
            {
                std::error_code ec;
                std::filesystem::remove(directoryEntry, ec);
                if (ec)
                {
                    FS_LOG(SHARE, ERROR) << "Cannot remove file '" << directoryEntry.string() << "'";
                }
                else
                {
                    FS_LOG(SHARE, INFO) << "Removed orphan file '" << directoryEntry.string() << "'";
                }
            }
        }
    }

    bool
    ShareCleaner::isOrphanFile(const std::filesystem::path& filePath)
    {
        const std::filesystem::path relativeFilePath{ std::filesystem::relative(filePath, _workingDirectory) };
        assert(!relativeFilePath.empty());

        Wt::Dbo::Session& session{ _db.getTLSSession() };
        Wt::Dbo::Transaction transaction{ session };

        return !File::getByPath(session, relativeFilePath)
            && !File::getByPath(session, filePath);
    }

    void
    ShareCleaner::scheduleNextCheck()
    {
        _timer.expires_after(_checkPeriod);

        _timer.async_wait([this](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted)
                return;

            checkExpiredShares();
            scheduleNextCheck();
        });
    }

    void
    ShareCleaner::checkExpiredShares()
    {
        FS_LOG(SHARE, DEBUG) << "Checking expired shares...";

        const auto now{ Wt::WLocalDateTime::currentServerDateTime().toUTC() };

        Wt::Dbo::Session& session{ _db.getTLSSession() };
        Wt::Dbo::Transaction transaction{ session };

        Share::visitAll(session, [&now](Share::pointer& share) {
            // give some extra time for the share before actually removing it
            // -> make any ongoing downloads a chance to complete before deleting the share
            if (now > share->getExpiryTime().addSecs(3600 * 2))
            {
                FS_LOG(SHARE, INFO) << "Removing expired share '" << share->getUUID().toString() << "'";
                Share::destroy(share);
            }
            else
            {
                FS_LOG(SHARE, DEBUG) << "Share '" << share->getUUID().toString() << "' not due to removal";
            }
        });
    }
} // namespace Share
