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
#include <string_view>

#include <Wt/Dbo/WtSqlTraits.h>

#include "share/Types.hpp"

namespace Wt::Dbo
{
    template<>
    struct sql_value_traits<UUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size);
        static void bind(const UUID& v, SqlStatement* statement, int column, int size);
        static bool read(UUID& v, SqlStatement* statement, int column, int size);
    };

    template<>
    struct sql_value_traits<Share::ShareUUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size) { return sql_value_traits<UUID, void>::type(conn, size); }
        static void bind(const Share::ShareUUID& v, SqlStatement* statement, int column, int size) { sql_value_traits<UUID, void>::bind(v, statement, column, size); }
        static bool read(Share::ShareUUID& v, SqlStatement* statement, int column, int size) { return sql_value_traits<UUID, void>::read(v, statement, column, size); }
    };

    template<>
    struct sql_value_traits<Share::ShareEditUUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size) { return sql_value_traits<UUID, void>::type(conn, size); }
        static void bind(const Share::ShareEditUUID& v, SqlStatement* statement, int column, int size) { sql_value_traits<UUID, void>::bind(v, statement, column, size); }
        static bool read(Share::ShareEditUUID& v, SqlStatement* statement, int column, int size) { return sql_value_traits<UUID, void>::read(v, statement, column, size); }
    };

    template<>
    struct sql_value_traits<Share::FileUUID, void>
    {
        static const bool specialized = true;

        static const char* type(SqlConnection* conn, int size) { return sql_value_traits<UUID, void>::type(conn, size); }
        static void bind(const Share::FileUUID& v, SqlStatement* statement, int column, int size) { sql_value_traits<UUID, void>::bind(v, statement, column, size); }
        static bool read(Share::FileUUID& v, SqlStatement* statement, int column, int size) { return sql_value_traits<UUID, void>::read(v, statement, column, size); }
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
    struct sql_value_traits<Share::FileSize, void>
    {
        static const bool specialized = true;

        static std::string type(SqlConnection* conn, int size);
        static void bind(Share::FileSize v, SqlStatement* statement, int column, int size);
        static bool read(Share::FileSize& v, SqlStatement* statement, int column, int size);
    };

} // namespace Wt::Dbo
