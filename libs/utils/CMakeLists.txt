
add_library(fileshelterutils SHARED
	impl/Config.cpp
	impl/Logger.cpp
	impl/String.cpp
	impl/UUID.cpp
	impl/Zipper.cpp
	)

target_include_directories(fileshelterutils INTERFACE
	include
	)

target_include_directories(fileshelterutils PRIVATE
	include
	)

target_link_libraries(fileshelterutils PRIVATE
	PkgConfig::Config++
	)

target_link_libraries(fileshelterutils PUBLIC
	std::filesystem
	Wt::Wt
	)

install(TARGETS fileshelterutils DESTINATION lib)

