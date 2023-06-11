
add_executable(fileshelter-cmd
	FileShelterCmd.cpp
	Common.cpp
	CreateCommand.cpp
	DestroyCommand.cpp
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


