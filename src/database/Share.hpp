#pragma once

#include <boost/filesystem/path.hpp>

#include <Wt/Dbo/Dbo>

namespace Database {

class Share
{

	public:
		typedef Wt::Dbo::ptr<Share> pointer;

		Share();

		// Helpers
		static pointer	create(Wt::Dbo::Session& session);

		static pointer	getByEditUUID(Wt::Dbo::Session& session, std::string UUID);
		static pointer	getByDownloadUUID(Wt::Dbo::Session& session, std::string UUID);
		static pointer	delByEditUUID(Wt::Dbo::Session& session, std::string UUID);

		// Getters
		boost::filesystem::path	getPath(void) const { return _path; }
		std::string		getFileName(void) const { return _filename; }
		std::size_t		getFileSize(void) const { return _filesize; }

		bool hasPassword(void) const { return !_password.empty(); }
		bool verifyPassword(std::string password);
		std::string getDesc(void) const { return _desc; }
		boost::posix_time::ptime getCreationTime(void) const { return _creationTime; }
		boost::posix_time::time_duration getValidityDuration(void) const { return _validityDuration; }
		bool hasExpired(void) const;

		std::size_t getMaxDownloads(void) const { return _nbMaxDownloads; }
		std::size_t getNbDownloads(void) const { return _nbDownloads; }

		std::string getDownloadUUID(void) const { return _downloadUUID; }
		std::string getEditUUID(void) const { return _editUUID; }

		// Setters
		void setPath(boost::filesystem::path path) { _path = path.string(); }
		void setFileName(std::string name) { _filename = name; }
		void setFileSize(std::size_t size) { _filesize = size; }
		void setPassword(std::string password);
		void setDesc(std::string desc) { _desc = desc; }
		void setCreationTime(boost::posix_time::ptime time);
		void setValidityDuration(boost::posix_time::ptime time);
		void setMaxDownloads(std::size_t nbMaxDownloads);



		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _filename,		"filename");
				Wt::Dbo::field(a, _filesize,		"filesize");
				Wt::Dbo::field(a, _checksum,		"checksum");
				Wt::Dbo::field(a, _path,		"path");
				Wt::Dbo::field(a, _password,		"password");
				Wt::Dbo::field(a, _desc,		"desc");
				Wt::Dbo::field(a, _creationTime,	"creation_time");
				Wt::Dbo::field(a, _validityDuration,	"validity_duration");
				Wt::Dbo::field(a, _nbMaxDownloads,	"nb_max_downloads");
				Wt::Dbo::field(a, _nbMaxDownloads,	"nb_downloads");
				Wt::Dbo::field(a, _downloadUUID,	"download_UUID");
				Wt::Dbo::field(a, _editUUID,		"edit_UUID");
			}

	private:

		std::string				_filename;
		long long				_filesize;
		std::string				_checksum;
		std::string				_path;
		std::string				_password;	// optional
		std::string				_desc;		// optional

		boost::posix_time::ptime		_creationTime;
		boost::posix_time::time_duration	_validityDuration;

		int					_nbMaxDownloads; //optional
		int					_nbDownloads;

		std::string		_downloadUUID;
		std::string		_editUUID;


};

} // namespace Database

