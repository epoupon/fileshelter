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

#include <numeric>
#include <unordered_set>

#include <Wt/WAbstractItemModel.h>
#include <Wt/WComboBox.h>
#include <Wt/WDateTime.h>
#include <Wt/WEnvironment.h>
#include <Wt/WFileDropWidget.h>
#include <Wt/WFormModel.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WSpinBox.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTimer.h>
#include <Wt/WTemplateFormView.h>
#include "utils/Config.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"

#include "database/Share.hpp"
#include "share/ShareUtils.hpp"

#include "ProgressBar.hpp"
#include "FileShelterApplication.hpp"
#include "PasswordUtils.hpp"
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

struct ShareParameters
{
	Wt::WString	description;
	Duration	maxDuration;
	Wt::WString	password;
	std::vector<const Wt::Http::UploadedFile*> uploadedFiles;
};

class ShareCreateFormModel : public Wt::WFormModel
{
	public:

		static inline const Field DescriptionField {"desc"};
		static inline const Field DurationValidityField {"duration-validity"};
		static inline const Field DurationUnitValidityField {"duration-unit-validity"};
		static inline const Field PasswordField {"password"};

		ShareCreateFormModel()
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

			updateValidityDuration( ShareUtils::getDefaultValidatityDuration() );
			updateDurationValidator();
		}

		void updateDurationValidator()
		{
			auto maxValidityDuration = ShareUtils::getMaxValidatityDuration();
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
			auto maxValidityDuration = ShareUtils::getMaxValidatityDuration();
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
			auto maxDuration = std::chrono::duration_cast<std::chrono::hours>(ShareUtils::getMaxValidatityDuration());

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

class ShareCreateFormView : public Wt::WTemplateFormView
{
	public:

	Wt::Signal<>& validated() { return _sigValidated;}
	Wt::Signal<unsigned>& progressUpdate() { return _sigProgressUpdate;}
	Wt::Signal<ShareParameters>& complete() { return _sigComplete;}

	unsigned getProgress() const
	{
		return (_totalReceivedSize + _currentReceivedSize ) * 100 / _totalSize;
	}

	ShareCreateFormView()
	: _model {std::make_shared<ShareCreateFormModel>()}
	{
		setTemplateText(tr("template-share-create-form"));

		bindString("max-size", Wt::WString::tr("msg-max-share-size").arg( sizeToString(ShareUtils::getMaxShareSize()) ));

		_drop = bindNew<Wt::WFileDropWidget>("file-drop");

		auto* filesContainer {bindNew<Wt::WContainerWidget>("files")};

		_drop->drop().connect([=](const std::vector<Wt::WFileDropWidget::File*>& files)
		{
			for (Wt::WFileDropWidget::File* file : files)
			{
				auto* fileEntry {filesContainer->addNew<Wt::WTemplate>(tr("template-share-create-form-file"))};
				fileEntry->bindString("name", file->clientFileName(), Wt::TextFormat::Plain);
				fileEntry->bindString("size", sizeToString(file->size()), Wt::TextFormat::Plain);
				auto* delBtn {fileEntry->bindNew<Wt::WText>("del-btn", tr("template-share-create-del-btn"))};

				delBtn->clicked().connect([=]
				{
					_drop->cancelUpload(file);
					_drop->remove(file); 			// may not work if current handled file is before this one
					_deletedFiles.emplace(file);	// hence this tracking
					filesContainer->removeWidget(fileEntry);
					checkFiles();
				});
			}

			checkFiles();
		});

		_drop->tooLarge().connect([=](const Wt::WFileDropWidget::File* file, std::uint64_t size)
		{
			FS_LOG(UI, ERROR) << "File '" << file->clientFileName() << "' is too large: " << size;
		});

		// Desc
		setFormWidget(ShareCreateFormModel::DescriptionField, std::make_unique<Wt::WLineEdit>());

		if (ShareUtils::canValidatityDurationBeSet())
		{
			setCondition("if-validity-duration", true);
			// Duration validity
			setFormWidget(ShareCreateFormModel::DurationValidityField, std::make_unique<Wt::WSpinBox>());

			// Duration validity unit
			auto durationUnitValidity = std::make_unique<Wt::WComboBox>();
			durationUnitValidity->setModel(_model->durationValidityModel());

			// each time the unit is changed, make sure to update the limits
			durationUnitValidity->changed().connect([=]
			{
				updateModel(_model.get());
				_model->updateDurationValidator();

				// Revalidate the fields if necessary
				if (_model->isValidated(ShareCreateFormModel::DurationValidityField))
				{
					_model->validateField(ShareCreateFormModel::DurationValidityField);
					_model->validateField(ShareCreateFormModel::DurationUnitValidityField);
				}

				updateView(_model.get());
			});

			setFormWidget(ShareCreateFormModel::DurationUnitValidityField, std::move(durationUnitValidity));
		}

		// Password
		auto password {std::make_unique<Wt::WLineEdit>()};
		password->setEchoMode(Wt::EchoMode::Password);
		setFormWidget(ShareCreateFormModel::PasswordField, std::move(password));

		// Buttons
		_createBtn = bindNew<Wt::WPushButton>("create-btn", tr("msg-create"));
		_createBtn->disable();
		_createBtn->clicked().connect([=]
		{
			updateModel(_model.get());

			if (_model->validate())
			{
				_createBtn->disable();
				listenFileEvents();
				_sigValidated.emit();
			}

			updateView(_model.get());
		});

		updateView(_model.get());
	}

	~ShareCreateFormView()
	{
/*		for (Wt::WFileDropWidget::File* file : _drop->uploads())
		{
			_drop->cancelUpload(file);
			_drop->remove(file);
			if (file->uploadFinished())
			{
				file->uploadedFile().stealSpoolFile();
				std::filesystem::remove(file->uploadedFile().spoolFileName());
			}
		}*/
	}

	private:

		void listenFileEvents()
		{
			for (Wt::WFileDropWidget::File* file : _drop->uploads())
			{
				if (isFileDeleted(*file))
					continue;

				_totalSize += file->size();
				if (file->uploadFinished())
				{
					_totalReceivedSize += file->size();
				}
				else
				{
					file->dataReceived().connect([this](std::uint64_t received, std::uint64_t total)
					{
						_currentReceivedSize = received;
						_sigProgressUpdate.emit(getProgress());
					});

					file->uploaded().connect([=]
					{
						_currentReceivedSize = 0;
						_totalReceivedSize += file->size();

						if (_totalReceivedSize == _totalSize)
						{
							_progressTimer.stop();
							emitDone();
						}
					});
				}
			}

			_progressTimer.setInterval(std::chrono::milliseconds {500});
			_progressTimer.setSingleShot(false);
			_progressTimer.timeout().connect([this]
			{
				_sigProgressUpdate.emit(getProgress());
			});
			_progressTimer.start();
		}

		void emitDone()
		{
			ShareParameters params;

			params.description = _model->valueText(ShareCreateFormModel::DescriptionField);
			params.maxDuration.value = Wt::asNumber(_model->value(ShareCreateFormModel::DurationValidityField));

			auto unit = _model->valueText(ShareCreateFormModel::DurationUnitValidityField);
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

			params.password = _model->valueText(ShareCreateFormModel::PasswordField);

			visitUploadedFiles([&](const Wt::WFileDropWidget::File& file)
			{
				params.uploadedFiles.emplace_back(&file.uploadedFile());
			});

			_sigComplete.emit(params);
		}

		void visitUploadedFiles(std::function<void(Wt::WFileDropWidget::File& file)> visitor)
		{
			for (auto* file : _drop->uploads())
			{
				if (!isFileDeleted(*file))
					visitor(*file);
			}
		}

		std::uint64_t getTotalFileSize() const
		{
			const auto& files {_drop->uploads()};
			return std::accumulate(std::cbegin(files), std::cend(files), std::uint64_t {},
					[this](std::uint64_t sum, const Wt::WFileDropWidget::File* file)
					{
						return sum + (isFileDeleted(*file) ? 0 : file->size());
					});
		}

		bool shareSizeOverflow() const
		{
			return getTotalFileSize() > ShareUtils::getMaxShareSize();
		};

		bool hasDuplicateNames() const
		{
			std::unordered_set<std::string_view> names;

			for (const Wt::WFileDropWidget::File* file : _drop->uploads())
			{
				if (!isFileDeleted(*file))
				{
					auto [it, inserted] {names.emplace(file->clientFileName())};
					if (!inserted)
						return true;
				}
			}

			return false;
		};

		std::size_t countNbFilesToUpload() const
		{
			const auto& files {_drop->uploads()};
			return std::count_if(std::cbegin(files), std::cend(files),
					[this](const Wt::WFileDropWidget::File* file)
					{
						return (!isFileDeleted(*file));
					});
		}

		void checkFiles()
		{
			const bool overflow {shareSizeOverflow()};
			setCondition("if-max-share-size", overflow);

			const bool duplicateNames {hasDuplicateNames()};
			setCondition("if-has-duplicate-names", duplicateNames);

			const std::size_t nbFiles {countNbFilesToUpload()};

			if (overflow || duplicateNames || nbFiles == 0)
				_createBtn->disable();
			else
				_createBtn->enable();
		};

		bool uploadComplete() const
		{
			const auto& files {_drop->uploads()};
			return std::all_of(std::cbegin(files), std::cend(files), [this](const Wt::WFileDropWidget::File* file)
					{
						return isFileDeleted(*file) || file->uploadFinished();
					});
		}

		bool isFileDeleted(const Wt::WFileDropWidget::File& file) const
		{
			return _deletedFiles.find(&file) != std::cend(_deletedFiles);
		}

		Wt::Signal<>				_sigValidated;
		Wt::Signal<unsigned>		_sigProgressUpdate;
		Wt::Signal<ShareParameters>	_sigComplete;

		Wt::WTimer					_progressTimer;

		std::shared_ptr<ShareCreateFormModel> _model;
		Wt::WPushButton*			_createBtn {};

		std::unordered_set<const Wt::WFileDropWidget::File*> _deletedFiles;
		Wt::WFileDropWidget*		_drop {};
		std::uint64_t 				_totalReceivedSize {};
		std::uint64_t 				_currentReceivedSize {};
		std::uint64_t 				_totalSize {};
};

class ShareCreateProgress : public Wt::WTemplate
{
	public:
		ShareCreateProgress() : Wt::WTemplate {tr("template-share-create-progress")}
		{
			addFunction("tr", &Wt::WTemplate::Functions::tr);
			_progress = bindNew<ProgressBar>("progress");
		}

		void handleProgressUpdate(unsigned progress)
		{
			_progress->setValue(progress);
		}

	private:
		ProgressBar*	_progress {};
};


ShareCreate::ShareCreate()
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

	if (isUploadPassordRequired())
		displayPassword();
	else
		displayCreate();
}

