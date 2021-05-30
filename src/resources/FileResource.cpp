/*
 * Copyright (C) 2021 Emeric Poupon
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

#include "FileResource.hpp"

#include <fstream>
#include <sstream>

#include <Wt/Http/Response.h>
#include <Wt/Utils.h>
#include <Wt/WLocalDateTime.h>

#include "database/Db.hpp"
#include "database/File.hpp"
#include "database/Share.hpp"
#include "share/ShareUtils.hpp"
#include "utils/Logger.hpp"

FileResource::FileResource(Database::Db& db)
: _db {db}
{}

FileResource::~FileResource()
{
	beingDeleted();
}

std::string_view
FileResource::getDeployPath()
{
	return "/file";
}

Wt::WLink
FileResource::createLink(const UUID& uuid, std::optional<std::string_view> password)
{
	return {Wt::LinkType::Url, std::string {getDeployPath()} + "?id=" + uuid.getAsString() + (password ? ("&p=" + Wt::Utils::hexEncode(std::string {*password})) : "")};
}

std::shared_ptr<FileResource::FileContext>
FileResource::parseFileRequest(Wt::Dbo::Session& session, const Wt::Http::Request& request)
{
	const std::string* strUuid {request.getParameter("id")};
	if (!strUuid)
	{
		FS_LOG(FILE_RESOURCE, DEBUG) << "Missing parameter 'id'!";
		return {};
	}
	std::optional<UUID> uuid {UUID::fromString(*strUuid)};
	if (!uuid)
	{
		FS_LOG(FILE_RESOURCE, DEBUG) << "Cannot parse UUID '" << *strUuid << "'";
		return {};
	}

	const std::string* password {request.getParameter("p")};
	// TODO password

	Wt::Dbo::Transaction transaction {session};

	if (Database::File::pointer file {Database::File::getByDownloadUUID(session, *uuid)})
	{
		file.modify()->incHits();

		return std::make_shared<FileContext>(FileContext {*uuid, file->getPath(), file->getName()});
	}

	FS_LOG(FILE_RESOURCE, DEBUG) << "Cannot parse file request";
	return {};
}

void
FileResource::handleFileRequest(std::shared_ptr<FileContext> fileContext, const Wt::Http::Request& request, Wt::Http::Response& response)
{
	std::ifstream ifs {fileContext->filePath.string().c_str(), std::ios::in | std::ios::binary};
	if (!ifs)
	{
		FS_LOG(FILE_RESOURCE, ERROR) << "File '" << fileContext->filePath.string() << "' does no longer exist!";
		response.setStatus(404);
		return;
	}

	std::uint64_t startByte {fileContext->currentOffset};

	if (startByte == 0)
	{
		response.setStatus(200);

		ifs.seekg(0, std::ios::end);
		std::istream::pos_type isize {ifs.tellg()};
		ifs.seekg(0, std::ios::beg);

		const Wt::Http::Request::ByteRangeSpecifier ranges {request.getRanges(isize)};

		if (!ranges.isSatisfiable())
		{
			std::ostringstream contentRange;
			contentRange << "bytes */" << isize;
			response.setStatus(416); // Requested range not satisfiable
			response.addHeader("Content-Range", contentRange.str());
			return;
		}

		if (ranges.size() == 1)
		{
			response.setStatus(206);
			startByte = ranges[0].firstByte();
			fileContext->beyondLastByte = ranges[0].lastByte() + 1;

			std::ostringstream contentRange;
			contentRange << "bytes " << startByte << "-"
				<< fileContext->beyondLastByte - 1 << "/" << isize;
			response.addHeader("Content-Range", contentRange.str());
			response.setContentLength(fileContext->beyondLastByte - startByte);
		}
		else
		{
			fileContext->beyondLastByte = isize;
			response.setContentLength(fileContext->beyondLastByte);
		}
	}

	ifs.seekg(static_cast<std::istream::pos_type>(startByte));
	if (ifs.fail())
	{
		FS_LOG(FILE_RESOURCE, ERROR) << "Cannot seek in file '" << fileContext->filePath.string() << "' at pos " << startByte;
		return;
	}

	// According to 27.6.1.3, paragraph 1 of ISO/IEC 14882:2003(E),
	// each unformatted input function may throw an exception.
	const std::uint64_t restSize {fileContext->beyondLastByte - startByte};
	const std::uint64_t pieceSize {_bufferSize > restSize ? restSize : _bufferSize};

	std::array<std::byte, _bufferSize> buffer;
	ifs.read(reinterpret_cast<char*>(buffer.data()), pieceSize);
	if (ifs.fail())
	{
		FS_LOG(FILE_RESOURCE, ERROR) << "Cannot read in file '" << fileContext->filePath.string() << "' at pos " << startByte;
		return;
	}

	const std::streamsize actualPieceSize {ifs.gcount()};
	response.out().write(reinterpret_cast<const char*>(buffer.data()), actualPieceSize);

	if (ifs.good() && actualPieceSize < restSize)
	{
		auto continuation = response.createContinuation();
		fileContext->currentOffset = startByte + std::uint64_t {actualPieceSize};
		continuation->setData(fileContext);
	}
}

void
FileResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
	std::shared_ptr<FileContext> fileContext;

	// First, see if this request is for a continuation
	Wt::Http::ResponseContinuation *continuation {request.continuation()};
	if (continuation)
	{
		fileContext = Wt::cpp17::any_cast<std::shared_ptr<FileContext>>(continuation->data());
		if (!fileContext)
			return;
	}
	else
	{
		fileContext = parseFileRequest(_db.getTLSSession(), request);
		if (!fileContext)
			return;

		suggestFileName(Wt::WString::fromUTF8(fileContext->fileName));
		response.setMimeType("application/octet-stream");
	}

	handleFileRequest(fileContext, request, response);
}

