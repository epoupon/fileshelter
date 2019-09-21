AC_PREREQ(2.59)
AC_INIT(fileshelter, 4.0.1, test@test)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER(src/config/config.h)

AC_LANG_CPLUSPLUS

# Checks for programs
AC_PROG_CXX

PKG_CHECK_MODULES(libzip, libzip >= 0.11)

AC_ARG_VAR([WT_RESOURCES_DIR],[Specify Wt's resources directory (default are '/usr/share/Wt/resources' and '/usr/local/share/Wt/resources')'])

AC_CHECK_HEADERS([Wt/WApplication.h],
                 [],
                 [AC_MSG_ERROR([Header not found or unusable !])])

AC_CHECK_LIB([pthread],
		[pthread_create],
		,
		[AC_MSG_ERROR([libpthread not found!])])

AC_CHECK_LIB([stdc++fs],
		[main],
		,
		[AC_MSG_ERROR([libstdc++fs not found!])])

AC_CHECK_LIB([boost_system],
		[main],
		,
		[AC_MSG_ERROR([libboost_system not found!])])

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


