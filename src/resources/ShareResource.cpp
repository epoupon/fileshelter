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

#include <iostream>
#include <iomanip>

#include <Wt/Http/Response.h>
#include <Wt/Utils.h>
#include <Wt/WLocalDateTime.h>

#include "database/Db.hpp"
#include "database/File.hpp"
#include "database/Share.hpp"
#include "share/ShareUtils.hpp"
#include "utils/Logger.hpp"
#include "utils/Zipper.hpp"

static
std::unique_ptr<Zip::Zipper>
createZipper(const std::vector<Database::File::pointer>& files)
{
	std::map<std::string, std::filesystem::path> zipFiles;

	for (const Database::File::pointer& file : files)
		zipFiles.emplace(file->getName(), file->getPath());

	// mask creation time
	return std::make_unique<Zip::Zipper>(zipFiles, Wt::WLocalDateTime::currentDateTime().toUTC());
}

static
std::unique_ptr<Zip::Zipper>
createZipper(const std::string& uuid, const std::string* password, Wt::Dbo::Session& session)
{
	std::optional<Wt::Auth::PasswordHash> passwordHash;

	const std::optional<UUID> downloadUUID {UUID::fromString(uuid)};
	if (!downloadUUID)
	{
		FS_LOG(UI, DEBUG) << "Bad parameter 'id'!";
		return {};
	}

	{
		Wt::Dbo::Transaction transaction {session};

		const Database::Share::pointer share {Database::Share::getByDownloadUUID(session, *downloadUUID)};
		if (!share)
		{
			FS_LOG(UI, DEBUG) << "Share '" << uuid << "' not found!";
			return {};
		}

		if (share->hasPassword())
			passwordHash = share->getPasswordHash();
	}

	if (passwordHash)
	{
		if (!password)
		{
			FS_LOG(UI, DEBUG) << "Missing parameter 'p' for share '" << uuid << "'!";
			return {};
		}

		if (!ShareUtils::checkPassword(*passwordHash, Wt::Utils::hexDecode(*password)))
		{
			FS_LOG(UI, DEBUG) << "Bad password for share '" << uuid << "'!";
			return {};
		}
	}

	{
		Wt::Dbo::Transaction transaction {session};

		const Database::Share::pointer share {Database::Share::getByDownloadUUID(session, *downloadUUID)};
		if (!share)
		{
			FS_LOG(UI, DEBUG) << "Share '" << uuid << "' not found!";
			return {};
		}

		return createZipper(share->getFiles());
	}
}

ShareResource::ShareResource(Database::Db& db)
: _db {db}
{}

ShareResource::~ShareResource()
{
	beingDeleted();
}

std::string_view
ShareResource::getDeployPath()
{
	return "/share";
}

Wt::WLink
ShareResource::createLink(const UUID& uuid, std::optional<std::string_view> password)
{
	return {Wt::LinkType::Url, std::string {getDeployPath()} + "?id=" + uuid.getAsString() + (password ? ("&p=" + Wt::Utils::hexEncode(std::string {*password})) : "")};
}

void
ShareResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
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

			zipper = createZipper(*uuid, request.getParameter("p"), _db.getTLSSession());
			if (!zipper)
				return;

			response.setContentLength(zipper->getTotalZipFile());
			response.setMimeType("application/zip");
			suggestFileName(*uuid + ".zip");
		}

		std::array<std::byte, _bufferSize> buffer;
		const std::size_t nbWrittenBytes {zipper->writeSome(buffer.data(), buffer.size())};

		response.out().write(reinterpret_cast<const char *>(buffer.data()), nbWrittenBytes);

		if (!zipper->isComplete())
		{
			auto* continuation {response.createContinuation()};
			continuation->setData(zipper);
		}
		else
		{
			const std::optional<UUID> uuid {UUID::fromString(*request.getParameter("id"))};
			assert(uuid);

			Wt::Dbo::Transaction transaction {_db.getTLSSession()};

			if (Database::Share::pointer share {Database::Share::getByDownloadUUID(_db.getTLSSession(), *uuid)})
				share.modify()->incHits();
		}
	}
	catch (Zip::ZipperException& exception)
	{
		FS_LOG(UI, ERROR) << "Zipper exception: " << exception.what();
	}
}

