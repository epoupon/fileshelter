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
	const ShareCreateFormModel::Field ShareCreateFormModel::ValidityPeriodField {"validity-period"};
	const ShareCreateFormModel::Field ShareCreateFormModel::ValidityPeriodUnitField {"validity-period-unit"};
	const ShareCreateFormModel::Field ShareCreateFormModel::PasswordField {"password"};

	ShareCreateFormModel::ShareCreateFormModel()
	{
		addField(DescriptionField, Wt::WString::tr("msg-optional"));
		addField(ValidityPeriodField);
		addField(ValidityPeriodUnitField);
		addField(PasswordField, Wt::WString::tr("msg-optional"));

		initializeModels();

		// Period Validity Unit
		setValidator(ValidityPeriodUnitField, std::make_unique<Wt::WValidator>(true));  // mandatory

		// Period Validity
		auto durationValidator = std::make_unique<Wt::WIntValidator>();
		durationValidator->setMandatory(true);
		durationValidator->setBottom(1);
		setValidator(ValidityPeriodField, std::move(durationValidator));

		updateValidityPeriod( Service<Share::IShareManager>::get()->getDefaultValidityPeriod() );
		updatePeriodValidator();
	}

	void
	ShareCreateFormModel::updatePeriodValidator()
	{
		const auto maxValidityPeriod {Service<Share::IShareManager>::get()->getMaxValidityPeriod()};
		auto durationValidator {std::dynamic_pointer_cast<Wt::WIntValidator>(validator(ValidityPeriodField))};

		const auto maxValidityPeriodHours {std::chrono::duration_cast<std::chrono::hours>(maxValidityPeriod).count()};

		int maxValue {1};
		auto unit {valueText(ValidityPeriodUnitField)};
		if (unit == Wt::WString::tr("msg-hours"))
			maxValue = maxValidityPeriodHours;
		else if (unit == Wt::WString::tr("msg-days"))
			maxValue = maxValidityPeriodHours / 24;
		else if (unit == Wt::WString::tr("msg-weeks"))
			maxValue = maxValidityPeriodHours / 24 / 7;
		else if (unit == Wt::WString::tr("msg-months"))
			maxValue = maxValidityPeriodHours / 24 / 31;
		else if (unit == Wt::WString::tr("msg-years"))
			maxValue = maxValidityPeriodHours / 24 / 365;

		durationValidator->setTop(maxValue);

		// If the current value is too high, change it to the max
		if (Wt::asNumber(value(ValidityPeriodField)) > maxValue)
			setValue(ValidityPeriodField, maxValue);
	}

	void
	ShareCreateFormModel::validateValidatityFields()
	{
		if (isValidated(ValidityPeriodField))
		{
			validateField(ValidityPeriodField);
			validateField(ValidityPeriodUnitField);
		}
	}

	bool
	ShareCreateFormModel::validateField(Field field)
	{
		Wt::WString error; // empty means validated

		if (field == ValidityPeriodUnitField)
		{
			// Since they are grouped together,
			// make it share the same state as ValidityPeriodField
			validateField(ValidityPeriodField);
			setValidation(field, validation(ValidityPeriodField));
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
	ShareCreateFormModel::updateValidityPeriod(std::chrono::seconds duration)
	{
		auto maxValidityPeriod = Service<Share::IShareManager>::get()->getMaxValidityPeriod();
		if (duration > maxValidityPeriod)
			duration = maxValidityPeriod;

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

		setValue(ValidityPeriodField, value);
		setValue(ValidityPeriodUnitField, unit);
	}

	void
	ShareCreateFormModel::initializeModels()
	{
		const auto maxPeriod {std::chrono::duration_cast<std::chrono::hours>(Service<Share::IShareManager>::get()->getMaxValidityPeriod())};

		_validityPeriodModel = std::make_shared<Wt::WStringListModel>();

		_validityPeriodModel->addString( Wt::WString::tr("msg-hours") );
		_validityPeriodModel->addString( Wt::WString::tr("msg-days") );

		if (maxPeriod >= std::chrono::hours {7*24})
			_validityPeriodModel->addString( Wt::WString::tr("msg-weeks") );
		if (maxPeriod >= std::chrono::hours {31*24})
			_validityPeriodModel->addString( Wt::WString::tr("msg-months") );
		if (maxPeriod >= std::chrono::hours {365*24})
			_validityPeriodModel->addString( Wt::WString::tr("msg-years") );
	}

	std::chrono::seconds
	ShareCreateFormModel::getValidityPeriod() const
	{
		// TODO
		return std::chrono::hours {48};
	}

}
