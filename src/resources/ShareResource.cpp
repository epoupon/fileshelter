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

#include <optional>
#include <Wt/Auth/PasswordHash.h>
#include <Wt/Http/Response.h>
#include <Wt/Utils.h>
#include <Wt/WLocalDateTime.h>

#include "share/IShareManager.hpp"
#include "share/Exception.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "utils/Zipper.hpp"

using namespace Share;

ShareResource::~ShareResource()
{
	beingDeleted();
}

Wt::WLink
ShareResource::createLink(const ShareUUID& uuid, std::optional<std::string_view> password)
{
	return {Wt::LinkType::Url, std::string {getDeployPath()} + "?id=" + uuid.toString() + (password ? ("&p=" + Wt::Utils::hexEncode(std::string {*password})) : "")};
}

std::filesystem::path
ShareResource::getClientFileName(const Share::ShareDesc& share)
{
	std::filesystem::path fileName;

	for (const Share::FileDesc& file : share.files)
	{
		if (file.clientPath.empty())
			fileName = file.clientPath.string() + ".zip";
		else
			fileName = share.uuid.toString() + ".zip";
	}

	return fileName;
}

void
ShareResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{

	for (const auto& header : request.headers())
	{
		FS_LOG(UI, DEBUG) << "Header = '" << header.name() << "', value = '" << header.value() << "'";
	}
	FS_LOG(UI, DEBUG) << "request.clientAddr = " << request.clientAddress();

	try
	{
		std::shared_ptr<Zip::Zipper> zipper;

		// First, see if this request is for a continuation
		Wt::Http::ResponseContinuation *continuation {request.continuation()};
		if (continuation)
		{
			zipper = Wt::cpp17::any_cast<std::shared_ptr<Zip::Zipper>>(continuation->data());
			if (!zipper)
				return;
		}
		else
		{
			const std::string* uuid {request.getParameter("id")};
			if (!uuid)
			{
				FS_LOG(UI, DEBUG) << "Missing parameter 'id'!";
				return;
			}

			std::optional<std::string> password;
			if (const std::string *p {request.getParameter("p")})
				password = Wt::Utils::hexDecode(*password);

			const ShareUUID& shareUUID {*uuid};

			try
			{
				zipper = Service<IShareManager>::get()->getShareZipper(shareUUID, password);
				if (!zipper)
					return;

				// TODO avoid double password check
				suggestFileName(getClientFileName(Service<Share::IShareManager>::get()->getShareDesc(shareUUID, password)).string());
			}
			catch (const Share::Exception& e)
			{
				FS_LOG(UI, ERROR) << "Caught Share::Exception while creating zipper: " << e.what();
				return;
			}

			response.setContentLength(zipper->getTotalZipFile());
			response.setMimeType("application/zip");
		}

		std::array<std::byte, _bufferSize> buffer;
		const std::size_t nbWrittenBytes {zipper->writeSome(buffer.data(), buffer.size())};

		response.out().write(reinterpret_cast<const char *>(buffer.data()), nbWrittenBytes);

		if (!zipper->isComplete())
		{
			Wt::Http::ResponseContinuation* continuation {response.createContinuation()};
			continuation->setData(zipper);
		}
	}
	catch (const UUIDException& e)
	{
		FS_LOG(UI, DEBUG) << "Bad parameter 'id'!";
	}
	catch (Zip::ZipperException& exception)
	{
		FS_LOG(UI, ERROR) << "Zipper exception: " << exception.what();
	}
}

