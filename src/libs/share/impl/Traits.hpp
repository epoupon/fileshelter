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

#include <filesystem>

#include <Wt/Dbo/WtSqlTraits.h>

#include "share/Types.hpp"

namespace Wt::Dbo
{
    template<>
    struct sql_value_traits<fs::UUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size);
        static void bind(const fs::UUID& v, SqlStatement* statement, int column, int size);
        static bool read(fs::UUID& v, SqlStatement* statement, int column, int size);
    };

    template<>
    struct sql_value_traits<fs::share::ShareUUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size) { return sql_value_traits<fs::UUID, void>::type(conn, size); }
        static void bind(const fs::share::ShareUUID& v, SqlStatement* statement, int column, int size) { sql_value_traits<fs::UUID, void>::bind(v, statement, column, size); }
        static bool read(fs::share::ShareUUID& v, SqlStatement* statement, int column, int size) { return sql_value_traits<fs::UUID, void>::read(v, statement, column, size); }
    };

    template<>
    struct sql_value_traits<fs::share::ShareEditUUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size) { return sql_value_traits<fs::UUID, void>::type(conn, size); }
        static void bind(const fs::share::ShareEditUUID& v, SqlStatement* statement, int column, int size) { sql_value_traits<fs::UUID, void>::bind(v, statement, column, size); }
        static bool read(fs::share::ShareEditUUID& v, SqlStatement* statement, int column, int size) { return sql_value_traits<fs::UUID, void>::read(v, statement, column, size); }
    };

    template<>
    struct sql_value_traits<fs::share::FileUUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size) { return sql_value_traits<fs::UUID, void>::type(conn, size); }
        static void bind(const fs::share::FileUUID& v, SqlStatement* statement, int column, int size) { sql_value_traits<fs::UUID, void>::bind(v, statement, column, size); }
        static bool read(fs::share::FileUUID& v, SqlStatement* statement, int column, int size) { return sql_value_traits<fs::UUID, void>::read(v, statement, column, size); }
    };

    template<>
    struct sql_value_traits<std::filesystem::path, void>
    {
        static const bool specialized = true;

        static std::string type(SqlConnection* conn, int size);
        static void bind(const std::filesystem::path& path, SqlStatement* statement, int column, int size);
        static bool read(std::filesystem::path& v, SqlStatement* statement, int column, int size);
    };

    template<>
    struct sql_value_traits<fs::share::FileSize, void>
    {
        static const bool specialized = true;

        static std::string type(SqlConnection* conn, int size);
        static void bind(fs::share::FileSize v, SqlStatement* statement, int column, int size);
        static bool read(fs::share::FileSize& v, SqlStatement* statement, int column, int size);
    };

} // namespace Wt::Dbo
