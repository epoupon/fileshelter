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

#include <Wt/WEnvironment>
#include <Wt/WFormModel>
#include <Wt/WSignal>
#include <Wt/WTemplateFormView>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WDateEdit>
#include <Wt/WSpinBox>
#include <Wt/WComboBox>
#include <Wt/WIntValidator>
#include <Wt/WDateValidator>
#include <Wt/WDateValidator>
#include <Wt/WStringListModel>
#include <Wt/WAbstractItemModel>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCreate.hpp"

namespace UserInterface {

using namespace Database;

class ShareCreateFormModel : public Wt::WFormModel
{
	public:

		// Associate each field with a unique string literal.
		static const Field DescriptionField;
		static const Field FileField;
		static const Field DurationValidityField;
		static const Field DurationUnitValidityField;
		static const Field HitsValidityField;
		static const Field PasswordField;
		static const Field PasswordConfirmField;

		ShareCreateFormModel(Wt::WObject *parent = 0)
			: Wt::WFormModel(parent)
		{
			addField(DescriptionField, Wt::WString::tr("msg-optional"));
			addField(FileField, Wt::WString::tr("msg-max-file-size").arg( Share::getMaxFileSize()));
			addField(DurationValidityField);
			addField(DurationUnitValidityField);
			addField(HitsValidityField);
			addField(PasswordField, Wt::WString::tr("msg-optional"));
			addField(PasswordConfirmField);

			initializeModels();

			// Duration Validity Unit
			setValidator(DurationUnitValidityField, new Wt::WValidator(true));  // mandatory

			// Duration Validity
			auto durationValidator = new Wt::WIntValidator();
			durationValidator->setMandatory(true);
			durationValidator->setBottom(1);
			setValidator(DurationValidityField, durationValidator);

			setValue(DurationValidityField, 1);
			setValue(DurationUnitValidityField, Wt::WString::tr("msg-days"));
			updateDurationValidator();

			// Hits validity
			auto maxValidityHits = Share::getMaxValidatityHits();
			auto hitsValidator = new Wt::WIntValidator();
			hitsValidator->setMandatory(true);
			if (maxValidityHits > 0)
			{
				hitsValidator->setBottom(1);
				hitsValidator->setTop(maxValidityHits);
			}
			else
			{
				hitsValidator->setBottom(0);
			}
			setValidator(HitsValidityField, hitsValidator);

			int suggestedValidityHits;
			if (maxValidityHits != 0 && maxValidityHits < 10)
				suggestedValidityHits = maxValidityHits;
			else
				suggestedValidityHits = 10;

			setValue(HitsValidityField, suggestedValidityHits);
		}

		void updateDurationValidator(void)
		{
			auto maxValidityDuration = Share::getMaxValidatityDuration();
			auto durationValidator = dynamic_cast<Wt::WIntValidator*>(validator(DurationValidityField));

			int maxValue = 1;
			auto unit = valueText(DurationUnitValidityField);
			if (unit == Wt::WString::tr("msg-hours"))
				maxValue = maxValidityDuration.hours();
			else if (unit == Wt::WString::tr("msg-days"))
				maxValue = maxValidityDuration.hours() / 24;
			else if (unit == Wt::WString::tr("msg-weeks"))
				maxValue = maxValidityDuration.hours() / 24 / 7;
			else if (unit == Wt::WString::tr("msg-months"))
				maxValue = maxValidityDuration.hours() / 24 / 31;
			else if (unit == Wt::WString::tr("msg-years"))
				maxValue = maxValidityDuration.hours() / 24 / 365;

			durationValidator->setTop(maxValue);

			// If the current value is too high, change it to the max
			if (Wt::asNumber(value(DurationValidityField)) > maxValue)
				setValue(DurationValidityField, maxValue);
		}

		bool validateField(Field field)
		{
			Wt::WString error; // empty means validated

			if (field == PasswordConfirmField)
			{
				if (valueText(PasswordField) != valueText(PasswordConfirmField))
					error = Wt::WString::tr("msg-passwords-dont-match");
			}
			else if (field == DurationUnitValidityField)
			{
				// Since they are grouped together,
				// make it share the same state as DurationValidityField
				validateField(DurationValidityField);
				setValidation(field, validation(DurationValidityField));
				return validation(field).state() == Wt::WValidator::Valid;
			}
			else
			{
				return Wt::WFormModel::validateField(field);
			}

			setValidation(field, Wt::WValidator::Result( error.empty() ? Wt::WValidator::Valid : Wt::WValidator::Invalid, error));
			return validation(field).state() == Wt::WValidator::Valid;
		}

		Wt::WAbstractItemModel *durationValidityModel() { return _durationValidityModel; }

	private:

