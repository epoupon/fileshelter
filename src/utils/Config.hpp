#pragma once

#include <boost/filesystem.hpp>
#include <libconfig.h++>

// Used to get config values from configuration files
class Config
{
	public:

		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;

		static Config& instance();

		void		setFile(boost::filesystem::path p);

		// Default values are returned in case of setting not found
		std::string	getString(std::string setting, std::string def = "");
		unsigned long	getULong(std::string setting, unsigned long def = 0);
		long		getLong(std::string setting, long def = 0);
		bool		getBool(std::string setting, bool def = false);

	private:

		Config();

		libconfig::Config	*_config;
};

