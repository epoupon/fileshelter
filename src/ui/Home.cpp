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
#include <Wt/WAnchor>

#include "Home.hpp"

namespace UserInterface {

Home::Home(Wt::WContainerWidget* parent)
: Wt::WContainerWidget( parent)
{
	Wt::WTemplate *home = new Wt::WTemplate(Wt::WString::tr("template-home"), this);
	home->addFunction("tr", &Wt::WTemplate::Functions::tr);

	Wt::WPushButton *createBtn = new Wt::WPushButton(Wt::WString::tr("msg-share-create"), Wt::XHTMLText);
	createBtn->addStyleClass("btn-primary");
	createBtn->setLink( Wt::WLink(Wt::WLink::InternalPath, "/share-create") );

	home->bindWidget("share-create-btn", createBtn);

	home->bindWidget("github-link", new Wt::WAnchor("https://github.com/epoupon/fileshelter", "<i class=\"fa fa-github\"></i>"));
}

} // namespace UserInterface

