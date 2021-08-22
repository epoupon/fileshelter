
add_executable(fileshelter
	main/main.cpp
	resources/ShareResource.cpp
	ui/FileShelterApplication.cpp
	ui/PasswordUtils.cpp
	ui/ProgressBar.cpp
	ui/ShareUtils.cpp
	ui/ShareCreate.cpp
	ui/ShareCreateFormModel.cpp
	ui/ShareCreateFormView.cpp
	ui/ShareCreatePassword.cpp
	ui/ShareCreated.cpp
	ui/ShareDownload.cpp
	ui/ShareEdit.cpp
	ui/ShareDownloadPassword.cpp
	ui/TermsOfService.cpp
	)

target_include_directories(fileshelter PRIVATE
	.
	ui
	)

target_link_libraries(fileshelter PRIVATE
	filesheltershare
	fileshelterutils
	std::filesystem
	Wt::Wt
	Wt::HTTP
	)

install(TARGETS fileshelter DESTINATION bin)

