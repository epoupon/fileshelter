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

#include "ShareCreate.hpp"

#include <Wt/WAbstractItemModel.h>
#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WDateTime.h>
#include <Wt/WEnvironment.h>
#include <Wt/WFileUpload.h>
#include <Wt/WFormModel.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WLineEdit.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WSpinBox.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTemplateFormView.h>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"
#include "utils/Zip.hpp"

#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCreatePassword.hpp"
#include "ShareCommon.hpp"

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

static std::pair<std::size_t, std::size_t> computeHitsRange()
{
	std::size_t max;
	auto maxValidityHits = Share::getMaxValidatityHits();

	if (maxValidityHits > 0)
		max = maxValidityHits;
	else
		max = std::numeric_limits<int>::max();

	return std::make_pair(1, max);
}

class ShareCreateFormModel : public Wt::WFormModel
{
	public:

		// Associate each field with a unique string literal.
		static const Field DescriptionField;
		static const Field FileField;
		static const Field DurationValidityField;
		static const Field DurationUnitValidityField;
		static const Field HitsValidityLimitField;
		static const Field HitsValidityField;
		static const Field PasswordField;

		ShareCreateFormModel()
		{
			addField(DescriptionField, Wt::WString::tr("msg-optional"));
			addField(DurationValidityField);
			addField(DurationUnitValidityField);
			addField(HitsValidityLimitField);
			addField(HitsValidityField);
			addField(PasswordField, Wt::WString::tr("msg-optional"));

			initializeModels();

			// Duration Validity Unit
			setValidator(DurationUnitValidityField, std::make_unique<Wt::WValidator>(true));  // mandatory

			// Duration Validity
			auto durationValidator = std::make_unique<Wt::WIntValidator>();
			durationValidator->setMandatory(true);
			durationValidator->setBottom(1);
			setValidator(DurationValidityField, std::move(durationValidator));

			updateValidityDuration( Share::getDefaultValidatityDuration() );
			updateDurationValidator();

			// Hits validity
			setValue(HitsValidityLimitField, Share::getDefaultValidatityHits() != 0);

			auto hitsRange = computeHitsRange();
			auto hitsValidator = std::make_unique<Wt::WIntValidator>();
			hitsValidator->setMandatory(true);
			hitsValidator->setRange(hitsRange.first, hitsRange.second);
			setValidator(HitsValidityField, std::move(hitsValidator));

			std::size_t suggestedValidityHits = Share::getDefaultValidatityHits();
			if (suggestedValidityHits > hitsRange.second)
				suggestedValidityHits = hitsRange.second;
			else if (suggestedValidityHits < hitsRange.first)
				suggestedValidityHits = hitsRange.first;

			setValue(HitsValidityField, suggestedValidityHits);
		}

		void updateDurationValidator()
		{
			auto maxValidityDuration = Share::getMaxValidatityDuration();
			auto durationValidator = std::dynamic_pointer_cast<Wt::WIntValidator>(validator(DurationValidityField));

			auto maxValidityDurationHours = std::chrono::duration_cast<std::chrono::hours>(maxValidityDuration).count();

			int maxValue = 1;
			auto unit = valueText(DurationUnitValidityField);
			if (unit == Wt::WString::tr("msg-hours"))
				maxValue = maxValidityDurationHours;
			else if (unit == Wt::WString::tr("msg-days"))
				maxValue = maxValidityDurationHours / 24;
			else if (unit == Wt::WString::tr("msg-weeks"))
				maxValue = maxValidityDurationHours / 24 / 7;
			else if (unit == Wt::WString::tr("msg-months"))
				maxValue = maxValidityDurationHours / 24 / 31;
			else if (unit == Wt::WString::tr("msg-years"))
				maxValue = maxValidityDurationHours / 24 / 365;

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
				return validation(field).state() == Wt::ValidationState::Valid;
			}
			else
			{
				return Wt::WFormModel::validateField(field);
			}

