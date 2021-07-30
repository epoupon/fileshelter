#pragma once

#include <chrono>
#include <filesystem>
#include <string>

namespace Share
{
	struct FileCreateParameters
	{
		std::filesystem::path path;
		std::string name;
	};

	struct ShareCreateParameters
	{
		std::chrono::seconds validityDuration; // 0 if infinite
		std::string description;
		std::string creatorAddress;
		std::string password;
	};
}
