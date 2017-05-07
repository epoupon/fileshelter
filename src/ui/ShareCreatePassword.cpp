/*
 * Copyright (C) 2017 Emeric Poupon
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

#include <Wt/WFormModel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>

#include "utils/Logger.hpp"
#include "utils/Config.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCreatePassword.hpp"

namespace UserInterface {

class ShareCreatePasswordValidator : public Wt::WValidator
{
	public:
		ShareCreatePasswordValidator(Wt::WObject *parent = 0)
		: Wt::WValidator(parent)
		{
			setMandatory(true);
		}

		Result validate(const Wt::WString& input) const
		{
			auto res = Wt::WValidator::validate(input);
			if (res.state() != Valid)
				return res;

			auto password = Config::instance().getString("upload-password", "");

			if (input.toUTF8() == password)
				return Result(Valid);

			return Result(Invalid, Wt::WString::tr("msg-bad-password"));
		}

};

class ShareCreatePasswordFormModel : public Wt::WFormModel
{
	public:
		static const Field PasswordField;

		ShareCreatePasswordFormModel(Wt::WObject *parent)
		: Wt::WFormModel(parent)
		{
			addField(PasswordField);

			setValidator(PasswordField, new ShareCreatePasswordValidator());
		}
};

const Wt::WFormModel::Field ShareCreatePasswordFormModel::PasswordField = "password";

ShareCreatePassword::ShareCreatePassword(Wt::WContainerWidget *parent)
: Wt::WTemplateFormView(parent)
{
	auto model = new ShareCreatePasswordFormModel(this);

	setTemplateText(tr("template-share-create-password"));
	addFunction("id", &WTemplate::Functions::id);
	addFunction("block", &WTemplate::Functions::id);

	// Password
	Wt::WLineEdit *password = new Wt::WLineEdit();
	password->setEchoMode(Wt::WLineEdit::Password);
	setFormWidget(ShareCreatePasswordFormModel::PasswordField, password);

	// Buttons
	Wt::WPushButton *unlockBtn = new Wt::WPushButton(tr("msg-unlock"));
	unlockBtn->setStyleClass("btn-primary");
	bindWidget("unlock-btn", unlockBtn);
	unlockBtn->clicked().connect(std::bind([=]
	{
		updateModel(model);

		if (model->validate())
		{
			FS_LOG(UI, DEBUG) << "Create password validation OK";

			success().emit();
			return;
		}

		FS_LOG(UI, DEBUG) << "Create password validation failed";

		// Mitigate brute force attemps
		sleep(1);

		updateView(model);
	}));

	updateView(model);
}


} // namespace UserInterface

