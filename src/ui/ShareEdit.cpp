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
#include <Wt/WPushButton>
#include <Wt/WMessageBox>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"

#include "ShareEdit.hpp"


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

	FS_LOG(UI, DEBUG) << "editUUID = '" << editUUID << "'";

	Wt::Dbo::Transaction transaction(DboSession());

	Database::Share::pointer share = Database::Share::getByEditUUID(DboSession(), editUUID);
	if (!share || !boost::filesystem::exists(share->getPath()))
	{
		FS_LOG(UI, ERROR) << "Edit UUID '" << editUUID << "' not found";
		displayNotFound();
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-edit"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	if (!share->getDesc().empty())
	{
		t->setCondition("if-desc", true);
		t->bindString("file-desc", share->getDesc());
	}
	t->bindString("file-name", share->getFileName());
	t->bindString("file-size", std::to_string(share->getFileSize() / 1000));
	t->bindString("expiracy-date", boost::gregorian::to_simple_string(share->getExpiracyDate()));
	t->bindString("hits", std::to_string(share->getHits()));
	t->bindString("max-hits", std::to_string(share->getMaxHits()));

	auto *deleteBtn = new Wt::WPushButton(tr("msg-delete"));
	deleteBtn->addStyleClass("btn btn-danger");
	t->bindWidget("delete-btn", deleteBtn);

	deleteBtn->clicked().connect(std::bind([=] ()
	{
		Wt::WMessageBox *messageBox = new Wt::WMessageBox
			(tr("msg-share-delete"),
			tr("msg-confirm-action"),
			 Wt::Question, Wt::Yes | Wt::No);

		messageBox->setModal(true);

		messageBox->buttonClicked().connect(std::bind([=] ()
		{
			auto result = messageBox->buttonResult();
			delete messageBox;

			if (result == Wt::Yes)
			{
				Wt::Dbo::Transaction transaction(DboSession());

				Database::Share::pointer share = Database::Share::getByEditUUID(DboSession(), editUUID);

				if (share)
					share.remove();

				displayRemoved();
			}

		}));

		messageBox->show();
	}));

}

void
ShareEdit::displayRemoved(void)
{
	clear();

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-removed"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}

void
ShareEdit::displayNotFound(void)
{
	clear();

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-found"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}


} // namespace UserInterface

