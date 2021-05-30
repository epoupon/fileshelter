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

#include <unordered_set>

#include <Wt/WSignal.h>
#include <Wt/WFileDropWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplateFormView.h>

#include "ShareCreateFormModel.hpp"
#include "ShareCreateParameters.hpp"

namespace UserInterface
{
	class ShareCreateFormView : public Wt::WTemplateFormView
	{
		public:
			ShareCreateFormView();

			unsigned getProgress() const;

			Wt::Signal<>& validated() { return _sigValidated;}
			Wt::Signal<unsigned>& progressUpdate() { return _sigProgressUpdate;}

			Wt::Signal<ShareCreateParameters>& complete() { return _sigComplete;}

		private:

			void deleteFile(Wt::WFileDropWidget::File& file);
			void addFile(Wt::WFileDropWidget::File& file);
			void emitDone();
			void visitUploadedFiles(std::function<void(Wt::WFileDropWidget::File& file)> visitor);
			std::uint64_t getTotalFileSize() const;
			bool isShareSizeOverflow() const;
			bool hasDuplicateNames() const;
			std::size_t countNbFilesToUpload() const;
			void checkFiles();
			bool isUploadComplete() const;
			bool isFileDeleted(const Wt::WFileDropWidget::File& file) const;
			void onFileUploaded(const Wt::WFileDropWidget::File& file);

			bool								_validated {};

			Wt::Signal<>						_sigValidated;
			Wt::Signal<unsigned>				_sigProgressUpdate;
			Wt::Signal<ShareCreateParameters>	_sigComplete;

			std::shared_ptr<ShareCreateFormModel> _model;
			Wt::WPushButton*			_createBtn {};

			std::unordered_set<const Wt::WFileDropWidget::File*> _deletedFiles;
			Wt::WFileDropWidget*		_drop {};
			std::uint64_t 				_totalReceivedSize {};
			std::uint64_t 				_currentReceivedSize {};
			std::uint64_t 				_totalSize {};
	};

}
