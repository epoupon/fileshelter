AC_PREREQ(2.59)
AC_INIT(fileshelter, 2.0.1, test@test)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER(src/config/config.h)

AC_LANG_CPLUSPLUS

# Checks for programs
AC_PROG_CXX
AC_PROG_LN_S

PKG_CHECK_MODULES(libzip, libzip >= 1.0.1)

AC_CHECK_HEADERS([Wt/WApplication],
                 [],
                 [AC_MSG_ERROR([Header not found or unusable !])])

AC_CHECK_LIB([boost_system],
		[main],
		,
		[AC_MSG_ERROR([libboost_system not found!])])

AC_CHECK_LIB(	[boost_signals],
		[main],
		,
		[AC_MSG_ERROR([libboost_signals not found!])])

AC_CHECK_LIB(	[boost_filesystem],
		[main],
		,
		[AC_MSG_ERROR([libboost_filesystem not found!])])

AC_CHECK_LIB(	[boost_date_time],
		[main],
		,
		[AC_MSG_ERROR([libboost_date_time not found!])])

AC_CHECK_LIB([wt],
             [main],
             ,
             [AC_MSG_ERROR([libwt not found!])])

AC_CHECK_LIB([wtdbo],
             [main],
             ,
             [AC_MSG_ERROR([libwtdbo not found!])])

AC_CHECK_LIB([wtdbosqlite3],
             [main],
             ,
             [AC_MSG_ERROR([libwtdbosqlite3 not found!])])

AC_CHECK_LIB([wthttp],
	     [main],
	     ,
	     [AC_MSG_ERROR([libwthttp not found!])])


AC_CHECK_LIB(	[config++],
		[main],
		,
		[AC_MSG_ERROR([libconfig not found!])])

AC_CONFIG_FILES([Makefile
		 src/Makefile])

AC_OUTPUT


