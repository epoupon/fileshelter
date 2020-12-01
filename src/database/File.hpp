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
#include <string>
#include <string_view>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include "utils/UUID.hpp"
#include "Types.hpp"

namespace Database {

class Share;
class File
{
	public:
		using pointer = Wt::Dbo::ptr<File>;

		static std::size_t getMaxFileSize();

		File() = default;
		File(std::string_view fileName,
				FileSize fileSize,
				const std::filesystem::path& filePath,
				const UUID& uuid,
				const Wt::Dbo::ptr<Share>& share);

		// Helpers
		static pointer	create(Wt::Dbo::Session& session,
				std::string_view fileName, 				// client file name
				FileSize fileSize,
				const std::filesystem::path& filePath,	// path where the file is stored
				const UUID& uuid,
				Wt::Dbo::ptr<Share>& share);			// share that owns this file
		static pointer	getById(Wt::Dbo::Session& session, IdType fileId);

		// Setters
		void incHits()				{ _hits++; }

		// Getters
		const std::string&			getName() const { return _name; }
		FileSize					getSize() const { return _size; }
		std::filesystem::path		getPath() const { return _path; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, _name,			"name");
			Wt::Dbo::field(a, _size,			"size");
			Wt::Dbo::field(a, _path,			"path");
			Wt::Dbo::field(a, _hits,			"hits");
			Wt::Dbo::field(a, _downloadUUID,	"download_UUID");

			Wt::Dbo::belongsTo(a, _share, "share",	Wt::Dbo::OnDeleteCascade);
		}

	private:

		std::string	_name;
		long long	_size {};
		std::string	_path;
		long long	_hits {};
		std::string	_downloadUUID;

		Wt::Dbo::ptr<Share> _share;

};

} // namespace Database

