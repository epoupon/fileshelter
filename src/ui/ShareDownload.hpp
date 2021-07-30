/*
 * Copyright (C) 2016 Emeric Poupon
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

#include <Wt/WContainerWidget.h>

#include "share/Types.hpp"

namespace UserInterface
{

	class ShareDownload : public Wt::WContainerWidget
	{
		public:
			ShareDownload();

		private:
			void handlePathChanged();

			void displayPassword(const Share::ShareUUID& shareUUID);
			void displayDownload(const Share::ShareDesc& share, std::optional<std::string_view> password = std::nullopt);
	};

} // namespace UserInterface

