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

#include "ShareDownloadPassword.hpp"

#include <Wt/WFormModel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"

namespace UserInterface {

class ShareDownloadPasswordValidator : public Wt::WValidator
{
	public:
		ShareDownloadPasswordValidator(std::string downloadUUID)
		: _downloadUUID(downloadUUID)
		{
			setMandatory(true);
		}

		Result validate(const Wt::WString& input) const
		{
			auto res = Wt::WValidator::validate(input);
			if (res.state() != Wt::ValidationState::Valid)
				return res;

			Wt::Dbo::Transaction transaction(FsApp->getDboSession());

			Database::Share::pointer share = Database::Share::getByDownloadUUID(FsApp->getDboSession(), _downloadUUID);

			if (share && share->verifyPassword(input))
				return Result(Wt::ValidationState::Valid);

			return Result(Wt::ValidationState::Invalid, Wt::WString::tr("msg-bad-password"));
		}

	private:
		std::string _downloadUUID;
};


class ShareDownloadPasswordFormModel : public Wt::WFormModel
{
	public:
	static const Field PasswordField;

	ShareDownloadPasswordFormModel(std::string downloadUUID)
	{
		addField(PasswordField);

		setValidator(PasswordField, std::make_unique<ShareDownloadPasswordValidator>(downloadUUID));
	}
};

const Wt::WFormModel::Field ShareDownloadPasswordFormModel::PasswordField = "password";

ShareDownloadPassword::ShareDownloadPassword()
{
	std::string downloadUUID = wApp->internalPathNextPart("/share-download/");

	auto model = std::make_shared<ShareDownloadPasswordFormModel>(downloadUUID);

	setTemplateText(tr("template-share-download-password"));
	addFunction("id", &WTemplate::Functions::id);
	addFunction("block", &WTemplate::Functions::id);

	// Password
	auto password = std::make_unique<Wt::WLineEdit>();
	password->setEchoMode(Wt::EchoMode::Password);
	setFormWidget(ShareDownloadPasswordFormModel::PasswordField, std::move(password));

	// Buttons
	Wt::WPushButton *unlockBtn = bindNew<Wt::WPushButton>("unlock-btn", tr("msg-unlock"));
	unlockBtn->clicked().connect([=]
	{
		updateModel(model.get());

		if (model->validate())
		{
			FS_LOG(UI, DEBUG) << "Download password validation OK";

			success().emit();
			return;
		}

		FS_LOG(UI, DEBUG) << "Download password validation failed";

		updateView(model.get());
	});

	updateView(model.get());
}


} // namespace UserInterface

