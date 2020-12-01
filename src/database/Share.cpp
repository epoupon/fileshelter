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

#include "Share.hpp"

#include <Wt/Auth/PasswordHash.h>

#include "utils/Logger.hpp"
#include "File.hpp"

namespace Database {

Share::pointer
Share::create(Wt::Dbo::Session& session)
{
	pointer res {session.add(std::make_unique<Share>())};
	session.flush();

	return res;
}

Share::pointer
Share::getByEditUUID(Wt::Dbo::Session& session, const UUID& uuid)
{
	return session.find<Share>().where("edit_UUID = ?").bind(uuid.getAsString());
}

Share::pointer
Share::getById(Wt::Dbo::Session& session, IdType id)
{
	return session.find<Share>().where("id = ?").bind(id);
}

Share::pointer
Share::getByDownloadUUID(Wt::Dbo::Session& session, const UUID& uuid)
{
	return session.find<Share>().where("download_UUID = ?").bind(uuid.getAsString());
}

std::vector<Share::pointer>
Share::getAll(Wt::Dbo::Session& session)
{
	Wt::Dbo::collection<pointer> res = session.find<Share>();
	return std::vector<pointer>(res.begin(), res.end());
}

std::vector<File::pointer>
Share::getFiles() const
{
	return std::vector<File::pointer>(_files.begin(), _files.end());
}

std::vector<IdType>
Share::getFileIds() const
{
	assert(self());
	assert(IdIsValid(self()->id()));
	assert(session());

	Wt::Dbo::collection<IdType> res = session()->query<IdType>("SELECT id FROM file WHERE file.share_id = ?").bind(self()->id());
	return std::vector<IdType>(res.begin(), res.end());
}

Wt::Auth::PasswordHash
Share::getPasswordHash() const
{
	return {_hashFunc, _salt, _password};
}

FileSize
Share::getShareSize() const
{
	assert(self());
	assert(IdIsValid(self()->id()));
	assert(session());

	return session()->query<long long>("SELECT COALESCE(SUM(size), 0) from file WHERE file.share_id = ?").bind(self()->id());
}

void
Share::setPasswordHash(const Wt::Auth::PasswordHash& hash)
{
	_password = hash.value();
	_salt = hash.salt();
	_hashFunc = hash.function();
}

void
Share::incHits()
{
	for (auto& file : _files)
		file.modify()->incHits();
}

} // namespace Database

