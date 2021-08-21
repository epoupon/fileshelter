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

#include <Wt/WApplication.h>
#include <Wt/WStackedWidget.h>
#include "share/IShareManager.hpp"
#include "utils/Logger.hpp"
#include "utils/Service.hpp"

#include "PasswordUtils.hpp"
#include "ProgressBar.hpp"
#include "ShareCreateFormView.hpp"
#include "ShareCreatePassword.hpp"

namespace UserInterface
{

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
		wApp->internalPathChanged().connect(this, [this]
		{
			handlePathChanged();
		});

		handlePathChanged();
	}


	void
	ShareCreate::handlePathChanged()
	{
		clear();

		if (!wApp->internalPathMatches("/share-create"))
			return;

		if (PasswordUtils::isUploadPassordRequired())
			displayPassword();
		else
			displayCreate();
	}

	void
	ShareCreate::displayPassword()
	{
		ShareCreatePassword* view {addNew<ShareCreatePassword>()};
		view->success().connect([=]
		{
			displayCreate();
		});
	}

	void
	ShareCreate::displayCreate()
	{
		using namespace Share;

		enum CreateStack
		{
			Form = 0,
			Progress = 1,
		};

		Wt::WStackedWidget* stack {addNew<Wt::WStackedWidget>()};

		ShareCreateFormView* form {stack->addNew<ShareCreateFormView>()};
		ShareCreateProgress* progress {stack->addNew<ShareCreateProgress>()};

		form->progressUpdate().connect(progress, [=](unsigned progressPerCent) { progress->handleProgressUpdate(progressPerCent); });

		form->validated().connect([=]
		{
			stack->setCurrentIndex(CreateStack::Progress);
		});

		form->complete().connect([=](const ShareCreateParameters& shareParameters, const std::vector<FileCreateParameters>& filesParameters)
		{
			FS_LOG(UI, DEBUG) << "Upload complete!";
			const Share::ShareEditUUID editUUID {Service<IShareManager>::get()->createShare(shareParameters, filesParameters, true /* transfer ownership */)};

			FS_LOG(UI, DEBUG) << "Redirecting...";
			wApp->setInternalPath("/share-created/" + editUUID.toString(), true);

			// Clear the widget in order to flush the temporary uploaded files
			FS_LOG(UI, DEBUG) << "Clearing...";
			clear();
			FS_LOG(UI, DEBUG) << "Clearing done";
		});
	}

} // namespace UserInterface
