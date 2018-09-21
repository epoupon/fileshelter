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

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <iomanip>
#include <sstream>

#include "ShareCommon.hpp"

std::unique_ptr<Wt::WAnchor>
createShareDownloadAnchor(Database::Share::pointer share)
{
	std::string downloadPath = "/share-download/" + share->getDownloadUUID();

	return std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, downloadPath), wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + wApp->environment().deploymentPath() + downloadPath);
}

std::unique_ptr<Wt::WAnchor>
createShareEditAnchor(Database::Share::pointer share)
{
	std::string editPath = "/share-edit/" + share->getEditUUID();

	return std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath, editPath), wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + wApp->environment().deploymentPath() + editPath);
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n)
{
	std::ostringstream out;
	out << std::setprecision(n) << std::fixed << a_value;
	return out.str();
}

Wt::WString sizeToString(std::size_t size)
{
	if (size >= 1024 * 1024 * 1024)
	{
		return Wt::WString::tr("msg-size-gb").arg(to_string_with_precision(size / 1024 / 1024 / 1024., 1));
	}
	if (size >= 1024 * 1024)
	{
		return Wt::WString::tr("msg-size-mb").arg(to_string_with_precision(size / 1024 / 1024., 1));
	}
	else if (size >= 1024)
	{
		return Wt::WString::tr("msg-size-kb").arg(to_string_with_precision(size / 1024., 1));
	}
	else
		return Wt::WString::tr("msg-size-b").arg(size);
}

