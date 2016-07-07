#include <Wt/WTemplate>
#include <Wt/WAnchor>
#include <Wt/WEnvironment>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"

#include "ShareCreated.hpp"


namespace UserInterface {

ShareCreated::ShareCreated(Wt::WContainerWidget* parent)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
}

void
ShareCreated::refresh(void)
{
	if (!wApp->internalPathMatches("/share-created"))
		return;

	clear();

	std::string editUUID = wApp->internalPathNextPart("/share-created/");

	FS_LOG(UI, DEBUG) << "editUUID = '" << editUUID << "'";

	Wt::Dbo::Transaction transaction(DboSession());

	Database::Share::pointer share = Database::Share::getByEditUUID(DboSession(), editUUID);
	if (!share)
	{
		FS_LOG(UI, ERROR) << "Edit UUID '" << editUUID << "' not found";
		this->addWidget(new Wt::WTemplate(tr("template-share-not-found")));
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-created"), this);

	std::string downloadPath = "/share-download/" + share->getDownloadUUID();
	std::string editPath = "/share-edit/" + share->getEditUUID();

	t->bindWidget("public-link", new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, downloadPath), wApp->environment().headerValue("Host") + downloadPath));
	t->bindWidget("edit-link", new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, editPath), wApp->environment().headerValue("Host") + editPath));
}

} // namespace UserInterface

