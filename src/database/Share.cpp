#include "Share.hpp"

#include "utils/UUID.hpp"

namespace Database {

Share::Share(void)
: _hits(0),
_maxHits(0),
_downloadUUID(generateUUID()),
_editUUID(generateUUID())
{
}

Share::pointer
Share::create(Wt::Dbo::Session& session)
{
	return session.add(new Share());
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

	auto currentDate = boost::gregorian::day_clock::universal_day();
	if (currentDate >= _expiracyTime.date())
		return true;

	return false;

}


} // namespace Database

