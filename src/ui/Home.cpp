#include <Wt/WTemplate>
#include <Wt/WPushButton>

#include "Home.hpp"

namespace UserInterface {

Home::Home(Wt::WContainerWidget* parent)
: Wt::WContainerWidget( parent)
{
	Wt::WTemplate *home = new Wt::WTemplate(Wt::WString::tr("template-home"), this);

	Wt::WPushButton *createBtn = new Wt::WPushButton(Wt::WString::tr("msg-create-share"));
	createBtn->addStyleClass("btn-primary");
	createBtn->setLink( Wt::WLink(Wt::WLink::InternalPath, "/share-create") );

	home->bindWidget("create-share-btn", createBtn);
}

} // namespace UserInterface

