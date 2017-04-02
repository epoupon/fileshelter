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

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <libconfig.h++>

// Used to get config values from configuration files
class Config
{
	public:

		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;
		~Config();

		static Config& instance();

		void		setFile(boost::filesystem::path p);

		// Default values are returned in case of setting not found
		std::string	getString(std::string setting, std::string def = "");
		boost::optional<boost::filesystem::path> getOptPath(std::string setting);
		unsigned long	getULong(std::string setting, unsigned long def = 0);
		long		getLong(std::string setting, long def = 0);
		bool		getBool(std::string setting, bool def = false);

		// Optional versions
		boost::filesystem::path getPath(std::string setting, boost::filesystem::path def = boost::filesystem::path());

	private:

		Config();

		libconfig::Config	*_config;
};

