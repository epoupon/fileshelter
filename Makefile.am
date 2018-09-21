
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

SUBDIRS = src

fileshelter_docrootdir=$(pkgdatadir)/docroot
fileshelter_approotdir=$(pkgdatadir)/approot

fileshelter_cssdir=$(fileshelter_docrootdir)/css

dist_fileshelter_css_DATA = \
		 docroot/css/fileshelter.css

dist_fileshelter_approot_DATA = \
		approot/templates.xml	\
		approot/messages.xml	\
		approot/messages_de.xml	\
		approot/messages_fr.xml \
		approot/messages_ru.xml \
		approot/tos.xml

dist_sysconf_DATA = \
	        $(top_srcdir)/conf/fileshelter.conf

