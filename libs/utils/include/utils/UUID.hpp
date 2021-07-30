#pragma once

#include <string_view>
#include <boost/uuid/uuid.hpp>

#include "utils/Exception.hpp"

class UUIDException :  public FsException
{
	public:
		using FsException::FsException;
};

class UUID
{
	public:
		struct Generate {};

		UUID() {}
		UUID(Generate);
		UUID(std::string_view uuid);

		std::string toString() const;

		auto cbegin() const { return _uuid.begin(); }
		auto cend() const { return _uuid.end(); }
		auto begin() { return _uuid.begin(); }
		auto end() { return _uuid.end(); }

	protected:
		boost::uuids::uuid _uuid;
};

