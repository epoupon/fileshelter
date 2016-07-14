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
#include <Wt/WSignal>
#include <Wt/WTemplateFormView>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WDateEdit>
#include <Wt/WSpinBox>
#include <Wt/WIntValidator>
#include <Wt/WDateValidator>
#include <Wt/WAbstractItemModel>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCreate.hpp"

namespace UserInterface {


class ShareCreateFormModel : public Wt::WFormModel
{
	public:

		// Associate each field with a unique string literal.
		static const Field DescriptionField;
		static const Field FileField;
		static const Field ExpiracyDateField;
		static const Field HitsValidityField;
		static const Field PasswordField;
		static const Field PasswordConfirmField;

		ShareCreateFormModel(Wt::WObject *parent = 0)
			: Wt::WFormModel(parent)
		{
			addField(DescriptionField, Wt::WString::tr("msg-optional"));
			addField(FileField);
			addField(ExpiracyDateField);
			addField(HitsValidityField);
			addField(PasswordField, Wt::WString::tr("msg-optional"));
			addField(PasswordConfirmField);

			auto maxHits = Config::instance().getULong("max-validity-hits", 0);
			auto maxDurationDays = Config::instance().getULong("max-validity-days", 0);

			auto hitsValidator = new Wt::WIntValidator();
			hitsValidator->setMandatory(true);
			if (maxHits > 0)
			{
				hitsValidator->setBottom(1);
				hitsValidator->setTop(maxHits);
			}
			else
			{
				hitsValidator->setBottom(0);
			}
			setValidator(HitsValidityField, hitsValidator);

			auto dateValidator = new Wt::WDateValidator();
			dateValidator->setMandatory(true);
			dateValidator->setBottom(Wt::WDate::currentServerDate().addDays(1));
			if (maxDurationDays > 0)
				dateValidator->setTop(Wt::WDate::currentServerDate().addDays(maxDurationDays));
			setValidator(ExpiracyDateField, dateValidator);

			setValue(ExpiracyDateField, Wt::WDate::currentServerDate().addDays(7));
			setValue(HitsValidityField, 30);

		}

		bool validateField(Field field)
		{
			Wt::WString error; // empty means validated

			if (field == PasswordConfirmField)
			{
				if (valueText(PasswordField) != valueText(PasswordConfirmField))
					error = Wt::WString::tr("msg-passwords-dont-match");
			}
			else
			{
				return Wt::WFormModel::validateField(field);
			}

			setValidation(field, Wt::WValidator::Result( error.empty() ? Wt::WValidator::Valid : Wt::WValidator::Invalid, error));
			return validation(field).state() == Wt::WValidator::Valid;
		}

};

const Wt::WFormModel::Field ShareCreateFormModel::DescriptionField = "desc";
const Wt::WFormModel::Field ShareCreateFormModel::FileField = "file";
const Wt::WFormModel::Field ShareCreateFormModel::ExpiracyDateField = "expiracy-date";
const Wt::WFormModel::Field ShareCreateFormModel::HitsValidityField = "hits-validity";
const Wt::WFormModel::Field ShareCreateFormModel::PasswordField = "password";
const Wt::WFormModel::Field ShareCreateFormModel::PasswordConfirmField = "password-confirm";

class ShareCreateFormView : public Wt::WTemplateFormView
{
	private:
		Wt::Signal<Wt::WString> _sigFailed;

	public:

	Wt::Signal<Wt::WString>& failed() { return _sigFailed;}

