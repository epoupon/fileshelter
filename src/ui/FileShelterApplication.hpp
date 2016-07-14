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

#include <Wt/WApplication>

#include "database/DbHandler.hpp"

namespace UserInterface {

class FileShelterApplication : public Wt::WApplication
{
	public:
		static Wt::WApplication *create(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool);
		static FileShelterApplication* instance();

		FileShelterApplication(const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& connectionPool);

		// Session application data
		Database::Handler& getDbHandler() { return _db;}

	private:

		Database::Handler	_db;
};

// Helpers to get session data
#define LmsApp	FileShelterApplication::instance()

Database::Handler& DbHandler();
Wt::Dbo::Session& DboSession();

} // namespace UserInterface


