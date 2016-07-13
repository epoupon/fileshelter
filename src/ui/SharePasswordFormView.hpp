#pragma once

#include <Wt/WSignal>
#include <Wt/WTemplateFormView>

namespace UserInterface
{

class SharePasswordFormView : public Wt::WTemplateFormView
{
	private:
		Wt::Signal<void> _sigSuccess;

	public:
		Wt::Signal<void>& success() { return _sigSuccess;}

		SharePasswordFormView(Wt::WContainerWidget *parent = 0);

};

} // namespace UserInterface

