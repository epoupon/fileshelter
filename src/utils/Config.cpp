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

#include <sstream>

#include "Config.hpp"

namespace {

}

Config::Config()
: _config (nullptr)
{
}

Config::~Config()
{
	if (_config != nullptr)
		delete _config;
}

Config&
Config::instance()
{
	static Config instance;
	return instance;
}

void
Config::setFile(const std::filesystem::path& p)
{
	if (_config != nullptr)
		delete _config;

	_config = new libconfig::Config();

	_config->readFile(p.string().c_str());
}

std::string
Config::getString(const std::string& setting, const std::string& def)
{
	try {
		return _config->lookup(setting);
	}
	catch (std::exception &e)
	{
		return def;
	}
}

std::optional<std::filesystem::path>
Config::getOptPath(const std::string& setting)
{
	try {
		const char* res = _config->lookup(setting);
		return std::filesystem::path {res};
	}
	catch (std::exception &e)
	{
		return std::nullopt;
	}
}

std::filesystem::path
Config::getPath(const std::string& setting, const std::filesystem::path& path)
{
	try {
		const char* res = _config->lookup(setting);
		return std::filesystem::path {std::string(res)};
	}
	catch (std::exception &e)
	{
		return path;
	}
}

unsigned long
Config::getULong(const std::string& setting, unsigned long def)
{
	try {
		return static_cast<unsigned int>(_config->lookup(setting));
	}
	catch (...)
	{
		return def;
	}
}

long
Config::getLong(const std::string& setting, long def)
{
	try {
		return _config->lookup(setting);
	}
	catch (...)
	{
		return def;
	}
}

bool
Config::getBool(const std::string& setting, bool def)
{
	try {
		return _config->lookup(setting);
	}
	catch (...)
	{
		return def;
	}
}


