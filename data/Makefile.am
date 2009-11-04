bindir=$(exec_prefix)
sbindir=$(exec_prefix)
datadir=$(exec_prefix)

data_DATA = mysql-bahamut.sql mysql-unreal.sql mysql-ultimate28.sql mysql-ultimate30.sql mysql-hybrid.sql mysql-ircdru.sql thales.conf.default

EXTRA_DIST = mysql-bahamut.sql mysql-unreal.sql mysql-ultimate28.sql mysql-ultimate30.sql mysql-hybrid.sql mysql-ircdru.sql thales.conf.default

install-data-local:
	@if test -f $(datadir)/thales.conf ; then \
	   echo "$@ will not overwrite existing $(datadir)/thales.conf" ; \
        else \
           echo "$(INSTALL_DATA) thales.conf.default $(datadir)/thales.conf"; \
           $(INSTALL_DATA) thales.conf.default $(datadir)/thales.conf; \
        fi

uninstall-local:
	@if test -f $(datadir)/thales.conf ; then \
	   echo "rm -f $(datadir)/thales.conf"; \
	   $(RM) -f $(datadir)/thales.conf; \
	fi
