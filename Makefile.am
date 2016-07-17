
AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

SUBDIRS = src

fileshelter_docrootdir=$(pkgdatadir)/docroot
fileshelter_approotdir=$(pkgdatadir)/approot

fileshelter_approot_DATA = \
		approot/templates.xml	\
		approot/messages.xml	\
		approot/messages_fr.xml \
		approot/tos.xml

sysconf_DATA = \
	        $(top_srcdir)/conf/fileshelter.conf

install-data-hook:
	ln -s $(datadir)/Wt/resources $(fileshelter_docrootdir)/resources || echo "Skipped symlink creation"

uninstall-local:
	cd $(fileshelter_docrootdir) && rm -f -r $(fileshelter_docrootdir)/resources

