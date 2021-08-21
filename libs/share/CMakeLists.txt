
add_library(filesheltershare SHARED
	impl/Db.cpp
	impl/File.cpp
	impl/Share.cpp
	impl/ShareCleaner.cpp
	impl/ShareManager.cpp
	impl/Traits.cpp
	)

target_include_directories(filesheltershare INTERFACE
	include
	)

target_include_directories(filesheltershare PRIVATE
	include
	)

target_link_libraries(filesheltershare PRIVATE
	PkgConfig::Config++
	Wt::Dbo
	Wt::DboSqlite3
	)

target_link_libraries(filesheltershare PUBLIC
	fileshelterutils
	std::filesystem
	Wt::Wt
	)

install(TARGETS filesheltershare DESTINATION lib)

