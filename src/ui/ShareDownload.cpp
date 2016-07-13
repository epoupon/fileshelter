#include <Wt/WTemplate>
#include <Wt/WAnchor>
#include <Wt/WPushButton>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareResource.hpp"
#include "SharePasswordFormView.hpp"

#include "ShareDownload.hpp"


namespace UserInterface {

ShareDownload::ShareDownload(Wt::WContainerWidget* parent)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
}

void
ShareDownload::refresh(void)
{
	if (!wApp->internalPathMatches("/share-download"))
		return;

	clear();

	std::string downloadUUID = wApp->internalPathNextPart("/share-download/");

	Wt::Dbo::Transaction transaction(DboSession());

	Database::Share::pointer share = Database::Share::getByDownloadUUID(DboSession(), downloadUUID);
	if (!share)
	{
		displayNotFound();
		return;
	}

	if (share->hasPassword())
		displayPassword();
	else
		displayDownload();
}

void
ShareDownload::displayDownload(void)
{
	clear();

	std::string downloadUUID = wApp->internalPathNextPart("/share-download/");

	Wt::Dbo::Transaction transaction(DboSession());
	Database::Share::pointer share = Database::Share::getByDownloadUUID(DboSession(), downloadUUID);
	if (!share)
	{
		displayNotFound();
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-download"), this);

	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	if (!share->getDesc().empty())
	{
		t->setCondition("if-desc", true);
		t->bindString("file-desc", share->getDesc());
	}
	t->bindString("file-name", share->getFileName());
	t->bindString("file-size", std::to_string(share->getFileSize() / 1000));

	auto *downloadBtn = new Wt::WPushButton(tr("msg-download"));

	if (share->hasExpired())
	{
		t->setCondition("if-error", true);
		t->bindString("error", tr("msg-share-no-longer-available"));
		downloadBtn->setEnabled(false);
	}
	else
	{
		downloadBtn->setResource(new ShareResource(downloadUUID));
	}

	downloadBtn->addStyleClass("btn btn-primary");
	t->bindWidget("download-btn", downloadBtn);
}

void
ShareDownload::displayNotFound()
{
	clear();
	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-found"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);
}


void
ShareDownload::displayPassword()
{
	clear();

	auto view = new SharePasswordFormView(this);
	view->success().connect(std::bind([=]
	{
		displayDownload();
	}));
}


} // namespace UserInterface

