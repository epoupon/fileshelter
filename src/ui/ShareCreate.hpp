
#pragma once

#include <Wt/WString>
#include <Wt/WContainerWidget>


namespace UserInterface {

class ShareCreate : public Wt::WContainerWidget
{
	public:
		ShareCreate(Wt::WContainerWidget *parent = 0);

	private:
		void refresh(void);
};



} // namespace UserInterface