			setValidation(field, Wt::WValidator::Result( error.empty() ? Wt::ValidationState::Valid : Wt::ValidationState::Invalid, error));
			return validation(field).state() == Wt::ValidationState::Valid;
		}

		std::shared_ptr<Wt::WAbstractItemModel> durationValidityModel() { return _durationValidityModel; }

	private:

		void updateValidityDuration(std::chrono::seconds duration)
		{
			auto maxValidityDuration = Share::getMaxValidatityDuration();
			if (duration > maxValidityDuration)
				duration = maxValidityDuration;

			auto durationHours = std::chrono::duration_cast<std::chrono::hours>(duration).count();

			int value;
			Wt::WString unit;
			if (durationHours % 24)
			{
				value = durationHours;
				unit = Wt::WString::tr("msg-hours");
			}
			if ((durationHours / 24 % 365) == 0)
			{
				value = durationHours / 24 / 365;
				unit = Wt::WString::tr("msg-years");
			}
			else if ((durationHours / 24 % 31) == 0)
			{
				value = durationHours / 24 / 31;
				unit = Wt::WString::tr("msg-months");
			}
			else if ((durationHours / 24 % 7) == 0)
			{
				value = durationHours / 24 / 7;
				unit = Wt::WString::tr("msg-weeks");
			}
			else
			{
				value = durationHours / 24;
				unit = Wt::WString::tr("msg-days");
			}

			setValue(DurationValidityField, value);
			setValue(DurationUnitValidityField, unit);
		}

		void initializeModels()
		{
			auto maxDuration = std::chrono::duration_cast<std::chrono::hours>(Share::getMaxValidatityDuration());

			_durationValidityModel = std::make_shared<Wt::WStringListModel>();

			_durationValidityModel->addString( Wt::WString::tr("msg-hours") );
			_durationValidityModel->addString( Wt::WString::tr("msg-days") );

			if (maxDuration >= std::chrono::hours(7*24))
				_durationValidityModel->addString( Wt::WString::tr("msg-weeks") );
			if (maxDuration >= std::chrono::hours(31*24))
				_durationValidityModel->addString( Wt::WString::tr("msg-months") );
			if (maxDuration >= std::chrono::hours(365*24))
				_durationValidityModel->addString( Wt::WString::tr("msg-years") );

		}

		std::shared_ptr<Wt::WStringListModel>	_durationValidityModel;
};

const Wt::WFormModel::Field ShareCreateFormModel::DescriptionField = "desc";
const Wt::WFormModel::Field ShareCreateFormModel::FileField = "file";
const Wt::WFormModel::Field ShareCreateFormModel::DurationValidityField = "duration-validity";
const Wt::WFormModel::Field ShareCreateFormModel::DurationUnitValidityField = "duration-unit-validity";
const Wt::WFormModel::Field ShareCreateFormModel::HitsValidityLimitField = "hits-validity-limit";
const Wt::WFormModel::Field ShareCreateFormModel::HitsValidityField = "hits-validity";
const Wt::WFormModel::Field ShareCreateFormModel::PasswordField = "password";

class ShareCreateFormView : public Wt::WTemplateFormView
{
	public:

	Wt::Signal<ShareParameters>& validated() { return _sigValidated;}

	ShareCreateFormView()
	{
		auto model = std::make_shared<ShareCreateFormModel>();

		setTemplateText(tr("template-form-share-create"));
		addFunction("id", &WTemplate::Functions::id);
		addFunction("block", &WTemplate::Functions::id);

		// Desc
		setFormWidget(ShareCreateFormModel::DescriptionField, std::make_unique<Wt::WLineEdit>());

		if (Share::userCanSetValidatityDuration())
		{
			setCondition("if-validity-duration", true);
			// Duration validity
			setFormWidget(ShareCreateFormModel::DurationValidityField, std::make_unique<Wt::WSpinBox>());

			// Duration validity unit
			auto durationUnitValidity = std::make_unique<Wt::WComboBox>();
			durationUnitValidity->setModel(model->durationValidityModel());

			// each time the unit is changed, make sure to update the limits
			durationUnitValidity->changed().connect([=]
			{
				updateModel(model.get());
				model->updateDurationValidator();

				// Revalidate the fields if necessary
				if (model->isValidated(ShareCreateFormModel::DurationValidityField))
				{
					model->validateField(ShareCreateFormModel::DurationValidityField);
					model->validateField(ShareCreateFormModel::DurationUnitValidityField);
				}

				updateView(model.get());
			});

			setFormWidget(ShareCreateFormModel::DurationUnitValidityField, std::move(durationUnitValidity));
		}

		// Hits validity

		if (Share::userCanSetValidatityHits())
		{
			setCondition("if-validity-hits", true);

			auto hitsRange = computeHitsRange();
			auto hitsValidity = std::make_unique<Wt::WSpinBox>();
			hitsValidity->setRange(hitsRange.first, hitsRange.second);
			setFormWidget(ShareCreateFormModel::HitsValidityField, std::move(hitsValidity));

			// If the maximum number of download is unlimited, add a checkbox to enable a limit
			if (Share::getMaxValidatityHits() == 0)
			{
				setCondition("if-validity-hits-limit", true);
				auto hitsValidityLimit = std::make_unique<Wt::WCheckBox>();
				auto hitsValidityLimitRaw = hitsValidityLimit.get();
				setFormWidget(ShareCreateFormModel::HitsValidityLimitField, std::move(hitsValidityLimit));

				hitsValidityLimit->changed().connect([=] ()
					{
						model->setReadOnly(ShareCreateFormModel::HitsValidityField,
								!(hitsValidityLimitRaw->checkState() == Wt::CheckState::Checked));
						updateModel(model.get());
						updateViewField(model.get(), ShareCreateFormModel::HitsValidityField);
					});
			}
		}

		// Password
		auto password = std::make_unique<Wt::WLineEdit>();
		password->setEchoMode(Wt::EchoMode::Password);
		setFormWidget(ShareCreateFormModel::PasswordField, std::move(password));

		// Buttons
		Wt::WPushButton *uploadBtn = bindNew<Wt::WPushButton>("create-btn", tr("msg-upload"));
		uploadBtn->clicked().connect([=]
		{
			updateModel(model.get());

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

				if (Wt::asNumber(model->value(ShareCreateFormModel::HitsValidityLimitField)))
					params.maxHits = Wt::asNumber(model->value(ShareCreateFormModel::HitsValidityField));
				else
					params.maxHits = 0;

				params.password = model->valueText(ShareCreateFormModel::PasswordField);

				_sigValidated.emit(params);
			}

			updateView(model.get());
		});

