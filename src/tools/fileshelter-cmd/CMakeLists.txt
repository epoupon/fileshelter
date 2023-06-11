
add_executable(fileshelter-cmd
	FileShelterCmd.cpp
	CreateCommand.cpp
	HelpCommand.cpp
	ListCommand.cpp
	)

target_link_libraries(fileshelter-cmd PRIVATE
	filesheltershare
	fileshelterutils
	std::filesystem
	Boost::program_options
	)

install(TARGETS fileshelter-cmd DESTINATION bin)


