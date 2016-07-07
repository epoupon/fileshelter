#include <sstream>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include "UUID.hpp"

std::string generateUUID(void)
{
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	std::ostringstream oss;
	oss << uuid;

	return oss.str();
}

