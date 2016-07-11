#include <Wt/WFormModel>
#include <Wt/WTemplateFormView>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WDateEdit>
#include <Wt/WSpinBox>
#include <Wt/WIntValidator>
#include <Wt/WAbstractItemModel>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"

#include "database/Share.hpp"

#include "FileShelterApplication.hpp"
#include "ShareCreate.hpp"

namespace UserInterface {


class ShareCreateFormModel : public Wt::WFormModel
{
	public:

		// Associate each field with a unique string literal.
		static const Field DescriptionField;
		static const Field FileField;
		static const Field ExpiracyDateField;
		static const Field HitsValidityField;
		static const Field PasswordField;
		static const Field PasswordConfirmField;

		ShareCreateFormModel(Wt::WObject *parent = 0)
			: Wt::WFormModel(parent)
		{
			addField(DescriptionField);
			addField(FileField);
			addField(ExpiracyDateField);
			addField(HitsValidityField);
			addField(PasswordField);
			addField(PasswordConfirmField);

			auto histValidator = new Wt::WIntValidator(0, 30);
			histValidator->setMandatory(true);
			setValidator(HitsValidityField, histValidator);

//			setValidator(BitrateField, new Wt::WValidator(true)); // mandatory
//			setValidator(EncodingField, new Wt::WValidator(true)); // mandatory

			// Default values
			setValue(ExpiracyDateField, Wt::WDate::currentServerDate().addDays(7));
			setValue(HitsValidityField, 30);

		}

		void saveData(void)
		{
			Wt::Dbo::Transaction transaction(DboSession());

//			CurrentUser().modify()->setAudioBitrate( Wt::asNumber(value(BitrateField)));
		}
};

const Wt::WFormModel::Field ShareCreateFormModel::DescriptionField = "desc";
const Wt::WFormModel::Field ShareCreateFormModel::FileField = "file";
const Wt::WFormModel::Field ShareCreateFormModel::ExpiracyDateField = "expiracy-date";
const Wt::WFormModel::Field ShareCreateFormModel::HitsValidityField = "hits-validity";
const Wt::WFormModel::Field ShareCreateFormModel::PasswordField = "password";
const Wt::WFormModel::Field ShareCreateFormModel::PasswordConfirmField = "password-confirm";

class ShareCreateFormView : public Wt::WTemplateFormView
{
	public:

