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

#include "ShareCreated.hpp"

#include <Wt/WTemplate.h>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCommon.hpp"


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

	Wt::Dbo::Transaction transaction(FsApp->getDboSession());

	Database::Share::pointer share = Database::Share::getByEditUUID(FsApp->getDboSession(), editUUID);
	if (!share)
	{
		FS_LOG(UI, ERROR) << "Edit UUID '" << editUUID << "' not found";
		Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-not-found"));
		t->addFunction("tr", &Wt::WTemplate::Functions::tr);
		return;
	}

	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-created"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	t->bindWidget("download-link", createShareDownloadAnchor(share));
	t->bindWidget("edit-link", createShareEditAnchor(share));
}

} // namespace UserInterface

