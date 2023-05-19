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

#include "ZipperResourceHandler.hpp"
#include "utils/Logger.hpp"

#include <array>

std::unique_ptr<IResourceHandler>
createZipperResourceHandler(std::unique_ptr<Zip::IZipper> zipper)
{
	return std::make_unique<ZipperResourceHandler>(std::move(zipper));
}

ZipperResourceHandler::ZipperResourceHandler(std::unique_ptr<Zip::IZipper> zipper)
: _zipper {std::move(zipper)}
{
}

void
ZipperResourceHandler::processRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
	try
	{
		_zipper->writeSome(response.out());
	}
	catch (const Zip::Exception& e)
	{
		FS_LOG(UTILS, ERROR) << "Caught exception while writing zip: " << e.what();
		_zipper.reset();
	}
}

bool
ZipperResourceHandler::isComplete() const
{
	return !_zipper || _zipper->isComplete();
}

void
ZipperResourceHandler::abort()
{
	if (!_zipper)
		return;

	_zipper->abort();
	_zipper.reset();
}
