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
#include <Wt/WApplication.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

#include "resources/ShareResource.hpp"
#include "share/Exception.hpp"
#include "share/IShareManager.hpp"
#include "share/Types.hpp"
#include "utils/Service.hpp"

#include "ShareDownloadPassword.hpp"
#include "ShareUtils.hpp"

namespace fs::ui
{
    ShareDownload::ShareDownload()
    {
        wApp->internalPathChanged().connect(this, [this] {
            handlePathChanged();
        });

        handlePathChanged();
    }

    void ShareDownload::handlePathChanged()
    {
        clear();

        if (!wApp->internalPathMatches("/share-download"))
            return;

        try
        {
            const share::ShareUUID shareUUID{ wApp->internalPathNextPart("/share-download/") };

            if (Service<share::IShareManager>::get()->shareHasPassword(shareUUID))
                displayPassword(shareUUID);
            else
                displayDownload(Service<share::IShareManager>::get()->getShareDesc(shareUUID));
        }
        catch (const share::ShareNotFoundException& e)
        {
            displayShareNotFound();
        }
        catch (const UUIDException& e)
        {
            displayShareNotFound();
        }
    }

    void ShareDownload::displayDownload(const share::ShareDesc& share, std::optional<std::string_view> password)
    {
        Wt::WTemplate* t{ addNew<Wt::WTemplate>(tr("template-share-download")) };

        t->addFunction("tr", &Wt::WTemplate::Functions::tr);

        if (!share.description.empty())
        {
            t->setCondition("if-desc", true);
            t->bindNew<Wt::WText>("file-desc", Wt::WString::fromUTF8(std::string{ share.description }), Wt::TextFormat::Plain);
        }

        t->bindNew<Wt::WText>("share-size", ShareUtils::fileSizeToString(share.size), Wt::TextFormat::Plain);
        t->bindNew<Wt::WText>("expiry-date-time", share.expiryTime.toString() + " UTC", Wt::TextFormat::Plain);

        {
            Wt::WPushButton* downloadBtn{ t->bindNew<Wt::WPushButton>("download-btn", tr("msg-download")) };
            downloadBtn->setLink(ShareResource::createLink(share.uuid, password));
        }

        {
            auto* filesContainer{ t->bindNew<Wt::WContainerWidget>("files") };
            for (const share::FileDesc& file : share.files)
            {
                Wt::WTemplate* fileTemplate{ filesContainer->addNew<Wt::WTemplate>(tr("template-share-download-file")) };

                fileTemplate->bindString("name", Wt::WString::fromUTF8(std::string{ file.clientPath }), Wt::TextFormat::Plain);
                fileTemplate->bindString("size", ShareUtils::fileSizeToString(file.size), Wt::TextFormat::Plain);
                if (share.files.size() > 1)
                {
                    fileTemplate->setCondition("if-download-btn", true);
                    auto* downloadBtn{ fileTemplate->bindNew<Wt::WPushButton>("download-btn", tr("template-share-download-download-btn"), Wt::TextFormat::XHTML) };
                    downloadBtn->setLink(ShareResource::createLink(share.uuid, file.uuid, password));
                }
            }
        }
    }

    void ShareDownload::displayPassword(const share::ShareUUID& shareUUID)
    {
        auto* view = addNew<ShareDownloadPassword>(shareUUID);
        view->success().connect([this](const share::ShareDesc& share, std::string_view password) {
            clear();
            displayDownload(share, password);
        });
    }

    void ShareDownload::displayShareNotFound()
    {
        clear();
        addNew<Wt::WTemplate>(tr("template-share-not-found"))->addFunction("tr", &Wt::WTemplate::Functions::tr);
    }
} // namespace fs::ui