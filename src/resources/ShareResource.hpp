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

#include <optional>
#include <string_view>

#include <Wt/WResource.h>
#include <Wt/WLink.h>

#include "utils/UUID.hpp"

namespace Database
{
	class Db;
}

class ShareResource : public Wt::WResource
{
	public:
		ShareResource(Database::Db& db);
		~ShareResource();

		ShareResource(const ShareResource&) = delete;
		ShareResource(ShareResource&&) = delete;
		ShareResource& operator=(const ShareResource&) = delete;
		ShareResource& operator=(ShareResource&&) = delete;

		static std::string_view getDeployPath();
		static Wt::WLink createLink(const UUID& uuid, std::optional<std::string_view> password = std::nullopt);

	private:

		void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;

		static constexpr std::size_t _bufferSize {32768};
		Database::Db& _db;
};


