#pragma once


namespace Database {

class Updater
{
	public:

		static Updater& instance();

		void setConnectionPool(Wt::Dbo::SqlConnectionPool& connectionPool);

	private:


};

} // namespace Database

