#include <fstream>
#include <Wt/Http/Response>

#include "database/Share.hpp"
#include "utils/Logger.hpp"

#include "FileShelterApplication.hpp"

#include "ShareResource.hpp"


namespace UserInterface {

ShareResource::ShareResource(std::string downloadUUID, Wt::WObject *parent)
  : WStreamResource(parent),
    _downloadUUID(downloadUUID)
{
}

ShareResource::~ShareResource(void)
{
  beingDeleted();
}

void
ShareResource::handleRequest(const Wt::Http::Request& request,
				  Wt::Http::Response& response)
{

	if (!request.continuation())
	{
		FS_LOG(UI, INFO) << "[" << _downloadUUID << "] - Not a continuation...";

		// Sessions are not thread safe in resources
		Wt::WApplication::UpdateLock lock(wApp);

		Wt::Dbo::Transaction transaction(DboSession());

		auto share = Database::Share::getByDownloadUUID(DboSession(), _downloadUUID);

		FS_LOG(UI, INFO) << "Hits = " << share->getHits() << " / " << share->getMaxHits();

		if (share->hasExpired())
		{
			FS_LOG(UI, WARNING) << "Share expired!";
			response.setStatus(404);
			return;
		}

		suggestFileName(share->getFileName());
		_path = share->getPath();
		share.modify()->incHits();

		FS_LOG(UI, INFO) << "Hits now set to " << share->getHits() << " / " << share->getMaxHits();
	}
	else
	{
		FS_LOG(UI, INFO) << "[" << _downloadUUID << "] - continuation!";

		if (_path.empty())
		{
			FS_LOG(UI, ERROR) << "Path not set!";
			return;
		}
	}

	std::ifstream is(_path.string().c_str(), std::ios::in | std::ios::binary);
	if (!is)
	{
		FS_LOG(UI, ERROR) << "Cannot open " << _path;
		return;
	}
	else
		handleRequestPiecewise(request, response, is);
}

}
