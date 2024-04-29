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

#include "ShareUtils.hpp"

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <sstream>
#include <iomanip>

namespace UserInterface::ShareUtils
{
    
    static
    std::string
    getScheme()
    {
        // Necessary because the scheme in envonment does not pay attention to X-Forwarded_Proto by default
        //and we have to use that beause you cannot get the full URL out of wLink as text
        const std::string xForwardedProto = wApp->environment().headerValue("X-Forwarded-Proto");
        if (!xForwardedProto.empty())
        {
            return xForwardedProto;  // will be 'http' or 'https'
        }

        // Fallback if header is not set
        return wApp->environment().urlScheme();
    }
    
	static
	std::string
	computeURL(const std::string& internalPath)
	{
		return getScheme() + "://" + wApp->environment().hostName() + (wApp->environment().deploymentPath() == "/" ? "" : wApp->environment().deploymentPath()) + internalPath;
	}

	std::unique_ptr<Wt::WAnchor>
	createShareDownloadAnchor(const Share::ShareUUID& shareUUID)
	{
		const std::string downloadPath {"/share-download/" + shareUUID.toString()};

		return std::make_unique<Wt::WAnchor>(Wt::WLink {Wt::LinkType::InternalPath, downloadPath}, computeURL(downloadPath));
	}

	std::unique_ptr<Wt::WAnchor>
	createShareEditAnchor(const Share::ShareEditUUID& shareEditUUID)
	{
		const std::string editPath {"/share-edit/" + shareEditUUID.toString()};

		return std::make_unique<Wt::WAnchor>(Wt::WLink {Wt::LinkType::InternalPath, editPath}, computeURL(editPath));
	}

	template <typename T>
	std::string to_string_with_precision(const T a_value, const int n)
	{
		std::ostringstream out;
		out << std::setprecision(n) << std::fixed << a_value;
		return out.str();
	}

	Wt::WString
	fileSizeToString(Share::FileSize size)
	{
		if (size >= 1024 * 1024 * 1024)
		{
			return Wt::WString::tr("msg-size-gb").arg(to_string_with_precision(size / 1024 / 1024 / 1024., 1));
		}
		if (size >= 1024 * 1024)
		{
			return Wt::WString::tr("msg-size-mb").arg(to_string_with_precision(size / 1024 / 1024., 0));
		}
		else if (size >= 1024)
		{
			return Wt::WString::tr("msg-size-kb").arg(to_string_with_precision(size / 1024., 0));
		}
		else
			return Wt::WString::tr("msg-size-b").arg(size);
	}

} // ns UserInterface::ShareUtils
