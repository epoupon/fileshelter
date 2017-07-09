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
#include <Wt/WTemplateFormView>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WSpinBox>
#include <Wt/WComboBox>
#include <Wt/WIntValidator>
#include <Wt/WStringListModel>
#include <Wt/WAbstractItemModel>
#include <Wt/WDateTime>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"
#include "utils/Zip.hpp"

#include "database/Share.hpp"

#include "ShareCreatePassword.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCommon.hpp"
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

static std::pair<std::size_t, std::size_t> computeHitsRange(void)
{
	std::size_t max, min;
	auto maxValidityHits = Share::getMaxValidatityHits();

	if (maxValidityHits > 0)
	{
		min = 1;
		max = maxValidityHits;
	}
	else
	{
		min = 0;
		max = std::numeric_limits<int>::max();
	}

	return std::make_pair(min, max);
}

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

		ShareCreateFormModel(Wt::WObject *parent = 0)
			: Wt::WFormModel(parent)
		{
			addField(DescriptionField, Wt::WString::tr("msg-optional"));
			addField(DurationValidityField);
			addField(DurationUnitValidityField);
			addField(HitsValidityField);
			addField(PasswordField, Wt::WString::tr("msg-optional"));

			initializeModels();

			// Duration Validity Unit
			setValidator(DurationUnitValidityField, new Wt::WValidator(true));  // mandatory

			// Duration Validity
			auto durationValidator = new Wt::WIntValidator();
			durationValidator->setMandatory(true);
			durationValidator->setBottom(1);
			setValidator(DurationValidityField, durationValidator);

			updateValidityDuration( Share::getDefaultValidatityDuration() );
			updateDurationValidator();

			// Hits validity
			auto hitsRange = computeHitsRange();
			auto hitsValidator = new Wt::WIntValidator();
			hitsValidator->setMandatory(true);
			hitsValidator->setRange(hitsRange.first, hitsRange.second);
			setValidator(HitsValidityField, hitsValidator);

			std::size_t suggestedValidityHits = Share::getDefaultValidatityHits();
			if (suggestedValidityHits > hitsRange.second)
				suggestedValidityHits = hitsRange.second;

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

			if (field == DurationUnitValidityField)
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

		void updateValidityDuration(boost::posix_time::time_duration duration)
		{
			auto maxValidityDuration = Share::getMaxValidatityDuration();
			if (duration > maxValidityDuration)
				duration = maxValidityDuration;

			int value;
			Wt::WString unit;
			if (duration.hours() % 24)
			{
				value = duration.hours();
				unit = Wt::WString::tr("msg-hours");
			}
			if ((duration.hours() / 24 % 365) == 0)
			{
				value = duration.hours() / 24 / 365;
				unit = Wt::WString::tr("msg-years");
			}
			else if ((duration.hours() / 24 % 31) == 0)
			{
				value = duration.hours() / 24 / 31;
				unit = Wt::WString::tr("msg-months");
			}
			else if ((duration.hours() / 24 % 7) == 0)
			{
				value = duration.hours() / 24 / 7;
				unit = Wt::WString::tr("msg-weeks");
			}
			else
			{
				value = duration.hours() / 24;
				unit = Wt::WString::tr("msg-days");
			}

			setValue(DurationValidityField, value);
			setValue(DurationUnitValidityField, unit);
		}

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
		auto hitsRange = computeHitsRange();
		auto hitsValidity = new Wt::WSpinBox();
		hitsValidity->setRange(hitsRange.first, hitsRange.second);
		setFormWidget(ShareCreateFormModel::HitsValidityField, hitsValidity);

		// Password
		auto password = new Wt::WLineEdit();
		password->setEchoMode(Wt::WLineEdit::Password);
		setFormWidget(ShareCreateFormModel::PasswordField, password);

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
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
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

	if (!Config::instance().getString("upload-password", "").empty())
		displayPassword();
	else
		displayCreate();
}

