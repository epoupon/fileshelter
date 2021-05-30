
add_executable(fileshelter
	main/main.cpp
	database/Db.cpp
	database/File.cpp
	database/Share.cpp
	resources/FileResource.cpp
	resources/ShareResource.cpp
	share/ShareCleaner.cpp
	share/ShareUtils.cpp
	ui/FileShelterApplication.cpp
	ui/PasswordUtils.cpp
	ui/ProgressBar.cpp
	ui/ShareCommon.cpp
	ui/ShareCreate.cpp
	ui/ShareCreateFormModel.cpp
	ui/ShareCreateFormView.cpp
	ui/ShareCreateParameters.cpp
	ui/ShareCreatePassword.cpp
	ui/ShareCreated.cpp
	ui/ShareDownload.cpp
	ui/ShareEdit.cpp
	ui/ShareDownloadPassword.cpp
	ui/TermsOfService.cpp
	utils/Config.cpp
	utils/Logger.cpp
	utils/String.cpp
	utils/UUID.cpp
	utils/Zipper.cpp
	)

target_include_directories(fileshelter PRIVATE
	.
	ui
	)

target_link_libraries(fileshelter PRIVATE
	std::filesystem
	PkgConfig::Config++
	Wt::Wt
	Wt::Dbo
	Wt::DboSqlite3
	Wt::HTTP
	)

install(TARGETS fileshelter DESTINATION bin)

