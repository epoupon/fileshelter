cmake_minimum_required(VERSION 3.12)

project(fileshelter)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules/)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Filesystem REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(Wt REQUIRED COMPONENTS Wt Dbo DboSqlite3 HTTP)
pkg_check_modules(Config++ REQUIRED IMPORTED_TARGET libconfig++)
find_package(GTest REQUIRED)

# WT
if (NOT Wt_FOUND)
	message(FATAL_ERROR "Wt package not found!")
endif ()
if (NOT TARGET Wt::Wt)
	message(FATAL_ERROR "Cannot find Wt::Wt!")
endif ()
if (NOT TARGET Wt::Dbo)
	message(FATAL_ERROR "Cannot find Wt::Dbo!")
endif ()
if (NOT TARGET Wt::DboSqlite3)
	message(FATAL_ERROR "Cannot find Wt::DboSqlite3!")
endif ()
if (NOT TARGET Wt::HTTP)
	message(FATAL_ERROR "Cannot find Wt::HTTP!")
endif ()

add_subdirectory(src)

install(DIRECTORY approot DESTINATION share/fileshelter)
install(DIRECTORY docroot DESTINATION share/fileshelter)
install(FILES conf/systemd/default.service DESTINATION share/fileshelter)
install(FILES conf/fileshelter.conf DESTINATION share/fileshelter)

