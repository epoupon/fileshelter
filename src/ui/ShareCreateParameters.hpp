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

#include <vector>

#include <Wt/Http/Request.h>
#include <Wt/WDateTime.h>
#include <Wt/WString.h>

namespace UserInterface
{
	struct Duration
	{
		enum class Unit
		{
			Hours,
			Days,
			Weeks,
			Months,
			Years,
		};

		std::size_t value {};
		Unit unit {Unit::Hours};
	};

	struct ShareCreateParameters
	{
		Wt::WString	description;
		Duration	maxDuration;
		Wt::WString	password;
		std::vector<const Wt::Http::UploadedFile*> uploadedFiles;
	};

	Wt::WDateTime operator+(const Wt::WDateTime& dateTime, const Duration& duration);
}

