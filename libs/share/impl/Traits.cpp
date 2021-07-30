
#include "Traits.hpp"

#include <sstream>

namespace Wt::Dbo
{

	const char*
	sql_value_traits<UUID, void>::type(SqlConnection *conn, int)
	{
		return conn->blobType();
	}

	void
	sql_value_traits<UUID, void>::bind(const UUID& uuid, SqlStatement *statement, int column, int /* size */)
	{
		const std::vector<unsigned char> value(uuid.cbegin(), uuid.cend());
		statement->bind(column, value);
	}

	bool
	sql_value_traits<UUID, void>::read(UUID& id, SqlStatement *statement, int column, int size)
	{
		std::vector<unsigned char> data;

		if (statement->getResult(column, &data, size))
		{
			if (data.size() == 16)
				std::copy(std::cbegin(data), std::cend(data), id.begin());
			else
				id = {};

			return true;
		}
		else
		{
			id = {};
			return false;
		}
	}

	std::string
	sql_value_traits<std::filesystem::path, void>::type(SqlConnection *conn, int size)
	{
		return conn->textType(size) + " not null";
	}

	void
	sql_value_traits<std::filesystem::path, void>::bind(const std::filesystem::path& path, SqlStatement *statement, int column, int /* size */)
	{
		statement->bind(column, path.string());
	}

	bool
	sql_value_traits<std::filesystem::path, void>::read(std::filesystem::path& path, SqlStatement *statement, int column, int size)
	{
		std::string str;

		if (statement->getResult(column, &str, size))
		{
			path = str;
			return true;
		}
		else
		{
			path = std::filesystem::path {};
			return false;
		}
	}

	std::string
	sql_value_traits<Share::FileSize, void>::type(SqlConnection *conn, int)
	{
		return conn->longLongType();
	}

	void
	sql_value_traits<Share::FileSize, void>::bind(Share::FileSize v, SqlStatement *statement, int column, int size)
	{
		if (v > Share::FileSize {std::numeric_limits<long long>::max()})
			throw FsException {"File size too big to fit in db"};

		statement->bind(column, static_cast<long long>(v));
	}

	bool
	sql_value_traits<Share::FileSize, void>::read(Share::FileSize& fileSize, SqlStatement *statement, int column, int size)
	{
		long long readData;
		if (statement->getResult(column, &readData))
		{
			fileSize = readData;
			return true;
		}
		else
		{
			fileSize= 0;
			return false;
		}
	}

} // namespace Wt::Dbo

