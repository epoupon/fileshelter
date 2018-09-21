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

#include "ShareEdit.hpp"

#include <Wt/WEnvironment.h>
#include <Wt/WTemplate.h>
#include <Wt/WPushButton.h>
#include <Wt/WMessageBox.h>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCommon.hpp"

namespace UserInterface {

ShareEdit::ShareEdit(Wt::WContainerWidget* parent)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
}

void
ShareEdit::refresh(void)
{
	if (!wApp->internalPathMatches("/share-edit"))
		return;

	clear();

	std::string editUUID = wApp->internalPathNextPart("/share-edit/");

	Wt::Dbo::Transaction transaction(FsApp->getDboSession());

	Database::Share::pointer share = Database::Share::getByEditUUID(FsApp->getDboSession(), editUUID);
	if (!share || !boost::filesystem::exists(share->getPath()))
	{
		FS_LOG(UI, ERROR) << "Edit UUID '" << editUUID << "' not found";
		displayNotFound();
		return;
	}

	FS_LOG(UI, INFO) << "[" << share->getDownloadUUID() << "] Editing share from " << wApp->environment().clientAddress();

	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-edit"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	if (!share->getDesc().empty())
	{
		t->setCondition("if-desc", true);
		t->bindString("file-desc", Wt::WString::fromUTF8(share->getDesc()));
	}
	t->bindString("file-name", Wt::WString::fromUTF8(share->getFileName()));
	t->bindString("file-size", sizeToString(share->getFileSize()));
	t->bindString("expiry-date-time", share->getExpiryTime().toString() + " UTC");

	auto hits = std::to_string(share->getHits());
	if (share->getMaxHits() > 0)
		hits += " / " + std::to_string(share->getMaxHits());
	t->bindString("hits", hits);

	t->bindWidget("download-link", createShareDownloadAnchor(share));

	Wt::WPushButton* deleteBtn = t->bindNew<Wt::WPushButton>("delete-btn", tr("msg-delete"));

	deleteBtn->clicked().connect([=] ()
	{
		auto messageBox = deleteBtn->addChild(std::make_unique<Wt::WMessageBox>(tr("msg-share-delete"),
			tr("msg-confirm-action"),
			 Wt::Icon::Question,
			 Wt::StandardButton::Yes | Wt::StandardButton::No));

		messageBox->setModal(true);

		messageBox->buttonClicked().connect([=] (Wt::StandardButton btn)
		{
			if (btn == Wt::StandardButton::Yes)
			{
				Wt::Dbo::Transaction transaction(FsApp->getDboSession());

				Database::Share::pointer share = Database::Share::getByEditUUID(FsApp->getDboSession(), editUUID);

				if (share)
				{
					FS_LOG(UI, INFO) << "[" << share->getDownloadUUID() << "] Deleting share from " << wApp->environment().clientAddress();
					share.modify()->destroy();
					share.remove();
				}

				displayRemoved();
			}
			deleteBtn->removeChild(messageBox);
		});

		messageBox->show();
	});

}

void
ShareEdit::displayRemoved(void)
{
	clear();

	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-removed"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}

void
ShareEdit::displayNotFound(void)
{
	clear();

	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-not-found"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}


} // namespace UserInterface

