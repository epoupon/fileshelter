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
#include <Wt/WPushButton>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareResource.hpp"
#include "ShareDownloadPassword.hpp"
#include "ShareCommon.hpp"

#include "ShareDownload.hpp"


namespace UserInterface {

ShareDownload::ShareDownload(Wt::WContainerWidget* parent)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
}

void
ShareDownload::refresh(void)
{
	if (!wApp->internalPathMatches("/share-download"))
		return;

	clear();

	std::string downloadUUID = wApp->internalPathNextPart("/share-download/");

	Wt::Dbo::Transaction transaction(DboSession());

	Database::Share::pointer share = Database::Share::getByDownloadUUID(DboSession(), downloadUUID);
	if (!share)
	{
		displayNotFound();
		return;
	}

	if (share->hasPassword())
		displayPassword();
	else
		displayDownload();
}

void
ShareDownload::displayDownload(void)
{
	clear();

	std::string downloadUUID = wApp->internalPathNextPart("/share-download/");

	Wt::Dbo::Transaction transaction(DboSession());
	Database::Share::pointer share = Database::Share::getByDownloadUUID(DboSession(), downloadUUID);
	if (!share)
	{
		displayNotFound();
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-download"), this);

	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	if (!share->getDesc().empty())
	{
		t->setCondition("if-desc", true);
		t->bindString("file-desc", Wt::WString::fromUTF8(share->getDesc()));
	}
	t->bindString("file-name", Wt::WString::fromUTF8(share->getFileName()));
	t->bindString("file-size", sizeToString(share->getFileSize()));

	auto *downloadBtn = new Wt::WPushButton(tr("msg-download"));

	if (share->hasExpired())
	{
		t->setCondition("if-error", true);
		t->bindString("error", tr("msg-share-no-longer-available"));
		downloadBtn->setEnabled(false);
	}
	else
	{
		downloadBtn->setResource(new ShareResource(downloadUUID));
	}

	downloadBtn->addStyleClass("btn btn-primary");
	t->bindWidget("download-btn", downloadBtn);
}

void
ShareDownload::displayNotFound()
{
	clear();
	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-found"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}


void
ShareDownload::displayPassword()
{
	clear();

	auto view = new ShareDownloadPassword(this);
	view->success().connect(std::bind([=]
	{
		displayDownload();
	}));
}


} // namespace UserInterface

