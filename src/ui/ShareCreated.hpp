#pragma once

#include <Wt/WContainerWidget>

namespace UserInterface {

class ShareCreated : public Wt::WContainerWidget
{
	public:
		ShareCreated(Wt::WContainerWidget* parent = 0);

	private:
		void refresh(void);

};

} // namespace UserInterface