void
ShareCreate::displayPassword()
{
	clear();

	auto* view {addNew<ShareCreatePassword>()};
	view->success().connect([=]
	{
		displayCreate();
	});
}

void
ShareCreate::displayCreate()
{
	clear();

	enum CreateStack
	{
		Form = 0,
		Progress = 1,
	};

	Wt::WStackedWidget* stack {addNew<Wt::WStackedWidget>()};

	auto* form {stack->addNew<ShareCreateFormView>()};
	auto* progress {stack->addNew<ShareCreateProgress>()};

	form->progressUpdate().connect(progress, [=](unsigned progressPerCent) { progress->handleProgressUpdate(progressPerCent); });

	form->validated().connect([=] ()
	{
		stack->setCurrentIndex(CreateStack::Progress);
	});

	form->complete().connect([this](const ShareParameters& parameters)
	{
		FS_LOG(UI, DEBUG) << "Upload complete!";
		UUID editUUID {createShare(parameters)};

		FS_LOG(UI, DEBUG) << "Redirecting...";
		wApp->setInternalPath("/share-created/" + editUUID.getAsString(), true);

		// Clear the widget in order to flush the temporary uploaded files
		FS_LOG(UI, DEBUG) << "Clearing...";
		clear();
		FS_LOG(UI, DEBUG) << "Clearing done";
	});
}

