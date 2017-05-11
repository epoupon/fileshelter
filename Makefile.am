
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
		approot/messages_fr.xml \
		approot/messages_ru.xml \
		approot/tos.xml

dist_sysconf_DATA = \
	        $(top_srcdir)/conf/fileshelter.conf

install-data-hook:
	cd $(DESTDIR)$(fileshelter_docrootdir); \
	for wt_dir in "/usr/local/share/Wt/resources" "/usr/share/Wt/resources"; do \
		if [ -d $$wt_dir ]; then \
			if ! [ -e  ./resources ] ; then \
				$(LN_S) /usr/share/Wt/resources resources; \
			fi; \
			break; \
		fi \
	done

uninstall-local:
	rm -f -r $(fileshelter_docrootdir)/resources

