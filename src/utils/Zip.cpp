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

#include <stdexcept>
#include <zip.h>

#include "Zip.hpp"

ZipFileWriter::ZipFileWriter(std::filesystem::path file)
{
	int err_code;

	_zip = zip_open(file.string().c_str(), ZIP_CREATE | ZIP_EXCL, &err_code);
	if (_zip == NULL)
		throw std::runtime_error("Cannot create archive file '" + file.string() + "'");
}

ZipFileWriter::~ZipFileWriter()
{
	zip_close(_zip);
}

void
ZipFileWriter::add(const std::string& fileName, const std::filesystem::path& file)
{

	struct zip_source *source = zip_source_file(_zip,
					file.string().c_str(),
					0, 0);
	if (source == NULL)
		throw std::runtime_error("Cannot read file '" + file.string() + "' to be put into archive!");

	zip_int64_t res = zip_file_add(_zip, fileName.c_str(), source, ZIP_FL_ENC_UTF_8);
	if (res == -1)
		throw std::runtime_error("Cannot add file '" + file.string() + "' into archive!");
}
