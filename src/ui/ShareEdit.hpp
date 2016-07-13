#pragma once

#include <Wt/WContainerWidget>

namespace UserInterface {

class ShareEdit : public Wt::WContainerWidget
{
	public:
		ShareEdit(Wt::WContainerWidget* parent = 0);

	private:
		void refresh(void);
		void displayRemoved(void);
		void displayNotFound(void);

};

} // namespace UserInterface

