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

#include "ShareCreateFormView.hpp"

#include <numeric>

#include <Wt/WComboBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>

#include "share/ShareUtils.hpp"
#include "utils/Logger.hpp"
#include "ShareCommon.hpp"

namespace UserInterface
{
	unsigned
	ShareCreateFormView::getProgress() const
	{
		return (_totalReceivedSize + _currentReceivedSize ) * 100 / _totalSize;
	}

	ShareCreateFormView::ShareCreateFormView()
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
					deleteFile(*file);
					filesContainer->removeWidget(fileEntry);
					checkFiles();
				});

				addFile(*file);
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
				_model->validateValidatityFields();

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

			_validated = _model->validate();
			updateView(_model.get());

			if (_validated)
			{
				_createBtn->disable();
				_sigValidated.emit();

				if (isUploadComplete())
					emitDone();
			}
		});

		updateView(_model.get());
	}

	void
	ShareCreateFormView::deleteFile(Wt::WFileDropWidget::File& file)
	{
		_drop->cancelUpload(&file);
		_drop->remove(&file); 			// may not work if current handled file is before this one
		_deletedFiles.emplace(&file);	// hence this tracking

		_totalSize -= file.size();

		if (file.uploadFinished())
			_totalReceivedSize -= file.size();

		_sigProgressUpdate.emit(getProgress());
	}

	void
	ShareCreateFormView::addFile(Wt::WFileDropWidget::File& file)
	{
		_totalSize += file.size();
		if (file.uploadFinished())
		{
			_totalReceivedSize += file.size();
		}
		else
		{
			file.dataReceived().connect([this, &file](std::uint64_t received, std::uint64_t total)
			{
				FS_LOG(UI, DEBUG) << "File '" << file.clientFileName() << "': data received: " << received << " / " << total;

				_currentReceivedSize = received;
				_sigProgressUpdate.emit(getProgress());
			});

			file.uploaded().connect([this, &file]
			{
				onFileUploaded(file);
			});
		}
	}

	void
	ShareCreateFormView::onFileUploaded(const Wt::WFileDropWidget::File& file)
	{
		FS_LOG(UI, DEBUG) << "File '" << file.clientFileName() << "' is uploaded!";

		_currentReceivedSize = 0;
		_totalReceivedSize += file.size();

		FS_LOG(UI, DEBUG) << "_totalReceivedSize = " << _totalReceivedSize;
		FS_LOG(UI, DEBUG) << "_totalSize = " << _totalSize;

		if (_validated && _totalReceivedSize == _totalSize)
		{
			emitDone();
		}
	}

	void
	ShareCreateFormView::emitDone()
	{
		ShareCreateParameters params;

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

		FS_LOG(UI, DEBUG) << "max duration = " << params.maxDuration.value;
		FS_LOG(UI, DEBUG) << "max duration unit = " << static_cast<int>(params.maxDuration.unit);

		params.password = _model->valueText(ShareCreateFormModel::PasswordField);

		visitUploadedFiles([&](const Wt::WFileDropWidget::File& file)
		{
			params.uploadedFiles.emplace_back(&file.uploadedFile());
		});

		_sigComplete.emit(params);
	}

	void
	ShareCreateFormView::visitUploadedFiles(std::function<void(Wt::WFileDropWidget::File& file)> visitor)
	{
		for (auto* file : _drop->uploads())
		{
			if (!isFileDeleted(*file))
				visitor(*file);
		}
	}

	std::uint64_t
	ShareCreateFormView::getTotalFileSize() const
	{
		const auto& files {_drop->uploads()};
		return std::accumulate(std::cbegin(files), std::cend(files), std::uint64_t {},
			[this](std::uint64_t sum, const Wt::WFileDropWidget::File* file)
			{
				return sum + (isFileDeleted(*file) ? 0 : file->size());
			});
	}

	bool
	ShareCreateFormView::isShareSizeOverflow() const
	{
		return getTotalFileSize() > ShareUtils::getMaxShareSize();
	}

	bool
	ShareCreateFormView::hasDuplicateNames() const
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
	}

	std::size_t
	ShareCreateFormView::countNbFilesToUpload() const
	{
		const auto& files {_drop->uploads()};
		return std::count_if(std::cbegin(files), std::cend(files),
			[this](const Wt::WFileDropWidget::File* file)
			{
				return (!isFileDeleted(*file));
			});
	}

	void
	ShareCreateFormView::checkFiles()
	{
		const bool overflow {isShareSizeOverflow()};
		setCondition("if-max-share-size", overflow);

		const bool duplicateNames {hasDuplicateNames()};
		setCondition("if-has-duplicate-names", duplicateNames);

		const std::size_t nbFiles {countNbFilesToUpload()};

		if (overflow || duplicateNames || nbFiles == 0)
			_createBtn->disable();
		else
			_createBtn->enable();
	}

	bool
	ShareCreateFormView::isUploadComplete() const
	{
		const auto& files {_drop->uploads()};
		return std::all_of(std::cbegin(files), std::cend(files), [this](const Wt::WFileDropWidget::File* file)
			{
				return isFileDeleted(*file) || file->uploadFinished();
			});
	}

	bool
	ShareCreateFormView::isFileDeleted(const Wt::WFileDropWidget::File& file) const
	{
		return _deletedFiles.find(&file) != std::cend(_deletedFiles);
	}

} // namespace UiserInterface

