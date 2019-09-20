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

#include "Share.hpp"

#include <Wt/WLocalDateTime.h>

#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/HashFunction.h>

#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include "utils/UUID.hpp"

namespace Database {

Share::Share(void)
:
_downloadUUID(generateUUID()),
_editUUID(generateUUID())
{
}

Share::pointer
Share::create(Wt::Dbo::Session& session, std::filesystem::path path)
{
	auto share = session.add(std::make_unique<Share>());
	auto storePath = share->getPath();

	std::error_code ec;
	std::filesystem::rename(path, storePath, ec);
	if (ec)
	{
		FS_LOG(DB, ERROR) << "Move file failed from " << path.string() << " to " << storePath.string() << ": " << ec.message();
		share.remove();
		return Share::pointer();
	}

	share.modify()->setFileSize(std::filesystem::file_size(storePath));

	return share;
}

void
Share::destroy()
{
	std::error_code ec;
	std::filesystem::remove(getPath(), ec);
	if (ec)
		FS_LOG(DB, ERROR) << "Cannot remove file " << getPath().string() << ": " << ec.message();
}

Share::pointer
Share::getByEditUUID(Wt::Dbo::Session& session, std::string UUID)
{
	pointer res = session.find<Share>().where("edit_UUID = ?").bind(UUID);

	if (!res || !std::filesystem::exists(res->getPath()))
		return pointer();

	return res;
}

Share::pointer
Share::getByDownloadUUID(Wt::Dbo::Session& session, std::string UUID)
{
	pointer res = session.find<Share>().where("download_UUID = ?").bind(UUID);

	if (!res || !std::filesystem::exists(res->getPath()))
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
	if (_maxHits > 0 && _hits >= _maxHits)
		return true;

	auto now = Wt::WLocalDateTime::currentDateTime().toUTC();
	if (now >= _expiryTime)
		return true;

	return false;

}

void
Share::setPassword(Wt::WString password)
{
	auto hashFunc = std::make_unique<Wt::Auth::BCryptHashFunction>(Config::instance().getULong("bcrypt-count", 12));

	Wt::Auth::PasswordVerifier verifier;
	verifier.addHashFunction(std::move(hashFunc));

	auto hash = verifier.hashPassword(password);

	_password = hash.value();
	_salt = hash.salt();
	_hashFunc = hash.function();
}

bool
Share::verifyPassword(Wt::WString password) const
{
	auto hashFunc = std::make_unique<Wt::Auth::BCryptHashFunction>(Config::instance().getULong("bcrypt-count", 12));
	Wt::Auth::PasswordVerifier verifier;
	verifier.addHashFunction(std::move(hashFunc));

	Wt::Auth::PasswordHash hash(_hashFunc, _salt, _password);

	return verifier.verify(password, hash);
}

std::filesystem::path
Share::getPath(void) const
{
	return Config::instance().getPath("working-dir") / "files" / _downloadUUID;
}

std::size_t
Share::getMaxFileSize(void)
{
	return Config::instance().getULong("max-file-size", 100);
}

std::chrono::seconds
Share::getMaxValidatityDuration(void)
{
	const auto durationDay =  std::chrono::hours(24);
	auto maxDuration = durationDay * Config::instance().getULong("max-validity-days", 100);

	if (durationDay > maxDuration)
		maxDuration = durationDay;

	return maxDuration;
}

std::chrono::seconds
Share::getDefaultValidatityDuration(void)
{
	const auto durationDay =  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(24));
	auto defaultDuration = std::chrono::duration_cast<std::chrono::seconds>(durationDay * Config::instance().getULong("default-validity-days", 7));
	auto maxDuration = getMaxValidatityDuration();

	if (defaultDuration < durationDay)
		defaultDuration = durationDay;

	if (defaultDuration > maxDuration)
		defaultDuration = maxDuration;

	return defaultDuration;
}

bool
Share::userCanSetValidatityDuration(void)
{
	return Config::instance().getBool("user-defined-validy-days", true);
}


std::size_t
Share::getMaxValidatityHits(void)
{
	return Config::instance().getULong("max-validity-hits", 100);
}

std::size_t
Share::getDefaultValidatityHits(void)
{
	auto defaultHits = Config::instance().getULong("default-validity-hits", 30);
	auto maxHits = getMaxValidatityHits();

	if (maxHits != 0 && maxHits < defaultHits)
		defaultHits = maxHits;

	return defaultHits;
}

bool
Share::userCanSetValidatityHits(void)
{
	return Config::instance().getBool("user-defined-validy-hits", true);
}

} // namespace Database

