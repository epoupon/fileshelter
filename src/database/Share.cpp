#include "Share.hpp"

#include "utils/UUID.hpp"

namespace Database {

Share::Share(void)
: _nbMaxDownloads(0),
_nbDownloads(00)
{
	// Generate UID

}

Share::pointer
Share::create(Wt::Dbo::Session& session)
{
	Share* share = new Share();

	share->_downloadUUID = generateUUID();
	share->_editUUID = generateUUID();

	return session.add(share);
}


Share::pointer
Share::getByEditUUID(Wt::Dbo::Session& session, std::string UUID)
{
	return session.find<Share>().where("edit_UUID = ?").bind(UUID);
}

Share::pointer
Share::getByDownloadUUID(Wt::Dbo::Session& session, std::string UUID)
{
	return session.find<Share>().where("download_UUID = ?").bind(UUID);
}


} // namespace Database

