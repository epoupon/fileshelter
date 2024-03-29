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
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

#include "share/IShareManager.hpp"
#include "share/Exception.hpp"
#include "utils/FileResourceHandlerCreator.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "utils/IZipper.hpp"
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
}

void
ShareResource::setWorkingDirectory(std::filesystem::path workingDirectory)
{
	if (std::filesystem::is_directory(workingDirectory))
	{
		_workingDirectory = workingDirectory;
		FS_LOG(RESOURCE, INFO) << "Working directory set to '" << _workingDirectory.string() << "'";
	}
	else
	{
		FS_LOG(RESOURCE, ERROR) << "Cannot set working directory to '" << _workingDirectory.string() << "'";
	}
}

ShareResource::~ShareResource()
{
	beingDeleted();
}

Wt::WLink
ShareResource::createLink(const ShareUUID& uuid, std::optional<std::string_view> password)
{
    return {Wt::LinkType::Url, wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + (wApp->environment().deploymentPath() == "/" ? "" : wApp->environment().deploymentPath()) + "share?id=" + uuid.toString() + (password ? ("&p=" + Wt::Utils::hexEncode(std::string {*password})) : "")};
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
				FS_LOG(RESOURCE, DEBUG) << "Missing parameter 'id'!";
				return;
			}
			const ShareUUID& shareUUID {*uuid};

			std::optional<std::string> password;
			if (const std::string *p {request.getParameter("p")})
				password = Wt::Utils::hexDecode(*p);

			const ShareDesc share {Service<IShareManager>::get()->getShareDesc(shareUUID, password)};
			if (share.files.size() > 1)
			{
				std::unique_ptr<Zip::IZipper> zipper {createZipper(share)};
				response.setMimeType("application/zip");
				resourceHandler = createZipperResourceHandler(std::move(zipper));
			}
			else
			{
				response.setMimeType("application/octet-stream");
				resourceHandler = createFileResourceHandler(getAbsolutePath(share.files.front().path));
			}

			auto encodeHttpHeaderField = [](const std::string& fieldName, const std::string& fieldValue)
			{
				// This implements RFC 5987
				return fieldName + "*=UTF-8''" + Wt::Utils::urlEncode(fieldValue);
			};

			const std::string cdp {encodeHttpHeaderField("filename", getClientFileName(share).string())};
			response.addHeader("Content-Disposition", "attachment; " + cdp);

			Service<IShareManager>::get()->incrementReadCount(shareUUID);
		}
		else
		{
			resourceHandler = Wt::cpp17::any_cast<std::shared_ptr<IResourceHandler>>(continuation->data());
		}

		resourceHandler->processRequest(request, response);
		if (!resourceHandler->isComplete())
		{
			continuation = response.createContinuation();
			continuation->setData(resourceHandler);
		}

		return;
	}
	catch (const UUIDException& e)
	{
		FS_LOG(RESOURCE, DEBUG) << "Bad parameter 'id'!";
	}
	catch (const Share::Exception& e)
	{
		FS_LOG(RESOURCE, ERROR) << "Caught Share::Exception: " << e.what();
	}
	catch (Zip::Exception& exception)
	{
		FS_LOG(RESOURCE, ERROR) << "Zipper exception: " << exception.what();
	}

	response.setStatus(404);
}

void
ShareResource::handleAbort(const Wt::Http::Request& request)
{
	Wt::Http::ResponseContinuation* continuation {request.continuation()};
	if (!continuation)
		return;

	std::shared_ptr<IResourceHandler> resourceHandler {Wt::cpp17::any_cast<std::shared_ptr<IResourceHandler>>(continuation->data())};
	resourceHandler->abort();
}

std::filesystem::path
ShareResource::getAbsolutePath(const std::filesystem::path& p)
{
	return p.is_absolute() ? p : _workingDirectory / p;
}

std::unique_ptr<Zip::IZipper>
ShareResource::createZipper(const ShareDesc& share)
{
	Zip::EntryContainer zipEntries;
	for (const FileDesc& file : share.files)
		zipEntries.emplace_back(Zip::Entry {file.clientPath, getAbsolutePath(file.path)});

	// mask creation time
	return Zip::createArchiveZipper(zipEntries);
}

