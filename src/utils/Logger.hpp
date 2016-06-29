#pragma once

#include <Wt/WServer>
#include <Wt/WApplication>
#include <Wt/WLogger>

#include <string>


enum class Severity
{
	FATAL,
	ERROR,
	WARNING,
	INFO,
	DEBUG,
};

enum class Module
{
	DB,
	DBUPDATER,
	MAIN,
	UI,
};

std::string getModuleName(Module mod);
std::string getSeverityName(Severity sev);

#define FS_LOG(module, level)	Wt::log(getSeverityName(Severity::level)) << Wt::WLogger::sep << "[" << getModuleName(Module::module) << "]" <<  Wt::WLogger::sep

