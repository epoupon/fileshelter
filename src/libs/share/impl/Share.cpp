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

#include "File.hpp"
#include "Types.hpp"
#include "utils/Logger.hpp"

namespace Share
{

    FileSize
    Share::getShareSize() const
    {
        assert(self());
        assert(IdIsValid(self()->id()));
        assert(session());

        return session()->query<long long>("SELECT COALESCE(SUM(size), 0) from file WHERE file.share_id = ?").bind(self()->id());
    }

    Wt::Auth::PasswordHash
    Share::getPasswordHash() const
    {
        return { _passwordHashFunc, _passwordSalt, _passwordHash };
    }

    bool
    Share::isExpired() const
    {
        const auto now{ Wt::WLocalDateTime::currentServerDateTime().toUTC() };
        return _expiryTime < now;
    }

    void
    Share::visitFiles(std::function<void(const File::pointer&)> visitor) const
    {
        for (const File::pointer& file : _files)
            visitor(file);
    }

    Share::pointer
    Share::create(Wt::Dbo::Session& session, const ShareCreateParameters& parameters, const Wt::Auth::PasswordHash* passwordHash)
    {
        pointer res{ session.add(std::make_unique<Share>()) };

        const auto now{ Wt::WLocalDateTime::currentServerDateTime().toUTC() };

        res.modify()->_creationTime = now;
        res.modify()->_expiryTime = now.addSecs(std::chrono::duration_cast<std::chrono::seconds>(parameters.validityPeriod).count());
        res.modify()->_desc = parameters.description;
        res.modify()->_creatorAddress = parameters.creatorAddress;
        if (passwordHash)
        {
            res.modify()->_passwordHash = passwordHash->value();
            res.modify()->_passwordSalt = passwordHash->salt();
            res.modify()->_passwordHashFunc = passwordHash->function();
        }
        session.flush();

        return res;
    }

    Share::pointer
    Share::getByEditUUID(Wt::Dbo::Session& session, const ShareEditUUID& uuid)
    {
        return session.find<Share>().where("edit_UUID = ?").bind(uuid);
    }

    Share::pointer
    Share::getByUUID(Wt::Dbo::Session& session, const ShareUUID& uuid)
    {
        return session.find<Share>().where("uuid = ?").bind(uuid);
    }

    void
    Share::visitAll(Wt::Dbo::Session& session, std::function<void(pointer& share)> visitor)
    {
        Wt::Dbo::collection<pointer> res = session.find<Share>();

        for (pointer& share : res)
            visitor(share);
    }

    void
    Share::destroy(pointer& share)
    {
        share->visitFiles([&](const File::pointer& file) {
            if (file->isOwned())
            {
                std::error_code ec;
                std::filesystem::remove(file->getPath(), ec);
                if (ec)
                {
                    FS_LOG(SHARE, ERROR) << "Cannot remove file '" << file->getPath().string() << "' from share '" << share->getUUID().toString() << "': " << ec.message();
                }
                else
                {
                    FS_LOG(SHARE, DEBUG) << "Removed file '" << file->getPath().string() << "' from share '" << share->getUUID().toString() << "'";
                }
            }
        });

        share.remove();
    }

    void
    Share::setPasswordHash(const Wt::Auth::PasswordHash& passwordHash)
    {
        _passwordHash = passwordHash.value();
        _passwordSalt = passwordHash.salt();
        _passwordHashFunc = passwordHash.function();
    }

} // namespace Share