	ShareCreateFormView(Wt::WContainerWidget *parent = 0)
	: Wt::WTemplateFormView(parent)
	{
		auto model = new ShareCreateFormModel(this);

		setTemplateText(tr("template-share-create"));
		addFunction("id", &WTemplate::Functions::id);
		addFunction("block", &WTemplate::Functions::id);

		// Desc
		Wt::WLineEdit *desc = new Wt::WLineEdit();
		setFormWidget(ShareCreateFormModel::DescriptionField, desc);

		// File
		Wt::WFileUpload *upload = new Wt::WFileUpload();
		upload->setFileTextSize(80);
		upload->setProgressBar(new Wt::WProgressBar());
		setFormWidget(ShareCreateFormModel::FileField, upload);

		// Time validity
		Wt::WDateEdit *expiracyDate = new Wt::WDateEdit();
		setFormWidget(ShareCreateFormModel::ExpiracyDateField, expiracyDate);

		// Hits validity
		Wt::WSpinBox *hitsValidity = new Wt::WSpinBox();
		setFormWidget(ShareCreateFormModel::HitsValidityField, hitsValidity);

		// Password
		Wt::WLineEdit *password = new Wt::WLineEdit();
		password->setEchoMode(Wt::WLineEdit::Password);
		setFormWidget(ShareCreateFormModel::PasswordField, password);

		// Password confirm
		Wt::WLineEdit *passwordConfirm = new Wt::WLineEdit();
		passwordConfirm->setEchoMode(Wt::WLineEdit::Password);
		setFormWidget(ShareCreateFormModel::PasswordConfirmField, passwordConfirm);

		// Buttons
		Wt::WPushButton *uploadBtn = new Wt::WPushButton(tr("msg-upload"));
		uploadBtn->setStyleClass("btn-primary");
		bindWidget("create-btn", uploadBtn);
		uploadBtn->clicked().connect(std::bind([=]
		{
			updateModel(model);

			if (model->validate())
			{
				FS_LOG(UI, DEBUG) << "validation OK";

				upload->upload();
				uploadBtn->disable();
			}

			updateView(model);
		}));

		upload->fileTooLarge().connect(std::bind([=] ()
		{
			FS_LOG(UI, DEBUG) << "File too large!";
			failed().emit(Wt::WString::tr("msg-create-share-too-large"));
		}));

		upload->uploaded().connect(std::bind([=] ()
		{
			FS_LOG(UI, DEBUG) << "Uploaded!";

			// Filename is the DownloadUID
			auto curPath = boost::filesystem::path(upload->spoolFileName());
			// Special case: the user did not choose a file
			if (!boost::filesystem::exists(curPath))
			{
				FS_LOG(UI, ERROR) << "User did not select a file";
				failed().emit(Wt::WString::tr("msg-no-file-selected"));
				return;
			}

			Wt::Dbo::Transaction transaction(DboSession());

			Database::Share::pointer share = Database::Share::create(DboSession());

			auto storePath = Config::instance().getPath("working-dir") / "files" / share->getDownloadUUID();

			boost::system::error_code ec;
			boost::filesystem::rename(curPath, storePath, ec);
			if (ec)
			{
				FS_LOG(UI, INFO) << "Move file failed from " << curPath << " to " << storePath << ": " << ec.message();

				// fallback on copy
				boost::filesystem::copy_file(curPath, storePath, ec);
				if (ec)
				{
					FS_LOG(UI, ERROR) << "Copy file failed from " << curPath << " to " << storePath << ": " << ec.message();
					failed().emit(Wt::WString::tr("msg-internal-error"));
					share.remove();
					return;
				}
			}
			else
				upload->stealSpooledFile();


			share.modify()->setDesc(model->valueText(ShareCreateFormModel::DescriptionField).toUTF8());
			share.modify()->setPath(storePath);
			share.modify()->setFileName(upload->clientFileName().toUTF8());
			share.modify()->setFileSize(boost::filesystem::file_size(storePath));
			share.modify()->setMaxHits(Wt::asNumber(model->value(ShareCreateFormModel::HitsValidityField)));

			Wt::WDate date = expiracyDate->date();
			share.modify()->setExpiracyDate(boost::gregorian::date(date.year(), date.month(), date.day()));
			if (!model->valueText(ShareCreateFormModel::PasswordField).empty())
				share.modify()->setPassword(model->valueText(ShareCreateFormModel::PasswordField));

			transaction.commit();

			wApp->setInternalPath("/share-created/" + share->getEditUUID(), true);
		}));

		updateView(model);

	}

};

ShareCreate::ShareCreate(Wt::WContainerWidget* parent)
: Wt::WContainerWidget(parent)
{
	refresh();

	wApp->internalPathChanged().connect(std::bind([=]
	{
			refresh();
	}));
}

void
ShareCreate::refresh(void)
{
	if (!wApp->internalPathMatches("/share-create"))
		return;

	clear();

	ShareCreateFormView* form = new ShareCreateFormView(this);

	form->failed().connect(std::bind([=] (Wt::WString error)
	{
		FS_LOG(UI, DEBUG) << "Handling error: " << error;
		clear();
		Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-created"), this);
		t->addFunction("tr", &Wt::WTemplate::Functions::tr);
		t->bindString("error", error);
	}, std::placeholders::_1));
}

} // namespace UserInterface
