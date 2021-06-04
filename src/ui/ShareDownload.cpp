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

#include "ShareDownload.hpp"

#include <Wt/WAnchor.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

#include "resources/FileResource.hpp"
#include "resources/ShareResource.hpp"
#include "database/File.hpp"
#include "database/Share.hpp"
#include "share/ShareUtils.hpp"
#include "utils/Logger.hpp"

#include "FileShelterApplication.hpp"
#include "ShareDownloadPassword.hpp"
#include "ShareCommon.hpp"

namespace UserInterface {

ShareDownload::ShareDownload()
{
	wApp->internalPathChanged().connect([this]
	{
		refresh();
	});

	refresh();
}

void
ShareDownload::refresh()
{
	clear();

	if (!wApp->internalPathMatches("/share-download"))
		return;

	const std::optional<UUID> downloadUUID {UUID::fromString(wApp->internalPathNextPart("/share-download/"))};
	if (!downloadUUID)
		return;

	Wt::Dbo::Transaction transaction {FsApp->getDboSession()};
	const Database::Share::pointer share {Database::Share::getByDownloadUUID(FsApp->getDboSession(), *downloadUUID)};
	if (!share)
	{
		displayNotFound();
		return;
	}

	if (share->hasPassword())
		displayPassword(*downloadUUID);
	else
		displayDownload(*downloadUUID);
}

void
ShareDownload::displayDownload(const UUID& downloadUUID, std::optional<std::string_view> password)
{
	clear();

	Wt::Dbo::Transaction transaction {FsApp->getDboSession()};
	const Database::Share::pointer share {Database::Share::getByDownloadUUID(FsApp->getDboSession(), downloadUUID)};
	if (!share || !ShareUtils::isShareAvailableForDownload(share))
	{
		displayNotFound();
		return;
	}

	Wt::WTemplate* t {addNew<Wt::WTemplate>(tr("template-share-download"))};

	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	if (!share->getDesc().empty())
	{
		t->setCondition("if-desc", true);
		t->bindString("file-desc", Wt::WString::fromUTF8(std::string {share->getDesc()}));
	}

	t->bindString("share-size", sizeToString(share->getShareSize()));
	t->bindString("expiry-date-time", share->getExpiryTime().toString() + " UTC");

	{
		Wt::WPushButton* downloadBtn {t->bindNew<Wt::WPushButton>("download-as-zip-btn", tr("msg-download-as-zip"))};
		downloadBtn->setLink(ShareResource::createLink(downloadUUID, password));
	}

	{
		auto* filesContainer {t->bindNew<Wt::WContainerWidget>("files")};
		for (const Database::File::pointer& file : share->getFiles())
		{
			Wt::WTemplate* fileTemplate {filesContainer->addNew<Wt::WTemplate>(tr("template-share-download-file"))};

			fileTemplate->bindString("name", file->getName(), Wt::TextFormat::Plain);
			fileTemplate->bindString("size", sizeToString(file->getSize()), Wt::TextFormat::Plain);
			{
				Wt::WPushButton* downloadBtn {fileTemplate->bindNew<Wt::WPushButton>("download-btn", tr("msg-download"))};
				downloadBtn->setLink(FileResource::createLink(file->getDownloadUUID(), password));
			}
		}
	}
}

void
ShareDownload::displayNotFound()
{
	clear();
	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-not-found"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}


void
ShareDownload::displayPassword(const UUID& downloadUUID)
{
	clear();

	auto view = addNew<ShareDownloadPassword>(downloadUUID);
	view->success().connect([=] (std::string_view password)
	{
		displayDownload(downloadUUID, password);
	});
}


} // namespace UserInterface

