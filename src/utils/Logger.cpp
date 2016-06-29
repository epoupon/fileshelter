#include "Logger.hpp"

std::string getModuleName(Module mod)
{
	switch (mod)
	{
		case Module::DB:		return "DB";
		case Module::DBUPDATER:		return "DB UPDATER";
		case Module::MAIN:		return "MAIN";
		case Module::UI:		return "UI";
	}
	return "";
}

std::string getSeverityName(Severity sev)
{
	switch (sev)
	{
		case Severity::FATAL:		return "fatal";
		case Severity::ERROR:		return "error";
		case Severity::WARNING:		return "warning";
		case Severity::INFO:		return "info";
		case Severity::DEBUG:		return "debug";
	}
	return "";
}

