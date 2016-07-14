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

#include <Wt/WFormModel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "SharePasswordFormView.hpp"

namespace UserInterface {

class SharePasswordValidator : public Wt::WValidator
{
	public:
		SharePasswordValidator(std::string downloadUUID, Wt::WObject *parent = 0)
		: Wt::WValidator(parent),
		 _downloadUUID(downloadUUID)
		{
			setMandatory(true);
		}

		Result validate(const Wt::WString& input) const
		{
			auto res = Wt::WValidator::validate(input);
			if (res.state() != Valid)
				return res;

			Wt::Dbo::Transaction transaction(DboSession());

			Database::Share::pointer share = Database::Share::getByDownloadUUID(DboSession(), _downloadUUID);

			FS_LOG(UI, DEBUG) << "share = " << share;

			if (share && share->verifyPassword(input))
				return Result(Valid);

			return Result(Invalid, Wt::WString::tr("msg-bad-password"));
		}

	private:
		std::string _downloadUUID;
};


class SharePasswordFormModel : public Wt::WFormModel
{
	public:
	static const Field PasswordField;

	SharePasswordFormModel(std::string downloadUUID, Wt::WObject *parent)
	: Wt::WFormModel(parent)
	{
		addField(PasswordField);

		setValidator(PasswordField, new SharePasswordValidator(downloadUUID));
	}
};

const Wt::WFormModel::Field SharePasswordFormModel::PasswordField = "password";

SharePasswordFormView::SharePasswordFormView(Wt::WContainerWidget *parent)
: Wt::WTemplateFormView(parent)
{
	std::string downloadUUID = wApp->internalPathNextPart("/share-download/");
	auto model = new SharePasswordFormModel(downloadUUID, this);

	setTemplateText(tr("template-share-download-password"));
	addFunction("id", &WTemplate::Functions::id);
	addFunction("block", &WTemplate::Functions::id);

	// Password
	Wt::WLineEdit *password = new Wt::WLineEdit();
	password->setEchoMode(Wt::WLineEdit::Password);
	setFormWidget(SharePasswordFormModel::PasswordField, password);

	// Buttons
	Wt::WPushButton *unlockBtn = new Wt::WPushButton(tr("msg-unlock"));
	unlockBtn->setStyleClass("btn-primary");
	bindWidget("unlock-btn", unlockBtn);
	unlockBtn->clicked().connect(std::bind([=]
	{
		updateModel(model);

		if (model->validate())
		{
			FS_LOG(UI, DEBUG) << "Password validation OK";

			success().emit();
			return;
		}

		updateView(model);
	}));

	updateView(model);
}


} // namespace UserInterface

