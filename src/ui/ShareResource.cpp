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

#include "ShareResource.hpp"

#include <fstream>

#include <Wt/WEnvironment.h>
#include <Wt/Http/Response.h>

#include "database/Share.hpp"
#include "utils/Logger.hpp"

#include "FileShelterApplication.hpp"


namespace UserInterface {

ShareResource::ShareResource(std::string downloadUUID)
  : _downloadUUID(downloadUUID)
{
}

ShareResource::~ShareResource()
{
  beingDeleted();
}

void
ShareResource::handleRequest(const Wt::Http::Request& request,
				  Wt::Http::Response& response)
{
	if (!request.continuation())
	{
		// Sessions are not thread safe in resources
		Wt::WApplication::UpdateLock lock(wApp);

		FS_LOG(UI, INFO) << "[" << _downloadUUID << "] New download from " << wApp->environment().clientAddress();

		Wt::Dbo::Transaction transaction(FsApp->getDboSession());

		auto share = Database::Share::getByDownloadUUID(FsApp->getDboSession(), _downloadUUID);
		if (!share)
		{
			FS_LOG(UI, WARNING) <<  "[" << _downloadUUID << "] Share does not exist!";
			return;
		}

		FS_LOG(UI, INFO) << "[" << _downloadUUID << "] Hits = " << share->getHits() << " / " << share->getMaxHits();

		if (share->hasExpired())
		{
			FS_LOG(UI, WARNING) <<  "[" << _downloadUUID << "] Share expired!";
			response.setStatus(404);
			return;
		}

		suggestFileName(Wt::WString::fromUTF8(share->getFileName()));
		_path = share->getPath();
		share.modify()->incHits();
	}
	else
	{
		if (_path.empty())
		{
			FS_LOG(UI, ERROR) << "Path not set!";
			return;
		}
	}

	std::ifstream is(_path.string().c_str(), std::ios::in | std::ios::binary);
	if (!is)
	{
		FS_LOG(UI, ERROR) << "Cannot open " << _path.string();
		return;
	}

	handleRequestPiecewise(request, response, is);

	if (!request.continuation())
	{
		FS_LOG(UI, INFO) << "[" << _downloadUUID << "] Download complete from " << wApp->environment().clientAddress();
	}
}

}