		void initializeModels(void)
		{
			auto maxDuration = Share::getMaxValidatityDuration();

			_durationValidityModel = new Wt::WStringListModel(this);

			_durationValidityModel->addString( Wt::WString::tr("msg-hours") );
			_durationValidityModel->addString( Wt::WString::tr("msg-days") );

			if (maxDuration >= boost::posix_time::hours(7*24))
				_durationValidityModel->addString( Wt::WString::tr("msg-weeks") );
			if (maxDuration >= boost::posix_time::hours(31*24))
				_durationValidityModel->addString( Wt::WString::tr("msg-months") );
			if (maxDuration >= boost::posix_time::hours(365*24))
				_durationValidityModel->addString( Wt::WString::tr("msg-years") );

		}

		Wt::WStringListModel*	_durationValidityModel;
};

const Wt::WFormModel::Field ShareCreateFormModel::DescriptionField = "desc";
const Wt::WFormModel::Field ShareCreateFormModel::FileField = "file";
const Wt::WFormModel::Field ShareCreateFormModel::DurationValidityField = "duration-validity";
const Wt::WFormModel::Field ShareCreateFormModel::DurationUnitValidityField = "duration-unit-validity";
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

		// Duration validity
		auto durationValidity = new Wt::WSpinBox();
		setFormWidget(ShareCreateFormModel::DurationValidityField, durationValidity);

		// Duration validity unit
		auto durationUnitValidity = new Wt::WComboBox();
		setFormWidget(ShareCreateFormModel::DurationUnitValidityField, durationUnitValidity);
		durationUnitValidity->setModel(model->durationValidityModel());

		// each time the unit is changed, make sure to update the limits
		durationUnitValidity->changed().connect(std::bind([=]
		{
			updateModel(model);
			model->updateDurationValidator();

			// Revalidate the fields if necessary
			if (model->isValidated(ShareCreateFormModel::DurationValidityField))
			{
				model->validateField(ShareCreateFormModel::DurationValidityField);
				model->validateField(ShareCreateFormModel::DurationUnitValidityField);
			}

			updateView(model);
		}));

		// Hits validity
		auto hitsValidity = new Wt::WSpinBox();
		setFormWidget(ShareCreateFormModel::HitsValidityField, hitsValidity);

		// Password
		auto password = new Wt::WLineEdit();
		password->setEchoMode(Wt::WLineEdit::Password);
		setFormWidget(ShareCreateFormModel::PasswordField, password);

		// Password confirm
		auto passwordConfirm = new Wt::WLineEdit();
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
			FS_LOG(UI, WARNING) << "File too large!";
			failed().emit(Wt::WString::tr("msg-share-create-too-large"));
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

			Database::Share::pointer share = Database::Share::create(DboSession(), curPath);
			if (!share)
			{
				failed().emit(Wt::WString::tr("msg-internal-error"));
				return;
			}

			upload->stealSpooledFile();
			share.modify()->setDesc(model->valueText(ShareCreateFormModel::DescriptionField).toUTF8());
			share.modify()->setFileName(upload->clientFileName().toUTF8());
			share.modify()->setMaxHits(Wt::asNumber(model->value(ShareCreateFormModel::HitsValidityField)));

			share.modify()->setCreationTime(boost::posix_time::second_clock::universal_time());
			share.modify()->setClientAddr(wApp->environment().clientAddress());

			// calculate the expiry date from the duration
			auto now = boost::posix_time::second_clock::universal_time();

			Wt::WDateTime expiryDateTime(now);
			auto duration = Wt::asNumber(model->value(ShareCreateFormModel::DurationValidityField));
			auto unit = model->valueText(ShareCreateFormModel::DurationUnitValidityField);
			if (unit == Wt::WString::tr("msg-hours"))
				expiryDateTime = expiryDateTime.addSecs(3600 * duration);
			else if (unit == Wt::WString::tr("msg-days"))
				expiryDateTime = expiryDateTime.addDays(duration);
			else if (unit == Wt::WString::tr("msg-weeks"))
				expiryDateTime = expiryDateTime.addDays(7 * duration);
			else if (unit == Wt::WString::tr("msg-months"))
				expiryDateTime = expiryDateTime.addMonths(duration);
			else if (unit == Wt::WString::tr("msg-years"))
				expiryDateTime = expiryDateTime.addYears(duration);

			share.modify()->setExpiryTime(expiryDateTime.toPosixTime());

			if (!model->valueText(ShareCreateFormModel::PasswordField).empty())
				share.modify()->setPassword(model->valueText(ShareCreateFormModel::PasswordField));

			transaction.commit();

			FS_LOG(UI, INFO) << "[" << share->getDownloadUUID() << "] Share created. Client = " << share->getClientAddr() << ", size = " << share->getFileSize() << ", name = '" << share->getFileName() << "', desc = '" << share->getDesc() << "', expiry " << share->getExpiryTime() << ", download limit = " << share->getMaxHits() << ", password protected = " << share->hasPassword();

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
