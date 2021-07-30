/*
 * Copyright (C) 2020 Emeric Poupon
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

#include "ShareCreateFormModel.hpp"

#include <Wt/WIntValidator.h>

#include "share/IShareManager.hpp"
#include "utils/Service.hpp"

namespace UserInterface
{

	const ShareCreateFormModel::Field ShareCreateFormModel::DescriptionField {"desc"};
	const ShareCreateFormModel::Field ShareCreateFormModel::DurationValidityField {"duration-validity"};
	const ShareCreateFormModel::Field ShareCreateFormModel::DurationUnitValidityField {"duration-unit-validity"};
	const ShareCreateFormModel::Field ShareCreateFormModel::PasswordField {"password"};

	ShareCreateFormModel::ShareCreateFormModel()
	{
		addField(DescriptionField, Wt::WString::tr("msg-optional"));
		addField(DurationValidityField);
		addField(DurationUnitValidityField);
		addField(PasswordField, Wt::WString::tr("msg-optional"));

		initializeModels();

		// Duration Validity Unit
		setValidator(DurationUnitValidityField, std::make_unique<Wt::WValidator>(true));  // mandatory

		// Duration Validity
		auto durationValidator = std::make_unique<Wt::WIntValidator>();
		durationValidator->setMandatory(true);
		durationValidator->setBottom(1);
		setValidator(DurationValidityField, std::move(durationValidator));

		updateValidityDuration( Service<Share::IShareManager>::get()->getDefaultValidatityDuration() );
		updateDurationValidator();
	}

	void
	ShareCreateFormModel::updateDurationValidator()
	{
		const auto maxValidityDuration {Service<Share::IShareManager>::get()->getMaxValidatityDuration()};
		auto durationValidator {std::dynamic_pointer_cast<Wt::WIntValidator>(validator(DurationValidityField))};

		const auto maxValidityDurationHours {std::chrono::duration_cast<std::chrono::hours>(maxValidityDuration).count()};

		int maxValue {1};
		auto unit {valueText(DurationUnitValidityField)};
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

	void
	ShareCreateFormModel::validateValidatityFields()
	{
		if (isValidated(DurationValidityField))
		{
			validateField(DurationValidityField);
			validateField(DurationUnitValidityField);
		}
	}

	bool
	ShareCreateFormModel::validateField(Field field)
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

	void
	ShareCreateFormModel::updateValidityDuration(std::chrono::seconds duration)
	{
		auto maxValidityDuration = Service<Share::IShareManager>::get()->getMaxValidatityDuration();
		if (duration > maxValidityDuration)
			duration = maxValidityDuration;

		const auto durationHours {std::chrono::duration_cast<std::chrono::hours>(duration).count()};

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

	void
	ShareCreateFormModel::initializeModels()
	{
		const auto maxDuration {std::chrono::duration_cast<std::chrono::hours>(Service<Share::IShareManager>::get()->getMaxValidatityDuration())};

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

	std::chrono::seconds
	ShareCreateFormModel::getDurationValidity() const
	{
		return std::chrono::hours {48};
	}

}
