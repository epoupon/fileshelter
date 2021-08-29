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

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WTemplate.h>
#include <Wt/WPushButton.h>
#include <Wt/WMessageBox.h>

#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "share/Exception.hpp"
#include "share/IShareManager.hpp"
#include "share/Types.hpp"

#include "ShareUtils.hpp"

namespace UserInterface
{

	ShareEdit::ShareEdit()
	{
		wApp->internalPathChanged().connect(this, [this]
		{
			handlePathChanged();
		});

		handlePathChanged();
	}

	void
	ShareEdit::handlePathChanged()
	{
		clear();

		if (!wApp->internalPathMatches("/share-edit"))
			return;

		try
		{
			const Share::ShareEditUUID editUUID {wApp->internalPathNextPart("/share-edit/")};
			displayEdit(editUUID);
		}
		catch (const Share::ShareNotFoundException& e)
		{
			displayShareNotFound();
		}
		catch (const UUIDException& e)
		{
			displayShareNotFound();
		}
	}

	void
	ShareEdit::displayEdit(const Share::ShareEditUUID& editUUID)
	{
		const Share::ShareDesc share {Service<Share::IShareManager>::get()->getShareDesc(editUUID)};

		FS_LOG(UI, INFO) << "[" << share.uuid.toString() << "] Editing share from " << wApp->environment().clientAddress();

		Wt::WTemplate *t {addNew<Wt::WTemplate>(tr("template-share-edit"))};
		t->addFunction("tr", &Wt::WTemplate::Functions::tr);

		t->bindInt("download-count", share.readCount);
		t->bindString("expiry-date-time", share.expiryTime.toString() + " UTC", Wt::TextFormat::Plain);

		t->bindWidget("download-link", ShareUtils::createShareDownloadAnchor(share.uuid));

		Wt::WPushButton* deleteBtn {t->bindNew<Wt::WPushButton>("delete-btn", tr("msg-delete"))};

		deleteBtn->clicked().connect([=]
		{
			auto messageBox = deleteBtn->addChild(std::make_unique<Wt::WMessageBox>(tr("msg-share-delete"),
						tr("msg-confirm-action"),
						Wt::Icon::Question,
						Wt::StandardButton::Yes | Wt::StandardButton::No));

			messageBox->setModal(true);

			messageBox->buttonClicked().connect([=] (Wt::StandardButton btn)
			{
				try
				{
					if (btn == Wt::StandardButton::Yes)
					{
						Service<Share::IShareManager>::get()->destroyShare(editUUID);
						displayRemoved();
					}
					else
						deleteBtn->removeChild(messageBox);
				}
				catch (const Share::ShareNotFoundException& e)
				{
					FS_LOG(UI, DEBUG) << "Share already removed!";
					displayShareNotFound();
				}
			});

			messageBox->show();
		});
	}

	void
	ShareEdit::displayRemoved()
	{
		clear();
		addNew<Wt::WTemplate>(tr("template-share-removed"))->addFunction("tr", &Wt::WTemplate::Functions::tr);
	}

	void
	ShareEdit::displayShareNotFound()
	{
		clear();
		addNew<Wt::WTemplate>(tr("template-share-not-found"))->addFunction("tr", &Wt::WTemplate::Functions::tr);
	}

} // namespace UserInterface

