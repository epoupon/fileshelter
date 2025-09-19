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

#include "utils/UUID.hpp"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

UUID::UUID(Generate)
    : _uuid{ boost::uuids::random_generator{}() }
{
}

std::string UUID::toString() const
{
    return boost::uuids::to_string(_uuid);
}

UUID::UUID(std::string_view uuid)
{
    try
    {
        boost::uuids::string_generator gen;
        _uuid = gen(std::string{ uuid });
    }
    catch (const std::runtime_error& e)
    {
        throw UUIDException{ "Invalid UUID format" };
    }
}
