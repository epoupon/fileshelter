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

#include <boost/filesystem.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/SqlConnectionPool>

namespace Database {

class Handler
{
	public:

		Handler(Wt::Dbo::SqlConnectionPool& connectionPool);

		Wt::Dbo::Session& getSession() { return _session; }

		static Wt::Dbo::SqlConnectionPool*	createConnectionPool(boost::filesystem::path db);

	private:

		Wt::Dbo::Session		_session;
};

} // namespace Database


