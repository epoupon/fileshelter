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

#pragma once

#include <filesystem>
#include <memory>
#include <mutex>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/SqlConnectionPool.h>

namespace Share
{
    class Db
    {
    public:
        Db(const std::filesystem::path& db);

        Db(const Db&) = delete;
        Db(Db&&) = delete;
        Db& operator=(const Db&) = delete;
        Db& operator=(Db&&) = delete;

        Wt::Dbo::Session& getTLSSession();

    private:
        void prepare();
        void doMigrationIfNeeded(Wt::Dbo::Session& session);
        std::unique_ptr<Wt::Dbo::Session> createSession();

        std::unique_ptr<Wt::Dbo::SqlConnectionPool> _connectionPool;
        std::mutex _tlsSessionsMutex;
        std::vector<std::unique_ptr<Wt::Dbo::Session>> _tlsSessions;
    };
} // namespace Share
