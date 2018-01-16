
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

install-data-hook:
	cd $(DESTDIR)$(fileshelter_docrootdir); \
	rm -f resources; \
	if ! [ -z "$(WT_RESOURCES_DIR)" ]; then \
		$(LN_S) "$(WT_RESOURCES_DIR)" resources; \
	else \
		for wt_dir in "/usr/local/share/Wt/resources" "/usr/share/Wt/resources"; do \
			if [ -d $$wt_dir ]; then \
				$(LN_S) $$wt_dir resources; \
				break; \
			fi \
		done \
	fi

uninstall-local:
	rm -f $(DESTDIR)$(fileshelter_docrootdir)/resources

