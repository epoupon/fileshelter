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

#include "FileResourceHandler.hpp"

#include <cstring>
#include <fstream>

#include "utils/Logger.hpp"

std::unique_ptr<IResourceHandler>
createFileResourceHandler(const std::filesystem::path& path)
{
	return std::make_unique<FileResourceHandler>(path);
}


FileResourceHandler::FileResourceHandler(const std::filesystem::path& path)
: _path {path}
{
}


Wt::Http::ResponseContinuation*
FileResourceHandler::processRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
	::uint64_t startByte {_offset};
	std::ifstream ifs {_path.string().c_str(), std::ios::in | std::ios::binary};
	if (!ifs)
	{
		const int err {errno};
		FS_LOG(UTILS, ERROR) << "Cannot open input file '" << _path.string() << "': " << std::string {::strerror(err)};
		_isFinished = true;

		if (startByte == 0)
			response.setStatus(404);

		return {};
	}

	if (startByte == 0)
	{
		response.setStatus(200);

		ifs.seekg(0, std::ios::end);
		const ::uint64_t fileSize {static_cast<::uint64_t>(ifs.tellg())};
		ifs.seekg(0, std::ios::beg);

		FS_LOG(UTILS, DEBUG) << "File '" << _path.string() << "', fileSize = " << fileSize;

		const Wt::Http::Request::ByteRangeSpecifier ranges {request.getRanges(fileSize)};
		if (!ranges.isSatisfiable())
		{
			std::ostringstream contentRange;
			contentRange << "bytes */" << fileSize;
			response.setStatus(416); // Requested range not satisfiable
			response.addHeader("Content-Range", contentRange.str());

			FS_LOG(UTILS, DEBUG) << "Range not satisfiable";
			_isFinished = true;
			return {};
		}

		if (ranges.size() == 1)
		{
			FS_LOG(UTILS, DEBUG) << "Range requested = " << ranges[0].firstByte() << "/" << ranges[0].lastByte();

			response.setStatus(206);
			startByte = ranges[0].firstByte();
			_beyondLastByte = ranges[0].lastByte() + 1;

			std::ostringstream contentRange;
			contentRange << "bytes " << startByte << "-"
				<< _beyondLastByte - 1 << "/" << fileSize;

			response.addHeader("Content-Range", contentRange.str());
			response.setContentLength(_beyondLastByte - startByte);
		}
		else
		{
			FS_LOG(UTILS, DEBUG) << "No range requested";

			_beyondLastByte = fileSize;
			response.setContentLength(_beyondLastByte);
		}
	}

	if (!ifs.seekg(static_cast<std::istream::pos_type>(startByte)))
	{
		const int err {errno};
		FS_LOG(UTILS, ERROR) << "Failed to seek in file '" << _path.string() << "' at " << startByte << ": " << std::string {::strerror(err)};
		return {};
	}

	std::vector<char> buf;
	buf.resize(_chunkSize);

	::uint64_t restSize = _beyondLastByte - startByte;
	::uint64_t pieceSize = buf.size() > restSize ? restSize : buf.size();

	if (!ifs.read(&buf[0], pieceSize))
	{
		const int err {errno};
		FS_LOG(UTILS, ERROR) << "Read failed in file '" << _path.string() << "': " << std::string {::strerror(err)};
		return {};
	}
	const ::uint64_t actualPieceSize {static_cast<::uint64_t>(ifs.gcount())};
	response.out().write(&buf[0], actualPieceSize);

	FS_LOG(UTILS, DEBUG) << "Written " << actualPieceSize << " bytes";

	FS_LOG(UTILS, DEBUG) << "Progress: " << actualPieceSize << "/" << restSize;
	if (ifs.good() && actualPieceSize < restSize)
	{
		_offset = startByte + actualPieceSize;
		FS_LOG(UTILS, DEBUG) << "Job not complete! Next chunk offset = " << _offset;

		return response.createContinuation();
	}

	_isFinished = true;
	FS_LOG(UTILS, DEBUG) << "Job complete!";

	return {};
}



