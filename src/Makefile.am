bin_PROGRAMS = fileshelter

fileshelter_SOURCES = \
	$(srcdir)/main/main.cpp			\
	$(srcdir)/database/DbCleaner.cpp	\
	$(srcdir)/database/DbHandler.cpp	\
	$(srcdir)/database/Share.cpp		\
	$(srcdir)/ui/FileShelterApplication.cpp	\
	$(srcdir)/ui/ShareCreate.cpp		\
	$(srcdir)/ui/ShareCreated.cpp		\
	$(srcdir)/ui/ShareDownload.cpp		\
	$(srcdir)/ui/ShareEdit.cpp		\
	$(srcdir)/ui/SharePasswordFormView.cpp	\
	$(srcdir)/ui/ShareResource.cpp		\
	$(srcdir)/utils/Config.cpp		\
	$(srcdir)/utils/Logger.cpp		\
	$(srcdir)/utils/UUID.cpp

# DBOOST_NO_CXX11_SCOPED_ENUMS prevents a link error with outdated boost filesystem lib
fileshelter_CXXFLAGS=-std=c++11 -Wall -I$(srcdir)/ui -DDBOOST_NO_CXX11_SCOPED_ENUMS

