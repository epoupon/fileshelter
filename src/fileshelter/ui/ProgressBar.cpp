/*
 * Copyright (C) 2020 Emeric Poupon
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

#include "ProgressBar.hpp"

namespace UserInterface
{
    ProgressBar::ProgressBar()
        : Wt::WTemplate{ tr("template-progress-bar") }
    {
        _progress = bindNew<Wt::WContainerWidget>("progress");
        _progress->setStyleClass("progress-bar");
        _progress->setAttributeValue("role", "progressbar");
        _progress->setAttributeValue("aria-valuenow", "0");
        _progress->setAttributeValue("aria-valuemin", "0");
        _progress->setAttributeValue("aria-valuemax", "100");
        _progress->setAttributeValue("style", "min-width: 2em;");

        _text = _progress->addNew<Wt::WText>("0%");
    }

    void ProgressBar::setValue(unsigned value)
    {
        _progress->setAttributeValue("aria-valuenow", std::to_string(value));
        _progress->resize(Wt::WLength{ static_cast<double>(value), Wt::LengthUnit::Percentage }, Wt::WLength{ 100, Wt::LengthUnit::Percentage });
        _text->setText(std::to_string(value) + "%");
    }

} // namespace UserInterface
