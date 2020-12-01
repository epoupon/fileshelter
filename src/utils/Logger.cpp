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

#include "Logger.hpp"

std::string getModuleName(Module mod)
{
	switch (mod)
	{
		case Module::DB:		return "DB";
		case Module::DBUPDATER:		return "DB UPDATER";
		case Module::MAIN:		return "MAIN";
		case Module::SHARE:		return "SHARE";
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

