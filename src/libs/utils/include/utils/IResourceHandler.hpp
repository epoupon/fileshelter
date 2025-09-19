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

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

// Helper class to serve a resource (must be saved as continuation data if not complete)
class IResourceHandler
{
public:
    virtual ~IResourceHandler() = default;

    virtual void processRequest(const Wt::Http::Request& request, Wt::Http::Response& response) = 0;
    [[nodiscard]] virtual bool isComplete() const = 0;
    virtual void abort() = 0;
};
