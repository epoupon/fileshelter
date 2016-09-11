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

#include <Wt/WTemplate>
#include <Wt/WAnchor>
#include <Wt/WEnvironment>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"

#include "ShareCreated.hpp"


namespace UserInterface {

ShareCreated::ShareCreated(Wt::WContainerWidget* parent)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
}

void
ShareCreated::refresh(void)
{
	if (!wApp->internalPathMatches("/share-created"))
		return;

	clear();

	std::string editUUID = wApp->internalPathNextPart("/share-created/");

	Wt::Dbo::Transaction transaction(DboSession());

	Database::Share::pointer share = Database::Share::getByEditUUID(DboSession(), editUUID);
	if (!share)
	{
		FS_LOG(UI, ERROR) << "Edit UUID '" << editUUID << "' not found";
		Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-found"), this);
		t->addFunction("tr", &Wt::WTemplate::Functions::tr);
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-created"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	std::string downloadPath = "/share-download/" + share->getDownloadUUID();
	std::string editPath = "/share-edit/" + share->getEditUUID();

	t->bindWidget("download-link", new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, downloadPath), wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + downloadPath));
	t->bindWidget("edit-link", new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, editPath), wApp->environment().urlScheme() + "://" + wApp->environment().hostName() + editPath));
}

} // namespace UserInterface

