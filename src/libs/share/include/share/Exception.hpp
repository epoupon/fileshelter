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

#include "utils/Exception.hpp"

namespace Share
{
    class Exception : public FsException
    {
        using FsException::FsException;
    };
    class ShareNotFoundException : public Exception
    {
    public:
        ShareNotFoundException()
            : Exception{ "Share not found" } {}
    };
    class FileException : public Exception
    {
    public:
        FileException(const std::filesystem::path& p, std::string_view message)
            : Exception{ "File error on '" + p.string() + "': " + std::string{ message } } {}
    };
    class ShareTooLargeException : public Exception
    {
    public:
        ShareTooLargeException()
            : Exception{ "Share too large" } {}
    };

    class OutOfRangeValidityPeriod : public Exception
    {
    public:
        OutOfRangeValidityPeriod()
            : Exception{ "Validity period out of range" } {}
    };
} // namespace Share
