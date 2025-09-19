/*
 * Copyright (C) 2021 Emeric Poupon
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

#include "TermsOfService.hpp"

#include <fstream>

#include <Wt/WTemplate.h>

#include "utils/IConfig.hpp"
#include "utils/Service.hpp"

namespace UserInterface
{
    std::unique_ptr<Wt::WWidget> createTermsOfService()
    {
        auto tos{ std::make_unique<Wt::WTemplate>() };

        // Override the ToS with a custom version is specified
        if (const auto path{ Service<IConfig>::get()->getPath("tos-custom") }; !path.empty())
        {
            std::ifstream ifs{ path.string().c_str() };
            std::stringstream buffer;
            buffer << ifs.rdbuf();

            tos->setTemplateText(buffer.str());
        }
        else
        {
            tos->setTemplateText(Wt::WString::tr("template-tos"));
            tos->addFunction("tr", &Wt::WTemplate::Functions::tr);
        }

        return tos;
    }
} // namespace UserInterface
