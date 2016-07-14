/*
 * Copyright (C) 2016 Emeric Poupon
 *
 * This file is part of fileshelter.
 *
 * fileshelter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fileshelter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fileshelter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/HashFunction>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"

#include "Share.hpp"

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

std::vector<Share::pointer>
Share::getAll(Wt::Dbo::Session& session)
{
	Wt::Dbo::collection<pointer> res = session.find<Share>();
	return std::vector<pointer>(res.begin(), res.end());
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

void
Share::setPassword(Wt::WString password)
{
	auto hashFunc = new Wt::Auth::BCryptHashFunction(Config::instance().getULong("bcrypt-count", 12));
	Wt::Auth::PasswordVerifier verifier;
	verifier.addHashFunction(hashFunc);

	auto hash = verifier.hashPassword(password);

	_password = hash.value();
	_salt = hash.salt();
	_hashFunc = hash.function();
}

bool
Share::verifyPassword(Wt::WString password) const
{
	auto hashFunc = new Wt::Auth::BCryptHashFunction();
	Wt::Auth::PasswordVerifier verifier;
	verifier.addHashFunction(hashFunc);

	Wt::Auth::PasswordHash hash(_hashFunc, _salt, _password);

	return verifier.verify(password, hash);
}



} // namespace Database

