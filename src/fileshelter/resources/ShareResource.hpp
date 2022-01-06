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

#include <filesystem>
#include <optional>
#include <string_view>
#include <Wt/WResource.h>
#include <Wt/WLink.h>

#include "share/Types.hpp"

namespace Share
{
	class IShare;
}

class ShareResource : public Wt::WResource
{
	public:
		~ShareResource();

		static void				setDeployPath(std::string_view deployPath) { _deployPath = deployPath; }
		static std::string_view	getDeployPath() { return _deployPath; }
		static Wt::WLink		createLink(const Share::ShareUUID& shareId, std::optional<std::string_view> password);

	private:
		static inline std::string _deployPath;
		void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;
};


