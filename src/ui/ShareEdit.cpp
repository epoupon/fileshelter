#include <Wt/WTemplate>
#include <Wt/WPushButton>
#include <Wt/WMessageBox>

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
		Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-not-found"), this);
		t->addFunction("tr", &Wt::WTemplate::Functions::tr);
		return;
	}

	Wt::WTemplate *t = new Wt::WTemplate(tr("template-share-edit"), this);
	t->addFunction("tr", &Wt::WTemplate::Functions::tr);

	if (!share->getDesc().empty())
	{
		t->setCondition("if-desc", true);
		t->bindString("file-desc", share->getDesc());
	}
	t->bindString("file-name", share->getFileName());
	t->bindString("file-size", std::to_string(share->getFileSize() / 1000));
	t->bindString("expiracy-date", boost::gregorian::to_simple_string(share->getExpiracyDate()));
	t->bindString("hits", std::to_string(share->getHits()));
	t->bindString("max-hits", std::to_string(share->getMaxHits()));

	auto *deleteBtn = new Wt::WPushButton(tr("msg-delete"));
	deleteBtn->addStyleClass("btn btn-danger");
	t->bindWidget("delete-btn", deleteBtn);

	deleteBtn->clicked().connect(std::bind([=] ()
	{
		Wt::WMessageBox *messageBox = new Wt::WMessageBox
			(tr("msg-share-delete"),
			tr("msg-confirm-action"),
			 Wt::Question, Wt::Yes | Wt::No);

		messageBox->setModal(true);

		messageBox->buttonClicked().connect(std::bind([=] ()
		{
			if (messageBox->buttonResult() == Wt::Yes)
			{
				Wt::Dbo::Transaction transaction(DboSession());

				Database::Share::pointer share = Database::Share::getByEditUUID(DboSession(), editUUID);

				if (share)
					share.remove();

				wApp->setInternalPath("/home", true);
			}

			delete messageBox;
		}));

		messageBox->show();
	}));

}

} // namespace UserInterface

