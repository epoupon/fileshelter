#pragma once

#include <boost/filesystem/path.hpp>

#include <Wt/Dbo/Dbo>

namespace Database {

class File
{

	public:

		File();

		// Getters
		boost::filesystem::path getPath(void) const { return _path; }
		std::string getName(void) const { return _filename; }
		bool hasPassword(void) const { return !_password.empty(); }
		bool verifyPassword(std::string password);
		std::string getDesc(void) const { return _desc; }
		boost::posix_time::ptime getCreationTime(void) const { return _creationTime; }
		boost::posix_time::time_duration getValidityDuration(void) const { return _validityDuration; }

		std::size_t getMaxDownloads(void) const { return _nbMaxDownloads; }
		std::size_t getNbDownloads(void) const { return _nbDownloads; }

		std::string getDownloadUID(void) const { return _downloadUID; }

		// Setters
		void setPath(boost::filesystem::path path);
		void setName(std::string name);
		void setPassword(std::string password);
		void setDesc(std::string desc);
		void setCreationTime(boost::posix_time::ptime time);
		void setValidityDuration(boost::posix_time::ptime time);
		void setMaxDownloads(std::size_t nbMaxDownloads);



		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _filename,		"filename");
				Wt::Dbo::field(a, _path,		"path");
				Wt::Dbo::field(a, _password,		"password");
				Wt::Dbo::field(a, _desc,		"desc");
				Wt::Dbo::field(a, _creationTime,	"creation_time");
				Wt::Dbo::field(a, _validityDuration,	"validity_duration");
				Wt::Dbo::field(a, _nbMaxDownloads,	"nb_max_downloads");
				Wt::Dbo::field(a, _nbMaxDownloads,	"nb_downloads");
				Wt::Dbo::field(a, _downloadUID,		"download_UID");
				Wt::Dbo::field(a, _editUID,		"edit_UID");
			}

	private:

		std::string				_filename;
		std::string				_path;
		std::string				_password;	// optional
		std::string				_desc;		// optional

		boost::posix_time::ptime		_creationTime;
		boost::posix_time::time_duration	_validityDuration;

		int					_nbMaxDownloads; //optional
		int					_nbDownloads;

		std::string		_downloadUID;
		std::string		_editUID;


};

} // namespace Database

