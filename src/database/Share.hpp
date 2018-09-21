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

#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

namespace Database {

class Share
{
	public:
		using pointer = Wt::Dbo::ptr<Share>;

		Share();

		static std::size_t getMaxFileSize();
		static std::chrono::seconds getMaxValidatityDuration();
		static std::chrono::seconds getDefaultValidatityDuration();
		static bool userCanSetValidatityDuration();

		static std::size_t getMaxValidatityHits();
		static std::size_t getDefaultValidatityHits();
		static bool userCanSetValidatityHits();

		// Helpers
		// Create a new share, will move the underlying file in the working directory
		static pointer	create(Wt::Dbo::Session& session, boost::filesystem::path file);

		// Remove the underlying file, must call remove on dbo object after that
		void	destroy();

		static pointer	getByEditUUID(Wt::Dbo::Session& session, std::string UUID);
		static pointer	getByDownloadUUID(Wt::Dbo::Session& session, std::string UUID);
		static pointer	delByEditUUID(Wt::Dbo::Session& session, std::string UUID);

		static std::vector<pointer> getAll(Wt::Dbo::Session& session);

		// Getters
		boost::filesystem::path		getPath(void) const;
		std::string			getFileName(void) const { return _filename; }
		std::size_t			getFileSize(void) const { return _filesize; }
		bool				hasPassword(void) const { return !_password.empty(); }
		bool				verifyPassword(Wt::WString password) const;
		std::string			getDesc(void) const { return _desc; }
		Wt::WDateTime			getCreationTime(void) const { return _creationTime; }
		Wt::WDateTime			getExpiryTime(void) const { return _expiryTime; }
		bool				hasExpired(void) const;
		std::size_t			getMaxHits(void) const { return _maxHits; }
		std::size_t			getHits(void) const { return _hits; }
		std::string			getDownloadUUID(void) const { return _downloadUUID; }
		std::string			getEditUUID(void) const { return _editUUID; }
		std::string			getClientAddr(void) const { return _clientAddress; }

		// Setters
		void setFileName(std::string name) { _filename = name; }
		void setFileSize(std::size_t size) { _filesize = size; }
		void setPassword(Wt::WString password);
		void setDesc(std::string desc) { _desc = desc; }
		void setCreationTime(Wt::WDateTime time) { _creationTime = time; }
		void setValidityDuration(Wt::WDateTime time);
		void setMaxHits(std::size_t maxHits)	{ _maxHits = maxHits; }
		void incHits()				{ _hits++; }
		void setExpiryTime(Wt::WDateTime expiryTime) { _expiryTime = expiryTime; }
		void setClientAddr(std::string addr) { _clientAddress = addr; }


		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _filename,		"filename");
				Wt::Dbo::field(a, _filesize,		"filesize");
				Wt::Dbo::field(a, _checksum,		"checksum");
				Wt::Dbo::field(a, _clientAddress,	"client_addr");
				Wt::Dbo::field(a, _password,		"password");
				Wt::Dbo::field(a, _salt,		"salt");
				Wt::Dbo::field(a, _hashFunc,		"hash_func");
				Wt::Dbo::field(a, _desc,		"desc");
				Wt::Dbo::field(a, _creationTime,	"creation_time");
				Wt::Dbo::field(a, _expiryTime,		"expiry_time");
				Wt::Dbo::field(a, _maxHits,		"max_hits");
				Wt::Dbo::field(a, _hits,		"hits");
				Wt::Dbo::field(a, _downloadUUID,	"download_UUID");
				Wt::Dbo::field(a, _editUUID,		"edit_UUID");
			}

	private:

		std::string	_filename;
		long long	_filesize = 0;
		std::string	_checksum;
		std::string	_clientAddress;	// Client IP address that uploaded the file
		std::string	_password;	// optional
		std::string	_salt;
		std::string	_hashFunc;
		std::string	_desc;		// optional

		Wt::WDateTime	_creationTime;
		Wt::WDateTime	_expiryTime;

		int		_hits = 0;
		int		_maxHits = 0;	//optional

		std::string		_downloadUUID;
		std::string		_editUUID;


};

} // namespace Database

