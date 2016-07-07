#pragma once

#include <Wt/WContainerWidget>

namespace UserInterface {

class ShareDownload : public Wt::WContainerWidget
{
	public:
		ShareDownload(Wt::WContainerWidget* parent = 0);

	private:
		void refresh(void);

};

} // namespace UserInterface

