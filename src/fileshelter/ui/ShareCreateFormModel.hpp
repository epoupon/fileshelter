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

#pragma once

#include <chrono>
#include <memory>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WFormModel.h>
#include <Wt/WStringListModel.h>

namespace UserInterface
{
	class ShareCreateFormModel : public Wt::WFormModel
	{
		public:
			ShareCreateFormModel();

			std::shared_ptr<Wt::WAbstractItemModel> validityPeriodModel() { return _validityPeriodModel; }
			void updatePeriodValidator();

			void validateValidatityFields();

			std::chrono::seconds getValidityPeriod() const;

			static const Field DescriptionField;
			static const Field PasswordField;
			static const Field ValidityPeriodField;
			static const Field ValidityPeriodUnitField;

		private:
			void updateValidityPeriod(std::chrono::seconds duration);
			bool validateField(Field field) override;
			void initializeModels();

			std::shared_ptr<Wt::WStringListModel>	_validityPeriodModel;
	};
}
