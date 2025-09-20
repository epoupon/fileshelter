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

#pragma once

#include "Traits.hpp"
#include "share/CreateParameters.hpp"
#include <Wt/Auth/PasswordHash.h>
#include <Wt/Dbo/Dbo.h>

namespace Share
{

    class File;

    class Share : public Wt::Dbo::Dbo<Share>
    {
    public:
        using pointer = Wt::Dbo::ptr<Share>;

        // getters
        const ShareUUID& getUUID() const { return _uuid; }
        const ShareEditUUID& getEditUUID() const { return _editUuid; }
        FileSize getShareSize() const;
        bool hasPassword() const { return !_passwordHash.empty(); }
        Wt::Auth::PasswordHash getPasswordHash() const;
        std::string_view getDescription() const { return _desc; }

        Wt::WDateTime getCreationTime() const { return _creationTime; }
        bool isExpired() const;
        Wt::WDateTime getExpiryTime() const { return _expiryTime; }
        std::string_view getCreatorAddr() const { return _creatorAddress; }
        std::size_t getReadCount() const { return _readCount; }

        void visitFiles(std::function<void(const Wt::Dbo::ptr<File>&)> func) const;

        void incReadCount() { _readCount++; }

        // Helpers
        static pointer create(Wt::Dbo::Session& session, const ShareCreateParameters& parameters, const Wt::Auth::PasswordHash* passwordHash = nullptr);
        static pointer getByUUID(Wt::Dbo::Session& session, const ShareUUID& shareId);
        static pointer getByEditUUID(Wt::Dbo::Session& session, const ShareEditUUID& uuid);

        static void visitAll(Wt::Dbo::Session& session, std::function<void(pointer& share)> visitor);
        static void destroy(pointer& share);

        // Setters
        void setUUID(const ShareUUID& uuid) { _uuid = uuid; }
        void setEditUUID(const ShareEditUUID& uuid) { _editUuid = uuid; }
        void setPasswordHash(const Wt::Auth::PasswordHash& passwordHash);

    public:
        template<class Action>
        void persist(Action& a)
        {
            Wt::Dbo::field(a, _shareName, "share_name");
            Wt::Dbo::field(a, _creatorAddress, "creator_addr");
            Wt::Dbo::field(a, _passwordHash, "password_hash");
            Wt::Dbo::field(a, _passwordSalt, "password_salt");
            Wt::Dbo::field(a, _passwordHashFunc, "password_hash_func");
            Wt::Dbo::field(a, _desc, "desc");
            Wt::Dbo::field(a, _creationTime, "creation_time");
            Wt::Dbo::field(a, _expiryTime, "expiry_time");
            Wt::Dbo::field(a, _uuid, "uuid");
            Wt::Dbo::field(a, _editUuid, "edit_uuid");
            Wt::Dbo::field(a, _readCount, "read_count");

            Wt::Dbo::hasMany(a, _files, Wt::Dbo::ManyToOne, "share");
        }

    private:
        std::string _shareName;
        std::string _creatorAddress; // Client IP address that uploaded the file
        std::string _passwordHash;   // optional
        std::string _passwordSalt;
        std::string _passwordHashFunc;
        std::string _desc; // optional

        Wt::WDateTime _creationTime;
        Wt::WDateTime _expiryTime;

        ShareUUID _uuid;
        ShareEditUUID _editUuid;

        long long _readCount{};

        Wt::Dbo::collection<Wt::Dbo::ptr<File>> _files;
    };

} // namespace Share
