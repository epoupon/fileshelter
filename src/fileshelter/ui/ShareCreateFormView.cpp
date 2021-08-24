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

#include <Wt/WApplication.h>
#include <Wt/WComboBox.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>

#include "share/CreateParameters.hpp"
#include "share/IShareManager.hpp"
#include "share/Types.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"
#include "ShareUtils.hpp"

namespace UserInterface
{
	unsigned
	ShareCreateFormView::getProgress() const
	{
		if (_totalSize == 0)
			return 0;

		return (_totalReceivedSize + _currentReceivedSize ) * 100 / _totalSize;
	}

	ShareCreateFormView::ShareCreateFormView()
	: _model {std::make_shared<ShareCreateFormModel>()}
	{
		setTemplateText(tr("template-share-create-form"));

		_drop = bindNew<Wt::WFileDropWidget>("file-drop");

		auto* filesContainer {bindNew<Wt::WContainerWidget>("files")};

		_drop->drop().connect([=](const std::vector<Wt::WFileDropWidget::File*>& files)
		{
			for (Wt::WFileDropWidget::File* file : files)
			{
				auto* fileEntry {filesContainer->addNew<Wt::WTemplate>(tr("template-share-create-form-file"))};
				fileEntry->bindString("name", file->clientFileName(), Wt::TextFormat::Plain);
				fileEntry->bindString("size", ShareUtils::fileSizeToString(file->size()), Wt::TextFormat::Plain);

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

		_shareSize = bindNew<Wt::WText>("share-size");

		_error = bindNew<Wt::WTemplate>("error", Wt::WString::tr("template-share-create-error"));

		// Desc
		setFormWidget(ShareCreateFormModel::DescriptionField, std::make_unique<Wt::WLineEdit>());

		if (Service<Share::IShareManager>::get()->canValidityPeriodBeSet())
		{
			setCondition("if-validity-period", true);
			// Validity period
			setFormWidget(ShareCreateFormModel::ValidityPeriodField, std::make_unique<Wt::WSpinBox>());

			// Validity period unit
			auto validityPeriodUnit = std::make_unique<Wt::WComboBox>();
			validityPeriodUnit->setModel(_model->validityPeriodModel());

			// each time the unit is changed, make sure to update the limits
			validityPeriodUnit->changed().connect([=]
			{
				updateModel(_model.get());
				_model->updatePeriodValidator();
				_model->validateValidatityFields();

				updateView(_model.get());
			});

			setFormWidget(ShareCreateFormModel::ValidityPeriodUnitField, std::move(validityPeriodUnit));
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
		checkFiles();
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

		FS_LOG(UI, DEBUG) << "_validated = " << _validated;

		if (_validated && _totalReceivedSize == _totalSize)
		{
			emitDone();
		}
	}

	void
	ShareCreateFormView::emitDone()
	{
		using namespace Share;

		ShareCreateParameters params;

		params.description = _model->valueText(ShareCreateFormModel::DescriptionField).toUTF8();
		params.validityPeriod = _model->getValidityPeriod();
		params.password = _model->valueText(ShareCreateFormModel::PasswordField).toUTF8();
		params.creatorAddress = wApp->environment().clientAddress();

		std::vector<Share::FileCreateParameters> filesParameters;

		visitUploadedFiles([&](const Wt::WFileDropWidget::File& file)
		{
			Share::FileCreateParameters fileParameters;

			fileParameters.path = file.uploadedFile().spoolFileName();
			fileParameters.name = file.uploadedFile().clientFileName();

			filesParameters.emplace_back(std::move(fileParameters));
		});

		// two steps to minimize possibilities of throw (although we delete orphan uploaded files in the cleaner)
		visitUploadedFiles([&](const Wt::WFileDropWidget::File& file)
		{
			file.uploadedFile().stealSpoolFile();
		});

		_sigComplete.emit(params, filesParameters);
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
		return getTotalFileSize() > Service<Share::IShareManager>::get()->getMaxShareSize();
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
		_shareSize->setText(ShareUtils::fileSizeToString(getTotalFileSize()));

		Wt::WString errorMsg;

		const bool overflow {isShareSizeOverflow()};
		if (overflow)
			errorMsg = Wt::WString::tr("msg-max-share-size").arg( ShareUtils::fileSizeToString(Service<Share::IShareManager>::get()->getMaxShareSize()) );

		const bool duplicateNames {hasDuplicateNames()};
		if (duplicateNames)
			errorMsg = Wt::WString::tr("msg-duplicate-file-names");

		if (errorMsg.empty())
			hideError();
		else
			showError(errorMsg);

		if (overflow || duplicateNames || countNbFilesToUpload() == 0)
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

	void
	ShareCreateFormView::showError(const Wt::WString& error)
	{
		_error->setHidden(false);
		_error->bindString("error", error);
	}

	void
	ShareCreateFormView::hideError()
	{
		_error->setHidden(true);
	}
} // namespace UiserInterface

