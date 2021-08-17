/*
 * Copyright (C) 2020 Emeric Poupon
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

#include <memory>
#include <optional>
#include <Wt/Auth/PasswordHash.h>
#include <Wt/Http/Response.h>
#include <Wt/Utils.h>
#include <Wt/WLocalDateTime.h>

#include "share/IShareManager.hpp"
#include "share/Exception.hpp"
#include "utils/FileResourceHandlerCreator.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "utils/Zipper.hpp"
#include "utils/ZipperResourceHandlerCreator.hpp"

using namespace Share;

namespace
{
	std::filesystem::path
	getClientFileName(const ShareDesc& share)
	{
		if (share.files.size() == 1)
			return share.files.front().clientPath;

		return share.uuid.toString() + ".zip";
	}

	std::unique_ptr<Zip::Zipper>
	createZipper(const ShareDesc& share)
	{

		std::map<std::string, std::filesystem::path> zipFiles;
		for (const FileDesc& file : share.files)
			zipFiles.emplace(file.clientPath, file.path);

		// mask creation time
		return std::make_unique<Zip::Zipper>(zipFiles, Wt::WLocalDateTime::currentDateTime().toUTC());
	}
}

ShareResource::~ShareResource()
{
	beingDeleted();
}

Wt::WLink
ShareResource::createLink(const ShareUUID& uuid, std::optional<std::string_view> password)
{
	return {Wt::LinkType::Url, std::string {getDeployPath()} + "?id=" + uuid.toString() + (password ? ("&p=" + Wt::Utils::hexEncode(std::string {*password})) : "")};
}


void
ShareResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
	try
	{
		std::shared_ptr<IResourceHandler> resourceHandler;
		Wt::Http::ResponseContinuation* continuation {request.continuation()};
		if (!continuation)
		{
			// parse parameters
			const std::string* uuid {request.getParameter("id")};
			if (!uuid)
			{
				FS_LOG(UI, DEBUG) << "Missing parameter 'id'!";
				return;
			}
			const ShareUUID& shareUUID {*uuid};

			std::optional<std::string> password;
			if (const std::string *p {request.getParameter("p")})
				password = Wt::Utils::hexDecode(*password);

			const ShareDesc share {Service<IShareManager>::get()->getShareDesc(shareUUID, password)};
			if (share.files.size() > 1)
			{
				std::unique_ptr<Zip::Zipper> zipper {createZipper(share)};
				response.setContentLength(zipper->getTotalZipFile());
				response.setMimeType("application/zip");
				resourceHandler = createZipperResourceHandler(std::move(zipper));
			}
			else
			{
				response.setMimeType("application/octet-stream");
				resourceHandler = createFileResourceHandler(share.files.front().path);
			}

			suggestFileName(getClientFileName(share).string());
		}
		else
		{
			resourceHandler = Wt::cpp17::any_cast<std::shared_ptr<IResourceHandler>>(continuation->data());
		}

		continuation = resourceHandler->processRequest(request, response);
		if (continuation)
			continuation->setData(resourceHandler);
	}
	catch (const UUIDException& e)
	{
		FS_LOG(UI, DEBUG) << "Bad parameter 'id'!";
	}
	catch (const Share::Exception& e)
	{
		FS_LOG(UI, ERROR) << "Caught Share::Exception: " << e.what();
	}
	catch (Zip::ZipperException& exception)
	{
		FS_LOG(UI, ERROR) << "Zipper exception: " << exception.what();
	}
}

