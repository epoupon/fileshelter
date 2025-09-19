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

#include <Wt/WApplication.h>
#include <Wt/WTemplate.h>

#include "share/Exception.hpp"
#include "share/IShareManager.hpp"
#include "share/Types.hpp"
#include "utils/Service.hpp"

#include "ShareUtils.hpp"

namespace UserInterface
{
    ShareCreated::ShareCreated()
    {
        wApp->internalPathChanged().connect(this, [this] {
            handlePathChanged();
        });

        handlePathChanged();
    }

    void ShareCreated::handlePathChanged()
    {
        clear();

        if (!wApp->internalPathMatches("/share-created"))
            return;

        try
        {
            const Share::ShareEditUUID shareEditUUID{ wApp->internalPathNextPart("/share-created/") };

            const Share::ShareDesc share{ Service<Share::IShareManager>::get()->getShareDesc(shareEditUUID) };

            Wt::WTemplate* t{ addNew<Wt::WTemplate>(tr("template-share-created")) };
            t->addFunction("tr", &Wt::WTemplate::Functions::tr);

            t->bindWidget("download-link", ShareUtils::createShareDownloadAnchor(share.uuid));
            t->bindWidget("edit-link", ShareUtils::createShareEditAnchor(shareEditUUID));
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

    void ShareCreated::displayShareNotFound()
    {
        clear();
        addNew<Wt::WTemplate>(tr("template-share-not-found"))->addFunction("tr", &Wt::WTemplate::Functions::tr);
    }

} // namespace UserInterface
