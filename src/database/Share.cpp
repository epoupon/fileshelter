#include "Share.hpp"

#include "utils/UUID.hpp"

namespace Database {

Share::Share(void)
: _hits(0),
_maxHits(0)
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

bool
Share::hasExpired(void) const
{
	if (_maxHits > 0 && _hits >= _maxHits)
		return true;

	return false;

}


} // namespace Database

