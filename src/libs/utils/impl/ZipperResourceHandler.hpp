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

#include <memory>
#include "utils/IZipper.hpp"
#include "utils/IResourceHandler.hpp"

class ZipperResourceHandler final : public IResourceHandler
{
	public:
		ZipperResourceHandler(std::unique_ptr<Zip::IZipper> zipper);

	private:
		void processRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;
		bool isComplete() const override;
		void abort() override;

		std::unique_ptr<Zip::IZipper> _zipper;
};
