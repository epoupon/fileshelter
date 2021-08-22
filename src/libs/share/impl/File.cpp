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

#include "File.hpp"

#include "utils/Logger.hpp"
#include "Share.hpp"
#include "Types.hpp"

namespace Share
{
	File::pointer
	File::create(Wt::Dbo::Session& session, const FileCreateParameters& parameters, Share::pointer share)
	{
		pointer res {session.add(std::make_unique<File>())};

		res.modify()->_path = parameters.path;
		res.modify()->_name = parameters.name;
		res.modify()->_size = std::filesystem::file_size(parameters.path); // may throw
		res.modify()->_share = share;

		session.flush();
		return res;

	}

/*	File::pointer
	File::getByUUID(Wt::Dbo::Session& session, const FileUUID& uuid)
	{
		return session.find<File>().where("uuid = ?").bind(uuid);
	}

	File::pointer
	File::getByDownloadUUID(Wt::Dbo::Session& session, const UUID& downloadUUID)
	{
		return session.find<File>().where("download_UUID = ?").bind(downloadUUID.getAsString());
	}
	*/
}

