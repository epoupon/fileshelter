#include <Wt/WTemplate>
#include <Wt/WPushButton>

#include "utils/Logger.hpp"
#include "database/Share.hpp"

#include "FileShelterApplication.hpp"

#include "ShareEdit.hpp"


namespace UserInterface {

ShareEdit::ShareEdit(Wt::WContainerWidget* parent)
{
	wApp->internalPathChanged().connect(std::bind([=]
	{
		refresh();
	}));

	refresh();
}

void
ShareEdit::refresh(void)
{
	if (!wApp->internalPathMatches("/share-edit"))
		return;

	clear();

	std::string editUUID = wApp->internalPathNextPart("/share-edit/");

	FS_LOG(UI, DEBUG) << "editUUID = '" << editUUID << "'";

	Wt::Dbo::Transaction transaction(DboSession());

	Database::Share::pointer share = Database::Share::getByEditUUID(DboSession(), editUUID);
	if (!share || !boost::filesystem::exists(share->getPath()))
	{
		FS_LOG(UI, ERROR) << "Edit UUID '" << editUUID << "' not found";
		this->addWidget(new Wt::WTemplate(tr("template-share-not-found")));
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-edit"), this);

	t->bindString("file-desc", share->getDesc());
	t->bindString("file-name", share->getFileName());
	t->bindString("file-size", std::to_string(share->getFileSize() / 1000));
	t->bindString("expiracy-date", boost::gregorian::to_simple_string(share->getExpiracyDate()));
	t->bindString("hits", std::to_string(share->getHits()));
	t->bindString("max-hits", std::to_string(share->getMaxHits()));

	auto *deleteBtn = new Wt::WPushButton(tr("msg-delete"));
	deleteBtn->addStyleClass("btn btn-danger");
	t->bindWidget("delete-btn", deleteBtn);

}

} // namespace UserInterface

