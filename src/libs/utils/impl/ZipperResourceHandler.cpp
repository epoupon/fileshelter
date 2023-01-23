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

#include <array>

std::unique_ptr<IResourceHandler>
createZipperResourceHandler(std::unique_ptr<Zip::Zipper> zipper)
{
	return std::make_unique<ZipperResourceHandler>(std::move(zipper));
}

ZipperResourceHandler::ZipperResourceHandler(std::unique_ptr<Zip::Zipper> zipper)
: _zipper {std::move(zipper)}
{
}

Wt::Http::ResponseContinuation*
ZipperResourceHandler::processRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
	std::array<std::byte, _bufferSize> buffer;
	const Zip::SizeType nbWrittenBytes {_zipper->writeSome(buffer.data(), buffer.size())};

	response.out().write(reinterpret_cast<const char *>(buffer.data()), nbWrittenBytes);

	if (!_zipper->isComplete())
	{
		Wt::Http::ResponseContinuation* continuation {response.createContinuation()};
		continuation->setData(_zipper);
		return continuation;
	}

	return nullptr;
}