		updateView(model.get());
	}

	private:
		Wt::Signal<ShareParameters> _sigValidated;

};

ShareCreate::ShareCreate()
: _parameters(std::make_shared<ShareParameters>())
{
	wApp->internalPathChanged().connect([=]
	{
		refresh();
	});

	refresh();
}

void
ShareCreate::displayError(Wt::WString error)
{
	clear();

	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-not-created"));

	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
	t->bindString("error", error);
}

void
ShareCreate::refresh()
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

	auto view = addNew<ShareCreatePassword>();
	view->success().connect([=]
	{
		displayCreate();
	});
}

void
ShareCreate::displayCreate()
{
	clear();

	Wt::WTemplate *t = addNew<Wt::WTemplate>(tr("template-share-create"));
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	t->bindNew<Wt::WText>("msg-max-size", Wt::WString::tr("msg-max-file-size").arg( sizeToString(Share::getMaxFileSize() * 1024*1024) ));

	Wt::WFileUpload *upload = t->bindNew<Wt::WFileUpload>("file");
	upload->setFileTextSize(80);
	upload->setProgressBar(std::make_unique<Wt::WProgressBar>());
	upload->setMultiple(true);

	ShareCreateFormView* form = t->bindNew<ShareCreateFormView>("form");

	upload->fileTooLarge().connect([=] ()
	{
		FS_LOG(UI, WARNING) << "File too large!";
		displayError(Wt::WString::tr("msg-share-create-too-large"));
	});

	form->validated().connect([=] (ShareParameters params)
	{
		FS_LOG(UI, DEBUG) << "Starting upload...";

		// Start the upload
		form->hide();
		upload->upload();
		*_parameters = params;
	});

	upload->uploaded().connect([=] ()
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

		std::filesystem::path sharePath;
		Wt::WString fileName;

		if (uploadedFiles.size() == 1)
		{
			sharePath = std::filesystem::path(uploadedFiles.front().spoolFileName());
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
				ZipFileWriter zip {sharePath};

				std::vector<std::filesystem::path> files;
				for (auto uploadedFile : uploadedFiles)
				{
					zip.add(uploadedFile.clientFileName(),
						std::filesystem::path(uploadedFile.spoolFileName()));
				}
			}
			catch (std::exception& e)
			{
				FS_LOG(UI, ERROR) << "Cannot create zip file: " << e.what();
				displayError(Wt::WString::tr("msg-internal-error"));
				return;
			}
		}

		Wt::Dbo::Transaction transaction(FsApp->getDboSession());

		Database::Share::pointer share = Database::Share::create(FsApp->getDboSession(), sharePath);
		if (!share)
		{
			displayError(Wt::WString::tr("msg-internal-error"));
			return;
		}

		auto now = Wt::WLocalDateTime::currentDateTime().toUTC();

		share.modify()->setDesc(_parameters->description.toUTF8());
		share.modify()->setFileName(fileName.toUTF8());
		share.modify()->setMaxHits(_parameters->maxHits);
		share.modify()->setCreationTime(now);
		share.modify()->setClientAddr(wApp->environment().clientAddress());

		// calculate the expiry date from the duration
		share.modify()->setExpiryTime(now + _parameters->maxDuration);

		if (!_parameters->password.empty())
			share.modify()->setPassword(_parameters->password);

		transaction.commit();

		FS_LOG(UI, INFO) << "[" << share->getDownloadUUID() << "] Share created. Client = " << share->getClientAddr() << ", size = " << share->getFileSize() << ", name = '" << share->getFileName() << "', desc = '" << share->getDesc() << "', expiry " << share->getExpiryTime().toString() << ", download limit = " << share->getMaxHits() << ", password protected = " << share->hasPassword();

		wApp->setInternalPath("/share-created/" + share->getEditUUID(), true);

		// Clear the widget in order to flush the temporary uploaded files
		clear();
	});

}

} // namespace UserInterface
