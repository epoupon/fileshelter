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

#include <Wt/WDateTime.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WStackedWidget.h>
#include "utils/Config.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"

#include "database/Share.hpp"
#include "share/ShareUtils.hpp"

#include "ProgressBar.hpp"
#include "FileShelterApplication.hpp"
#include "PasswordUtils.hpp"
#include "ShareCreateFormModel.hpp"
#include "ShareCreateFormView.hpp"
#include "ShareCreateParameters.hpp"
#include "ShareCreatePassword.hpp"
#include "ShareCommon.hpp"

namespace UserInterface {

using namespace Database;



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
	wApp->internalPathChanged().connect([this]
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
	clear();

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
	enum CreateStack
	{
		Form = 0,
		Progress = 1,
	};

	Wt::WStackedWidget* stack {addNew<Wt::WStackedWidget>()};

	auto* form {stack->addNew<ShareCreateFormView>()};
	auto* progress {stack->addNew<ShareCreateProgress>()};

	form->progressUpdate().connect(progress, [=](unsigned progressPerCent) { progress->handleProgressUpdate(progressPerCent); });

	form->validated().connect([=]
	{
		stack->setCurrentIndex(CreateStack::Progress);
	});

	form->complete().connect([this](const ShareCreateParameters& parameters)
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
ShareCreate::createShare(const ShareCreateParameters& parameters)
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

		for (const Wt::Http::UploadedFile* uploadedFile : parameters.uploadedFiles)
		{
			FS_LOG(UI, DEBUG) << "PROCESSING FILE '" << uploadedFile->clientFileName() << "'";

			uploadedFile->stealSpoolFile();
			const std::filesystem::path clientFileName {uploadedFile->clientFileName()};
			ShareUtils::moveFileToShare(FsApp->getDboSession(), share.id(), uploadedFile->spoolFileName(), clientFileName.filename().string());
		}

		share.modify()->setState(Database::Share::State::Ready);

		FS_LOG(UI, INFO) << "[" << share->getDownloadUUID().getAsString() << "] Share created. Client = " << std::string {share->getClientAddr()} << ", size = " << share->getShareSize() << "', desc = '" << std::string {share->getDesc()} << "', expiry " << share->getExpiryTime().toString() << ", password protected = " << share->hasPassword() << ", download URL = '" << getDownloadURL(share) << "'";

		return editUUID;
	}

}

} // namespace UserInterface