UUID
ShareCreate::createShare(const ShareParameters& parameters)
{
	std::optional<Wt::Auth::PasswordHash> passwordHash;
	if (!parameters.password.empty())
		passwordHash = ShareUtils::computePasswordHash(parameters.password);

	const UUID editUUID {UUID::generate()};
	{
		Wt::Dbo::Transaction transaction {FsApp->getDboSession()};

		FS_LOG(UI, DEBUG) << "Creating share...";
		Database::Share::pointer share {Database::Share::create(FsApp->getDboSession())};
		if (!share)
			throw FsException {Wt::WString::tr("msg-internal-error").toUTF8()};

		const auto now {Wt::WLocalDateTime::currentDateTime().toUTC()};

		share.modify()->setDesc(parameters.description.toUTF8());
		share.modify()->setDownloadUUID(UUID::generate());
		share.modify()->setEditUUID(editUUID);
		share.modify()->setCreationTime(now);
		share.modify()->setClientAddr(wApp->environment().clientAddress());

		// calculate the expiry date from the duration
		share.modify()->setExpiryTime(now + parameters.maxDuration);

		if (passwordHash)
			share.modify()->setPasswordHash(*passwordHash);

		for (auto* uploadedFile : parameters.uploadedFiles)
		{
			FS_LOG(UI, DEBUG) << "PROCESSING FILE '" << uploadedFile->clientFileName() << "'";

			uploadedFile->stealSpoolFile();
			const std::filesystem::path clientFileName {uploadedFile->clientFileName()};
			ShareUtils::addFileToShare(FsApp->getDboSession(), share.id(), uploadedFile->spoolFileName(), clientFileName.filename().string());
		}

		share.modify()->setState(Database::Share::State::Ready);

		FS_LOG(UI, INFO) << "[" << share->getDownloadUUID().getAsString() << "] Share created. Client = " << std::string {share->getClientAddr()} << ", size = " << share->getShareSize() << "', desc = '" << std::string {share->getDesc()} << "', expiry " << share->getExpiryTime().toString() << ", password protected = " << share->hasPassword() << ", download URL = '" << getDownloadURL(share) << "'";

		return editUUID;
	}

}

} // namespace UserInterface
