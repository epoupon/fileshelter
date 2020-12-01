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

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include "utils/UUID.hpp"
#include "Types.hpp"


namespace Auth {
	class PasswordHash;
}

namespace Database {

class File;

class Share : public Wt::Dbo::Dbo<Share>
{
	public:
		using pointer = Wt::Dbo::ptr<Share>;

		enum class State
		{
			Init,		// can't be downloaded, can be modified
			Ready,		// can be download, can't be modified
			Deleting,	// can't be downloaded, being deleted
		};

		// Helpers
		static pointer	create(Wt::Dbo::Session& session);

		static pointer	getById(Wt::Dbo::Session& session, IdType shareId);
		static pointer	getByEditUUID(Wt::Dbo::Session& session, const UUID& uuid);
		static pointer	getByDownloadUUID(Wt::Dbo::Session& session, const UUID& uuid);

		static std::vector<pointer> getAll(Wt::Dbo::Session& session);

		// Getters
		State				getState() const { return _state; }
		FileSize			getShareSize() const;
		bool				hasPassword() const { return !_password.empty(); }
		std::string_view	getDesc() const { return _desc; }
		Wt::WDateTime		getCreationTime() const { return _creationTime; }
		Wt::WDateTime		getExpiryTime() const { return _expiryTime; }
		UUID				getDownloadUUID() const { return *UUID::fromString(_downloadUUID); }
		UUID				getEditUUID() const { return *UUID::fromString(_editUUID); }
		std::string_view	getClientAddr() const { return _clientAddress; }
		std::vector<Wt::Dbo::ptr<File>>	getFiles() const;
		std::vector<IdType>	getFileIds() const;
		std::size_t			getFileCount() const;
		Wt::Auth::PasswordHash getPasswordHash() const;

		// Setters
		void setDownloadUUID(const UUID& uuid) { _downloadUUID = uuid.getAsString(); }
		void setEditUUID(const UUID& uuid) { _editUUID = uuid.getAsString(); }
		void setState(State state) { _state = state; }
		void setShareName(std::string_view name) { _shareName = name; }
		void setPasswordHash(const Wt::Auth::PasswordHash& hash);
		void setDesc(std::string_view desc) { _desc = desc; }
		void setCreationTime(Wt::WDateTime time) { _creationTime = time; }
		void setValidityDuration(Wt::WDateTime time);
		void setExpiryTime(Wt::WDateTime expiryTime) { _expiryTime = expiryTime; }
		void setClientAddr(std::string_view addr) { _clientAddress = addr; }
		void incHits();

		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _state,			"state");
				Wt::Dbo::field(a, _shareName,		"share_name");
				Wt::Dbo::field(a, _clientAddress,	"client_addr");
				Wt::Dbo::field(a, _password,		"password");
				Wt::Dbo::field(a, _salt,			"salt");
				Wt::Dbo::field(a, _hashFunc,		"hash_func");
				Wt::Dbo::field(a, _desc,			"desc");
				Wt::Dbo::field(a, _creationTime,	"creation_time");
				Wt::Dbo::field(a, _expiryTime,		"expiry_time");
				Wt::Dbo::field(a, _downloadUUID,	"download_UUID");
				Wt::Dbo::field(a, _editUUID,		"edit_UUID");

				Wt::Dbo::hasMany(a, _files, Wt::Dbo::ManyToOne, "share");
			}

	private:

		State		_state {State::Init};
		std::string	_shareName;
		std::string	_clientAddress;		// Client IP address that uploaded the file
		std::string	_password;			// optional
		std::string	_salt;
		std::string	_hashFunc;
		std::string	_desc;				// optional

		Wt::WDateTime	_creationTime;
		Wt::WDateTime	_expiryTime;

		std::string		_downloadUUID;
		std::string		_editUUID;

		Wt::Dbo::collection<Wt::Dbo::ptr<File>> _files;


};

} // namespace Database

