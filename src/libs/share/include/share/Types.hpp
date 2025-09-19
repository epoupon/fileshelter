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

#include <cstdint>
#include <filesystem>

#include <Wt/WDateTime.h>

#include "utils/UUID.hpp"

namespace Share
{
    using FileSize = std::uint64_t;

    class ShareUUID : public UUID
    {
    public:
        using UUID::UUID;
    };

    class ShareEditUUID : public UUID
    {
    public:
        using UUID::UUID;
    };

    class FileUUID : public UUID
    {
    public:
        using UUID::UUID;
    };

    struct FileDesc
    {
        FileUUID uuid;
        std::filesystem::path path;
        std::filesystem::path clientPath;
        FileSize size{};
        bool isOwned{};
    };

    struct ShareDesc
    {
        ShareUUID uuid;
        ShareEditUUID editUuid;
        std::size_t readCount{};
        FileSize size{};
        bool hasPassword{};
        std::string description;
        Wt::WDateTime creationTime;
        Wt::WDateTime expiryTime;
        std::string creatorAddress;
        std::vector<FileDesc> files;
    };
} // namespace Share
