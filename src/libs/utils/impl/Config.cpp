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

#include "Config.hpp"

#include "utils/Exception.hpp"

std::unique_ptr<IConfig> createConfig(const std::filesystem::path& p)
{
    return std::make_unique<Config>(p);
}

Config::Config(const std::filesystem::path& p)
{
    try
    {
        _config.readFile(p.string().c_str());
    }
    catch (libconfig::FileIOException& e)
    {
        throw FsException{ "Cannot open config file '" + p.string() + "'" };
    }
    catch (libconfig::ParseException& e)
    {
        throw FsException{ "Cannot parse config file '" + p.string() + "', line = " + std::to_string(e.getLine()) + ", error = '" + e.getError() + "'" };
    }
    catch (libconfig::ConfigException& e)
    {
        throw FsException{ "Cannot open config file '" + p.string() + "': " + e.what() };
    }
}

std::string_view Config::getString(std::string_view setting, std::string_view def)
{
    try
    {
        return static_cast<const char*>(_config.lookup(std::string{ setting }));
    }
    catch (libconfig::ConfigException&)
    {
        return def;
    }
}

void Config::visitStrings(std::string_view setting, std::function<void(std::string_view)> _func, std::initializer_list<std::string_view> defs)
{
    try
    {
        const libconfig::Setting& values{ _config.lookup(std::string{ setting }) };
        for (int i{}; i < values.getLength(); ++i)
            _func(static_cast<const char*>(values[i]));
    }
    catch (const libconfig::SettingNotFoundException&)
    {
        for (std::string_view def : defs)
            _func(def);
    }
    catch (libconfig::ConfigException&)
    {
    }
}

std::filesystem::path Config::getPath(std::string_view setting, const std::filesystem::path& path)
{
    try
    {
        const char* res{ _config.lookup(std::string{ setting }) };
        return std::filesystem::path{ std::string(res) };
    }
    catch (libconfig::ConfigException&)
    {
        return path;
    }
}

unsigned long Config::getULong(std::string_view setting, unsigned long def)
{
    try
    {
        return static_cast<unsigned int>(_config.lookup(std::string{ setting }));
    }
    catch (libconfig::ConfigException&)
    {
        return def;
    }
}

long Config::getLong(std::string_view setting, long def)
{
    try
    {
        return _config.lookup(std::string{ setting });
    }
    catch (libconfig::ConfigException&)
    {
        return def;
    }
}

bool Config::getBool(std::string_view setting, bool def)
{
    try
    {
        return _config.lookup(std::string{ setting });
    }
    catch (libconfig::ConfigException&)
    {
        return def;
    }
}
