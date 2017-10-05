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
_downloadUUID(generateUUID()),
_editUUID(generateUUID())
{
}

Share::pointer
Share::create(Wt::Dbo::Session& session, boost::filesystem::path path)
{
	auto share = session.add(new Share());
	auto storePath = share->getPath();

	boost::system::error_code ec;
	boost::filesystem::rename(path, storePath, ec);
	if (ec)
	{
		FS_LOG(DB, ERROR) << "Move file failed from " << path << " to " << storePath << ": " << ec.message();
		share.remove();
		return Share::pointer();
	}

	share.modify()->setFileSize(boost::filesystem::file_size(storePath));

	return share;
}

void
Share::destroy()
{
	boost::system::error_code ec;
	boost::filesystem::remove(getPath(), ec);
	if (ec)
		FS_LOG(DB, ERROR) << "Cannot remove file " << getPath() << ": " << ec.message();
}

Share::pointer
Share::getByEditUUID(Wt::Dbo::Session& session, std::string UUID)
{
	pointer res = session.find<Share>().where("edit_UUID = ?").bind(UUID);

	if (!res || !boost::filesystem::exists(res->getPath()))
		return pointer();

	return res;
}

Share::pointer
Share::getByDownloadUUID(Wt::Dbo::Session& session, std::string UUID)
{
	pointer res = session.find<Share>().where("download_UUID = ?").bind(UUID);

	if (!res || !boost::filesystem::exists(res->getPath()))
		return pointer();

	return res;
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
	auto now = boost::posix_time::second_clock::universal_time();
	if (now >= _expiryTime)
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

boost::filesystem::path
Share::getPath(void) const
{
	return Config::instance().getPath("working-dir") / "files" / _downloadUUID;
}

std::size_t
Share::getMaxFileSize(void)
{
	return Config::instance().getULong("max-file-size", 100);
}

boost::posix_time::time_duration
Share::getMaxValidatityDuration(void)
{
	const auto durationDay =  boost::posix_time::hours(24);
	auto maxDuration = durationDay * Config::instance().getULong("max-validity-days", 100);

	if (durationDay > maxDuration)
		maxDuration = durationDay;

	return maxDuration;
}

boost::posix_time::time_duration
Share::getDefaultValidatityDuration(void)
{
	const auto durationDay =  boost::posix_time::hours(24);
	auto defaultDuration = durationDay * Config::instance().getULong("default-validity-days", 7);
	auto maxDuration = getMaxValidatityDuration();

	if (defaultDuration < durationDay)
		defaultDuration = durationDay;

	if (defaultDuration > maxDuration)
		defaultDuration = maxDuration;

	return defaultDuration;
}

} // namespace Database

