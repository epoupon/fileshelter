/*
 * Copyright (C) 2023 Emeric Poupon
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

#include "Common.hpp"

#include "share/Types.hpp"
#include <iostream>

void displayShareDesc(const Share::ShareDesc& share, bool details, std::string_view deployURL)
{
    std::cout << "Share '" << share.uuid.toString()
              << "', EditUUID = '" << share.editUuid.toString()
              << "', expires " << share.expiryTime.toString() << " UTC, "
              << "created by '" << share.creatorAddress << "', "
              << share.size << " bytes" << ", "
              << share.files.size() << " file" << (share.files.size() == 1 ? "" : "s") << ", "
              << share.readCount << " download" << (share.readCount > 1 ? "s" : "") << std::endl;
    if (!details)
        return;

    if (!deployURL.empty())
    {
        std::cout << "\tDownload URL: " << deployURL << "/share-download/" << share.uuid.toString() << std::endl;
        std::cout << "\tEdit URL: " << deployURL << "/share-edit/" << share.editUuid.toString() << std::endl;
    }

    for (const Share::FileDesc& file : share.files)
    {
        std::cout << "\tFile '" << file.path.string() << "'" << (file.isOwned ? " (owned)" : "") << ", '" << file.clientPath.string() << "', " << file.size << " bytes" << std::endl;
    }
}
