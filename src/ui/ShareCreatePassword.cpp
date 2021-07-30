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

#include "ShareCreatePassword.hpp"

#include <thread>
#include <Wt/WFormModel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>

#include "utils/Logger.hpp"
#include "PasswordUtils.hpp"

namespace UserInterface
{
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
				if (res.state() != Wt::ValidationState::Valid)
					return res;

				if (PasswordUtils::checkUploadPassord(input.toUTF8()))
					return Result {Wt::ValidationState::Valid};

				return Result {Wt::ValidationState::Invalid, Wt::WString::tr("msg-bad-password")};
			}

	};

	class ShareCreatePasswordFormModel : public Wt::WFormModel
	{
		public:
			static const Field PasswordField;

			ShareCreatePasswordFormModel()
			{
				addField(PasswordField);

				setValidator(PasswordField, std::make_unique<ShareCreatePasswordValidator>());
			}
	};

	const Wt::WFormModel::Field ShareCreatePasswordFormModel::PasswordField = "password";

	ShareCreatePassword::ShareCreatePassword()
	{
		auto model = std::make_shared<ShareCreatePasswordFormModel>();

		setTemplateText(tr("template-share-create-password"));
		addFunction("id", &WTemplate::Functions::id);
		addFunction("block", &WTemplate::Functions::block);

		// Password
		auto password = std::make_unique<Wt::WLineEdit>();
		password->setEchoMode(Wt::EchoMode::Password);
		setFormWidget(ShareCreatePasswordFormModel::PasswordField, std::move(password));

		// Buttons
		Wt::WPushButton* unlockBtn {bindNew<Wt::WPushButton>("unlock-btn", tr("msg-unlock"))};
		unlockBtn->clicked().connect([=]
		{
			updateModel(model.get());

			if (model->validate())
			{
				FS_LOG(UI, DEBUG) << "Create password validation OK";
				success().emit();
				return;
			}

			FS_LOG(UI, DEBUG) << "Create password validation failed";

			// Mitigate brute force attemps
			// TODO per peer/share
			std::this_thread::sleep_for(std::chrono::seconds {1});

			updateView(model.get());
		});

		updateView(model.get());
	}


} // namespace UserInterface

