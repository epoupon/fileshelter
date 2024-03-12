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

#include "ShareList.hpp"

#include <Wt/WAnchor.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

#include "resources/ShareResource.hpp"
#include "share/IShareManager.hpp"
#include "share/Exception.hpp"
#include "share/Types.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

#include "PasswordUtils.hpp"
#include "Exception.hpp"
#include "ShareListPassword.hpp"
#include "ShareUtils.hpp"

namespace UserInterface
{

	ShareList::ShareList()
	{
		wApp->internalPathChanged().connect(this, [this]
		{
			handlePathChanged();
		});

		handlePathChanged();
	}

	void
	ShareList::handlePathChanged()
	{
		clear();

		if (!wApp->internalPathMatches("/share-list"))
			return;

		if (!PasswordUtils::isListPasswordDefined())
			return;
		else
			displayPassword();
	}

	void
	ShareList::displayPassword()
	{
		auto view = addNew<ShareListPassword>();
		view->success().connect([=]
		{
			clear();
			displayList();
		});
	}

	void
	ShareList::displayList()
	{
		Wt::WTemplate* t {addNew<Wt::WTemplate>(tr("template-share-list"))};

		t->addFunction("tr", &Wt::WTemplate::Functions::tr);

		auto* sharesContainer {t->bindNew<Wt::WContainerWidget>("shares")};

		std::size_t nbShares{0};
		Service<Share::IShareManager>::get()->visitShares([&](const Share::ShareDesc& share)
		{
			nbShares++;

			Wt::WTemplate* shareTemplate {sharesContainer->addNew<Wt::WTemplate>(tr("template-share-list-file"))};

			shareTemplate->bindString("share-uuid", share.uuid.toString());
			shareTemplate->bindString("expiry-date-time", share.expiryTime.toString() + " UTC", Wt::TextFormat::Plain);
			shareTemplate->bindString("share-size", ShareUtils::fileSizeToString(share.size), Wt::TextFormat::Plain);
			shareTemplate->bindInt("download-count", share.readCount);
			shareTemplate->bindString("created-by", share.creatorAddress);
		});
		t->bindInt("total-shares", nbShares);
	}

} // namespace UserInterface
