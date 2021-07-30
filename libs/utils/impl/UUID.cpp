
#include "utils/UUID.hpp"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

UUID::UUID(Generate)
: _uuid {boost::uuids::random_generator{}()}
{
}

std::string
UUID::toString() const
{
	return boost::uuids::to_string(_uuid);
}

UUID::UUID(std::string_view uuid)
{
	try
	{
		boost::uuids::string_generator gen;
		_uuid = gen(std::string {uuid});
	}
	catch (const std::runtime_error& e)
	{
		throw UUIDException {"Invalid UUID format"};
	}
}

