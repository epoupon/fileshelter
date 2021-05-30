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

namespace Database
{
	File::File(std::string_view fileName,
			FileSize fileSize,
			const std::filesystem::path& filePath,
			const UUID& uuid,
			const Wt::Dbo::ptr<Share>& share)
	: _name {fileName}
	, _size {static_cast<long long>(fileSize)}
	, _path {filePath}
	, _downloadUUID {uuid.getAsString()}
	, _share {share}
	{}

	File::pointer
	File::create(Wt::Dbo::Session& session,
			std::string_view fileName,
			FileSize fileSize,
			const std::filesystem::path& filePath,
			const UUID& uuid,
			Wt::Dbo::ptr<Share>& share)
	{
		FS_LOG(DB, DEBUG) << "Created file '" << std::string {fileName} << "' with UUID '" << uuid.getAsString() << "' on share " << share.id();

		pointer res {session.add(std::make_unique<File>(fileName, fileSize, filePath, uuid, share))};
		session.flush();

		return res;
	}

	File::pointer
	File::getById(Wt::Dbo::Session& session, IdType id)
	{
		return session.find<File>().where("id = ?").bind(id);
	}

	File::pointer
	File::getByDownloadUUID(Wt::Dbo::Session& session, const UUID& downloadUUID)
	{
		return session.find<File>().where("download_UUID = ?").bind(downloadUUID.getAsString());
	}
}

