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

struct Duration
{
	enum class Unit
	{
		Hours,
		Days,
		Weeks,
		Months,
		Years,
	};

	std::size_t value;
	Unit unit;
};

Wt::WDateTime operator+(const Wt::WDateTime& dateTime, const Duration& duration)
{
	switch (duration.unit)
	{
		case Duration::Unit::Hours:
			return dateTime.addSecs(3600 * duration.value);
		case Duration::Unit::Days:
			return dateTime.addDays(duration.value);
		case Duration::Unit::Weeks:
			return dateTime.addDays(7 * duration.value);
		case Duration::Unit::Months:
			return dateTime.addMonths(duration.value);
		case Duration::Unit::Years:
			return dateTime.addYears(duration.value);
	}
	return dateTime;
}

class ShareParameters
{
	public:
		Wt::WString	description;
		Duration	maxDuration;
		std::size_t	maxHits;
		Wt::WString	password;
};

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
	public:

	Wt::Signal<ShareParameters>& validated() { return _sigValidated;}

	ShareCreateFormView(Wt::WContainerWidget *parent = 0)
	: Wt::WTemplateFormView(parent)
	{
		auto model = new ShareCreateFormModel(this);

		setTemplateText(tr("template-form-share-create"));
		addFunction("id", &WTemplate::Functions::id);
		addFunction("block", &WTemplate::Functions::id);

		// Desc
		Wt::WLineEdit *desc = new Wt::WLineEdit();
		setFormWidget(ShareCreateFormModel::DescriptionField, desc);

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
				FS_LOG(UI, DEBUG) << "Validation OK";
				ShareParameters params;

				params.description = model->valueText(ShareCreateFormModel::DescriptionField);
				params.maxDuration.value = Wt::asNumber(model->value(ShareCreateFormModel::DurationValidityField));

				auto unit = model->valueText(ShareCreateFormModel::DurationUnitValidityField);
				if (unit == Wt::WString::tr("msg-hours"))
					params.maxDuration.unit = Duration::Unit::Hours;
				else if (unit == Wt::WString::tr("msg-days"))
					params.maxDuration.unit = Duration::Unit::Days;
				else if (unit == Wt::WString::tr("msg-weeks"))
					params.maxDuration.unit = Duration::Unit::Weeks;
				else if (unit == Wt::WString::tr("msg-months"))
					params.maxDuration.unit = Duration::Unit::Months;
				else if (unit == Wt::WString::tr("msg-years"))
					params.maxDuration.unit = Duration::Unit::Years;

				params.maxHits = Wt::asNumber(model->value(ShareCreateFormModel::HitsValidityField));
				params.password = model->valueText(ShareCreateFormModel::PasswordField);

				_sigValidated.emit(params);
			}

			updateView(model);
		}));

		updateView(model);

	}

	private:
		Wt::Signal<ShareParameters> _sigValidated;

};

ShareCreate::ShareCreate(Wt::WContainerWidget* parent)
: Wt::WContainerWidget(parent),
 _parameters(std::make_shared<ShareParameters>())
{
	refresh();

	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));
}

void
ShareCreate::displayError(Wt::WString error)
{
	clear();

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-created"), this);

	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
	t->bindString("error", error);
}

void
ShareCreate::refresh(void)
{
	if (!wApp->internalPathMatches("/share-create"))
		return;

	clear();

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-create"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	t->bindWidget("msg-max-size", new Wt::WText(Wt::WString::tr("msg-max-file-size").arg( Share::getMaxFileSize() )));

	Wt::WFileUpload *upload = new Wt::WFileUpload();
	upload->setFileTextSize(80);
	upload->setProgressBar(new Wt::WProgressBar());
	t->bindWidget("file", upload);

	ShareCreateFormView* form = new ShareCreateFormView();
	t->bindWidget("form", form);

	upload->fileTooLarge().connect(std::bind([=] ()
	{
		FS_LOG(UI, WARNING) << "File too large!";
		displayError(Wt::WString::tr("msg-share-create-too-large"));
	}));

	form->validated().connect(std::bind([=] (ShareParameters params)
	{
		FS_LOG(UI, DEBUG) << "Starting upload...";

		// Start the upload
		form->hide();
		upload->upload();
		*_parameters = params;
	}, std::placeholders::_1));

	upload->uploaded().connect(std::bind([=] ()
	{
		FS_LOG(UI, DEBUG) << "File successfully uploaded!";

		auto curPath = boost::filesystem::path(upload->spoolFileName());

		// Special case: the user did not choose a file
		if (!boost::filesystem::exists(curPath))
		{
			FS_LOG(UI, ERROR) << "User did not select a file";
			displayError(Wt::WString::tr("msg-no-file-selected"));
			return;
		}

		Wt::Dbo::Transaction transaction(DboSession());

		Database::Share::pointer share = Database::Share::create(DboSession(), curPath);
		if (!share)
		{
			displayError(Wt::WString::tr("msg-internal-error"));
			return;
		}

		upload->stealSpooledFile();
		share.modify()->setDesc(_parameters->description.toUTF8());
		share.modify()->setFileName(upload->clientFileName().toUTF8());
		share.modify()->setMaxHits(_parameters->maxHits);
		share.modify()->setCreationTime(boost::posix_time::second_clock::universal_time());
		share.modify()->setClientAddr(wApp->environment().clientAddress());

		// calculate the expiry date from the duration
		auto now = boost::posix_time::second_clock::universal_time();
		Wt::WDateTime expiryDateTime(now);

		share.modify()->setExpiryTime((expiryDateTime + _parameters->maxDuration).toPosixTime());

		if (!_parameters->password.empty())
			share.modify()->setPassword(_parameters->password.toUTF8());

		transaction.commit();

		FS_LOG(UI, INFO) << "[" << share->getDownloadUUID() << "] Share created. Client = " << share->getClientAddr() << ", size = " << share->getFileSize() << ", name = '" << share->getFileName() << "', desc = '" << share->getDesc() << "', expiry " << share->getExpiryTime() << ", download limit = " << share->getMaxHits() << ", password protected = " << share->hasPassword();

		wApp->setInternalPath("/share-created/" + share->getEditUUID(), true);
	}));

}

} // namespace UserInterface
