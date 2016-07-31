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

#include <fstream>
#include <Wt/Http/Response>

#include "database/Share.hpp"
#include "utils/Logger.hpp"

#include "FileShelterApplication.hpp"

#include "ShareResource.hpp"


namespace UserInterface {

ShareResource::ShareResource(std::string downloadUUID, Wt::WObject *parent)
  : WStreamResource(parent),
    _downloadUUID(downloadUUID)
{
}

ShareResource::~ShareResource(void)
{
  beingDeleted();
}

void
ShareResource::handleRequest(const Wt::Http::Request& request,
				  Wt::Http::Response& response)
{
	if (!request.continuation())
	{
		FS_LOG(UI, INFO) << "[" << _downloadUUID << "] - Not a continuation...";

		// Sessions are not thread safe in resources
		Wt::WApplication::UpdateLock lock(wApp);

		Wt::Dbo::Transaction transaction(DboSession());

		auto share = Database::Share::getByDownloadUUID(DboSession(), _downloadUUID);

		FS_LOG(UI, INFO) << "Hits = " << share->getHits() << " / " << share->getMaxHits();

		if (share->hasExpired())
		{
			FS_LOG(UI, WARNING) << "Share expired!";
			response.setStatus(404);
			return;
		}

		suggestFileName(share->getFileName());
		_path = share->getPath();
		share.modify()->incHits();

		FS_LOG(UI, INFO) << "Hits now set to " << share->getHits() << " / " << share->getMaxHits();
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
		FS_LOG(UI, ERROR) << "Cannot open " << _path;
		return;
	}
	else
		handleRequestPiecewise(request, response, is);
}

}
