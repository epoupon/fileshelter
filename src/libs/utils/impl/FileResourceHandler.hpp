/*
 * Copyright (C) 2020 Emeric Poupon
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

#include "utils/IResourceHandler.hpp"
#include <filesystem>

class FileResourceHandler final : public IResourceHandler
{
public:
    FileResourceHandler(const std::filesystem::path& filePath);

private:
    void processRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;
    bool isComplete() const override;
    void abort() override;

    static constexpr std::size_t _chunkSize{ 65536 };

    std::filesystem::path _path;
    ::uint64_t _beyondLastByte{};
    ::uint64_t _offset{};
    bool _isFinished{};
};
