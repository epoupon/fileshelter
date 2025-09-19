/*
 * Copyright (C) 2020 Emeric Poupon
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

#include <string>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include "Traits.hpp"
#include "share/CreateParameters.hpp"
#include "share/Types.hpp"

namespace Share
{
    class Share;

    class File
    {
    public:
        using pointer = Wt::Dbo::ptr<File>;

        // Helpers
        static pointer create(Wt::Dbo::Session& session, const FileCreateParameters& parameters, Wt::Dbo::ptr<Share> share);
        static pointer getByPath(Wt::Dbo::Session& session, const std::filesystem::path& path);

        // Getters
        const FileUUID& getUUID() const { return _uuid; }
        const std::filesystem::path& getClientPath() const { return _name; }
        FileSize getSize() const { return _size; }
        const std::filesystem::path& getPath() const { return _path; }
        bool isOwned() const { return _isOwned; }

        // Setters
        void setUUID(const FileUUID& uuid) { _uuid = uuid; }
        void setIsOwned(bool value) { _isOwned = value; }
        void setSize(FileSize size) { _size = size; }

        template<class Action>
        void persist(Action& a)
        {
            Wt::Dbo::field(a, _name, "name");
            Wt::Dbo::field(a, _size, "size");
            Wt::Dbo::field(a, _path, "path");
            Wt::Dbo::field(a, _isOwned, "is_owned");

            Wt::Dbo::field(a, _uuid, "uuid"); // not used yet

            Wt::Dbo::belongsTo(a, _share, "share", Wt::Dbo::OnDeleteCascade);
        }

    private:
        friend class ShareManager;

        std::filesystem::path _name;
        FileSize _size{};
        std::filesystem::path _path;
        bool _isOwned{};

        FileUUID _uuid;

        Wt::Dbo::ptr<Share> _share;
    };

} // namespace Share