void
ShareCreate::displayPassword()
{
	clear();

	auto view = new ShareCreatePassword(this);
	view->success().connect(std::bind([=]
	{
		displayCreate();
	}));
}

void
ShareCreate::displayCreate()
{
	clear();

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-create"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	t->bindWidget("msg-max-size", new Wt::WText(Wt::WString::tr("msg-max-file-size").arg( sizeToString(Share::getMaxFileSize() * 1024*1024) )));

	Wt::WFileUpload *upload = new Wt::WFileUpload();
	upload->setFileTextSize(80);
	upload->setProgressBar(new Wt::WProgressBar());
	upload->setMultiple(true);
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
		auto uploadedFiles = upload->uploadedFiles();

		FS_LOG(UI, DEBUG) << uploadedFiles.size() << " file" << (uploadedFiles.size() > 1 ? "s" : "") << " successfully uploaded!";

		// Special case: the user did not choose a file
		if (uploadedFiles.empty())
		{
			FS_LOG(UI, ERROR) << "User did not select a file";
			displayError(Wt::WString::tr("msg-no-file-selected"));
			return;
		}

		boost::filesystem::path sharePath;
		Wt::WString fileName;

		if (uploadedFiles.size() == 1)
		{
			sharePath = boost::filesystem::path(uploadedFiles.front().spoolFileName());
			fileName = Wt::WString::fromUTF8(uploadedFiles.front().clientFileName());
			uploadedFiles.front().stealSpoolFile();
		}
		else
		{
			// Generate a zip that contains all the files in it
			// Name it using the description, fall back on the first file name otherwise

			Wt::WString description = _parameters->description;
			if (!description.empty())
				fileName = description + ".zip";
			else
				fileName = uploadedFiles.front().clientFileName() + ".zip";

			sharePath = Config::instance().getPath("working-dir") / "tmp" / generateUUID();
			try
			{
				ZipFileWriter zip(sharePath);

				std::vector<boost::filesystem::path> files;
				for (auto uploadedFile : uploadedFiles)
				{
					zip.add(uploadedFile.clientFileName(),
						boost::filesystem::path(uploadedFile.spoolFileName()));
				}
			}
			catch(std::exception& e)
			{
				FS_LOG(UI, ERROR) << "Cannot create zip file: " << e.what();
				displayError(Wt::WString::tr("msg-internal-error"));
				return;
			}
		}

		Wt::Dbo::Transaction transaction(DboSession());

		Database::Share::pointer share = Database::Share::create(DboSession(), sharePath);
		if (!share)
		{
			displayError(Wt::WString::tr("msg-internal-error"));
			return;
		}

		share.modify()->setDesc(_parameters->description.toUTF8());
		share.modify()->setFileName(fileName.toUTF8());
		share.modify()->setMaxHits(_parameters->maxHits);
		share.modify()->setCreationTime(boost::posix_time::second_clock::universal_time());
		share.modify()->setClientAddr(wApp->environment().clientAddress());

		// calculate the expiry date from the duration
		auto now = boost::posix_time::second_clock::universal_time();
		Wt::WDateTime expiryDateTime(now);

		share.modify()->setExpiryTime((expiryDateTime + _parameters->maxDuration).toPosixTime());

		if (!_parameters->password.empty())
			share.modify()->setPassword(_parameters->password);

		transaction.commit();

		FS_LOG(UI, INFO) << "[" << share->getDownloadUUID() << "] Share created. Client = " << share->getClientAddr() << ", size = " << share->getFileSize() << ", name = '" << share->getFileName() << "', desc = '" << share->getDesc() << "', expiry " << share->getExpiryTime() << ", download limit = " << share->getMaxHits() << ", password protected = " << share->hasPassword();

		wApp->setInternalPath("/share-created/" + share->getEditUUID(), true);

		// Clear the widget in order to flush the temporary uploaded files
		clear();
	}));

}

} // namespace UserInterface