	ShareCreateFormView(Wt::WContainerWidget *parent = 0)
	: Wt::WTemplateFormView(parent)
	{
		auto _model = new ShareCreateFormModel(this);

		setTemplateText(tr("template-share-create"));
		addFunction("id", &WTemplate::Functions::id);
		addFunction("block", &WTemplate::Functions::id);

		auto _applyInfo = new Wt::WText();
		_applyInfo->setInline(false);
		_applyInfo->setStyleClass("alert alert-danger");
		_applyInfo->hide();
		bindWidget("create-error", _applyInfo);

		// Desc
		Wt::WLineEdit *desc = new Wt::WLineEdit();
		setFormWidget(ShareCreateFormModel::DescriptionField, desc);
		desc->changed().connect(_applyInfo, &Wt::WWidget::hide);

		// File
		Wt::WFileUpload *upload = new Wt::WFileUpload();
		upload->setFileTextSize(1000000); // TODO CONFIG OR CONSTRUCTOR
		upload->setProgressBar(new Wt::WProgressBar());
		setFormWidget(ShareCreateFormModel::FileField, upload);
		upload->changed().connect(_applyInfo, &Wt::WWidget::hide);

		// Time validity
		Wt::WDateEdit *timeValidity = new Wt::WDateEdit();
		setFormWidget(ShareCreateFormModel::ExpiracyDateField, timeValidity);
		timeValidity->setDate(Wt::WDate::currentDate().addDays(7));
		timeValidity->changed().connect(_applyInfo, &Wt::WWidget::hide);

		// Hits validity
		Wt::WSpinBox *hitsValidity = new Wt::WSpinBox();
		setFormWidget(ShareCreateFormModel::HitsValidityField, hitsValidity);
		hitsValidity->changed().connect(_applyInfo, &Wt::WWidget::hide);

		// Password
		Wt::WLineEdit *password = new Wt::WLineEdit();
		password->setEchoMode(Wt::WLineEdit::Password);
		setFormWidget(ShareCreateFormModel::PasswordField, password);
		password->changed().connect(_applyInfo, &Wt::WWidget::hide);

		// Password confirm
		Wt::WLineEdit *passwordConfirm = new Wt::WLineEdit();
		passwordConfirm->setEchoMode(Wt::WLineEdit::Password);
		setFormWidget(ShareCreateFormModel::PasswordConfirmField, password);
		passwordConfirm->changed().connect(_applyInfo, &Wt::WWidget::hide);

		// Title & Buttons
		bindString("title", Wt::WString::tr("msg-create-share"));

		Wt::WPushButton *uploadBtn = new Wt::WPushButton(tr("msg-upload"));
		uploadBtn->setStyleClass("btn-primary");
		bindWidget("create-btn", uploadBtn);
		uploadBtn->clicked().connect(std::bind([=]
		{
			updateModel(_model);

			if (_model->validate())
			{
			FS_LOG(UI, DEBUG) << "validation OK";

			upload->upload();
			uploadBtn->disable();

			upload->uploaded().connect(std::bind([=] ()
			{
				FS_LOG(UI, DEBUG) << "Uploaded!";

				Wt::Dbo::Transaction transaction(DboSession());

				Database::Share::pointer share = Database::Share::create(DboSession());

				// Filename is the DownloadUID
				auto curPath = boost::filesystem::path(upload->spoolFileName());
				auto storePath = Config::instance().getPath("working-dir") / "files" / share->getDownloadUUID();

				boost::system::error_code ec;
				boost::filesystem::rename(curPath, storePath, ec);
				if (ec)
				{
				FS_LOG(UI, INFO) << "Move file failed from " << curPath << " to " << storePath << ": " << ec.message();

				// fallback on copy
				boost::filesystem::copy_file(curPath, storePath, ec);
				if (ec)
				{
					FS_LOG(UI, ERROR) << "Copy file failed from " << curPath << " to " << storePath << ": " << ec.message();
					_applyInfo->show();
					_applyInfo->setText( tr("msg-create-share-failed"));
					return;
				}
				}
				else
					upload->stealSpooledFile();

				share.modify()->setDesc(_model->valueText(ShareCreateFormModel::DescriptionField).toUTF8());
				share.modify()->setPath(storePath);
				share.modify()->setFileName(upload->clientFileName().toUTF8());
				share.modify()->setFileSize(boost::filesystem::file_size(storePath));
				share.modify()->setMaxHits(Wt::asNumber(_model->value(ShareCreateFormModel::HitsValidityField)));
				share.modify()->setExpiracyDate(boost::gregorian::from_string(Wt::asString(_model->value(ShareCreateFormModel::ExpiracyDateField)).toUTF8()));

				transaction.commit();

				wApp->setInternalPath("/share-created/" + share->getEditUUID(), true);
			}));

			upload->fileTooLarge().connect(std::bind([=] ()
			{
				_applyInfo->show();
				_applyInfo->setText( tr("msg-create-share-too-large"));
			}));
			}
			else
			{
				FS_LOG(UI, DEBUG) << "validation KO";

				_applyInfo->show();
				_applyInfo->setText( tr("msg-create-share-failed"));
			}

			updateView(_model);

		}));

		updateView(_model);

	}
};


ShareCreate::ShareCreate(Wt::WContainerWidget* parent)
: Wt::WContainerWidget(parent)
{
	addWidget(new ShareCreateFormView());

	wApp->internalPathChanged().connect(std::bind([=] (std::string path)
	{
		FS_LOG(UI, DEBUG) << "New path = " << path;

		if (wApp->internalPathMatches("/share-create"))
		{
			clear();
			addWidget(new ShareCreateFormView());
		}
	}, std::placeholders::_1));
}

} // namespace UserInterface
