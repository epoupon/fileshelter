
add_executable(filesheltercmd
	FileShelterCmd.cpp
	)

target_link_libraries(filesheltercmd PRIVATE
	filesheltershare
	fileshelterutils
	std::filesystem
	Boost::program_options
	)

install(TARGETS filesheltercmd DESTINATION bin)


