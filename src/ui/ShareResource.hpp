#pragma once

#include <boost/filesystem.hpp>

#include <Wt/WStreamResource>


namespace UserInterface {

class ShareResource : public Wt::WStreamResource
{
	public:

		ShareResource(std::string downloadUUID, Wt::WObject *parent = 0);
		~ShareResource();

		void setFileName(const std::string& name);

	private:

		void handleRequest(const Wt::Http::Request& request,
				  Wt::Http::Response& response);

		std::string _downloadUUID;
		boost::filesystem::path _path;
};

} // namespace UserInterface

