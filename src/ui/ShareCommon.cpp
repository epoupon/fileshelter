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
#include <Wt/WApplication>
#include <Wt/WEnvironment>

#include "ShareCommon.hpp"

Wt::WAnchor* createShareDownloadAnchor(Database::Share::pointer share)
{
	std::string downloadPath = "/share-download/" + share->getDownloadUUID();
	return new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, downloadPath), wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + downloadPath);
}

Wt::WAnchor* createShareEditAnchor(Database::Share::pointer share)
{
	std::string editPath = "/share-edit/" + share->getEditUUID();

	return new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, editPath), wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + editPath);
}

