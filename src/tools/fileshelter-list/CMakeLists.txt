
add_executable(fileshelter-list
	FileShelterList.cpp
	)

target_link_libraries(fileshelter-list PRIVATE
	filesheltershare
	fileshelterutils
	std::filesystem
	Boost::program_options
	)

install(TARGETS fileshelter-list DESTINATION bin)


