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

#include "share/Exception.hpp"
#include "share/IShareManager.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

namespace UserInterface
{

	class ShareDownloadPasswordValidator : public Wt::WValidator
	{
		public:
			ShareDownloadPasswordValidator(const Share::ShareUUID& shareUUID)
				: _shareUUID {shareUUID}
			{
				setMandatory(true);
			}

			Result validate(const Wt::WString& input) const override
			{
				_shareDesc.reset();

				{
					auto res {Wt::WValidator::validate(input)};
					if (res.state() != Wt::ValidationState::Valid)
						return res;
				}

				try
				{
					_shareDesc = Service<Share::IShareManager>::get()->getShareDesc(_shareUUID, input.toUTF8());

					return Result {Wt::ValidationState::Valid};
				}
				catch (const Share::ShareNotFoundException& e)
				{
					return Result {Wt::ValidationState::Invalid, Wt::WString::tr("msg-bad-password")};
				}
			}

			const std::optional<Share::ShareDesc>& getShareDesc() const { return _shareDesc; }

		private:
			const Share::ShareUUID _shareUUID;
			mutable std::optional<Share::ShareDesc> _shareDesc; // cache to save password evaluation
	};

	class ShareDownloadPasswordFormModel : public Wt::WFormModel
	{
		public:
			static inline const Field PasswordField {"password"};

			ShareDownloadPasswordFormModel(const Share::ShareUUID& shareUUID)
			{
				addField(PasswordField);

				setValidator(PasswordField, std::make_unique<ShareDownloadPasswordValidator>(shareUUID));
			}

			const std::optional<Share::ShareDesc>& getShareDesc() const
			{
				return std::dynamic_pointer_cast<ShareDownloadPasswordValidator>(validator(PasswordField))->getShareDesc();
			}
	};

	ShareDownloadPassword::ShareDownloadPassword(const Share::ShareUUID& shareUUID)
	{
		auto model {std::make_shared<ShareDownloadPasswordFormModel>(shareUUID)};

		setTemplateText(tr("template-share-download-password"));
		addFunction("id", &WTemplate::Functions::id);
		addFunction("block", &WTemplate::Functions::block);

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

				success().emit(*model->getShareDesc(), model->valueText(ShareDownloadPasswordFormModel::PasswordField).toUTF8());
				return;
			}

			FS_LOG(UI, DEBUG) << "Download password validation failed";

			updateView(model.get());
		});

		updateView(model.get());
	}
} // namespace UserInterface

