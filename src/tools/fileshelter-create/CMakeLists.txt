
add_executable(fileshelter-create
	FileShelterCreate.cpp
	)

target_link_libraries(fileshelter-create PRIVATE
	filesheltershare
	fileshelterutils
	std::filesystem
	Boost::program_options
	)

install(TARGETS fileshelter-create DESTINATION bin)


