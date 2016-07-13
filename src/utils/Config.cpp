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
Config::setFile(boost::filesystem::path p)
{
	if (_config != nullptr)
		delete _config;

	_config = new libconfig::Config();

	_config->readFile(p.string().c_str());
}

std::string
Config::getString(std::string setting, std::string def)
{
	try {
		return _config->lookup(setting);
	}
	catch (std::exception &e)
	{
		return def;
	}
}

boost::filesystem::path
Config::getPath(std::string setting, boost::filesystem::path path)
{
	try {
		const char* res = _config->lookup(setting);
		return boost::filesystem::path(std::string(res));
	}
	catch (std::exception &e)
	{
		return path;
	}
}

unsigned long
Config::getULong(std::string setting, unsigned long def)
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
Config::getLong(std::string setting, long def)
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
Config::getBool(std::string setting, bool def)
{
	try {
		return _config->lookup(setting);
	}
	catch (...)
	{
		return def;
	}
}


