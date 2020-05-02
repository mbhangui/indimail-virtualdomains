#
# Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
# distribution information.

@SET_MAKE@

TEMPLFILES=packaging/suse/courier-imap.init.in \
	packaging/suse/imap.pam \
	packaging/suse/pop3.pam

SUBDIRS=libs/numlib libs/bdbobj libs/gdbmobj libs/soxwrap \
	libs/unicode libs/rfc822 libs/rfc2045 \
	libs/md5 libs/sha1 libs/random128 libs/rfc1035 libs/liblock \
	libs/maildir libs/makedat libs/libhmac libs/authlib \
	libs/waitlib libs/tcpd libs/rootcerts libs/imap libs/maildrop

DISTCLEANFILES=stamp-h1 config.h config.log packaging/systemd/courier-imap.sysvinit packaging/systemd/courier-imap.service \
	packaging/suse/courier-imap.init pop3d.rc dbobj.h imapd-ssl.rc pop3d-ssl.rc imapd.rc dbobj.config $(MAILDROPHTML)

CLEANFILES=$(databin_SCRIPTS) $(man_MANS) $(sysconf_DATA) $(sbin_SCRIPTS)

EXTRA_PROGRAMS=

EXTRA_DIST=README README.cygwin\
	COPYING.GPL NEWS.html $(TEMPLFILES) pgpkeys.txt \
	INSTALL.html maildroptips.txt manpage.css

if HAVE_SGML
manpage.css: libs/docbook/manpage.css
	cp -f $(srcdir)/libs/docbook/manpage.css manpage.css
endif

htmldir=$(docdir)/html
MAILDROPHTML=lockmail.html maildiracl.html maildirkw.html \
	maildirmake.html maildropex.html \
	maildir.html \
	maildropfilter.html maildropgdbm.html maildrop.html mailbot.html \
	makemime.html reformail.html reformime.html

html_DATA=$(MAILDROPHTML) manpage.css

sysconf_DATA=quotawarnmsg.example
libexec_SCRIPTS=imapd.rc imapd-ssl.rc pop3d.rc pop3d-ssl.rc

HTML2TXT=links -dump -no-numbering -no-references

NEWS: NEWS.html
	$(HTML2TXT) NEWS.html >NEWS
distclean-local:
	-rm -rf .deps libs/*/.deps autom4te.cache libs/*/autom4te.cache

quotawarnmsg.example: libs/maildir/quotawarnmsg
	cp -f $(srcdir)/libs/maildir/quotawarnmsg quotawarnmsg.example

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libexecdir)/imapmodules
	for f in `cat libs/authlib/installlist` dummy ; do \
		test "$$f" = "dummy" && continue ; \
		echo $$f; \
		$(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) libs/authlib/$$f \
			$(DESTDIR)$(libexecdir)/imapmodules/$$f ; done
	$(mkinstalldirs) $(DESTDIR)$(databindir)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	test ! -x libs/tcpd/couriertls || $(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) \
		libs/tcpd/couriertls $(DESTDIR)$(bindir)/couriertls

install-exec-hook:
	chmod 444 $(DESTDIR)$(sysconfdir)/quotawarnmsg.example
	mkdir -p $(DESTDIR)$(sysconfdir)/shared
	chmod 755 $(DESTDIR)$(sysconfdir)/shared
	mkdir -p $(DESTDIR)$(sysconfdir)/shared.tmp
	chmod 755 $(DESTDIR)$(sysconfdir)/shared.tmp
	mkdir -p $(DESTDIR)$(sysconfdir)/imapaccess
	chmod 755 $(DESTDIR)$(sysconfdir)/shared.tmp

install-data-local: install-man
	$(INSTALL_DATA) libs/authlib/modulelist $(DESTDIR)$(sysconfdir)/authmodulelist
	for f in `cat libs/authlib/modulelist ; echo $$authmodulelist ` ; do \
		echo $$f; \
		test "$$f" = "authpam" || test "$$f" = "authshadow" || continue ; \
		chmod 4555 $(DESTDIR)/$(libexecdir)/imapmodules/$$f || exit 1; \
	done
	test -d $(DESTDIR)/etc/pam.d || exit 0 ; \
	test -w $(DESTDIR)/etc/pam.d || exit 0 ; \
	test -f $(DESTDIR)/etc/pam.d/imap || \
		$(INSTALL_DATA) $(srcdir)/libs/imap/imapd.pam \
		$(DESTDIR)/etc/pam.d/imap || exit 1 ;\
	test -f $(DESTDIR)/etc/pam.d/pop3 || \
		$(INSTALL_DATA) $(srcdir)/libs/imap/pop3d.pam \
		$(DESTDIR)/etc/pam.d/pop3 || exit 1
	for f in $(man_MANS); do chmod a+r $(DESTDIR)$(mandir)/*/$$f; done
	@echo "Do not forget to run make install-configure"

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/couriertls

INSTALL.txt: INSTALL.html
	$(HTML2TXT) INSTALL.html >INSTALL.txt

#
# For maintainer's use only
#

dist-hook: courier-imap.spec
	test -f $(srcdir)/rpm.release && cp $(srcdir)/rpm.release $(distdir) \
		; exit 0
	rm -f $(distdir)/INSTALL
	$(HTML2TXT) INSTALL.html >$(distdir)/INSTALL
	cp $(srcdir)/courier-imap.spec $(distdir)

if HAVE_HTML

README: html/main.html
	$(HTML2TXT) $(srcdir)/html/index.html >README
endif

distrelease:
	umask 022; $(MAKE) distcheck
	$(MAKE) rpm
	$(MAKE)
	$(MAKE)
	$(MAKE) release

release: dist

rpm: build-rpm

build-rpm:
	$(MAKE) courier-imap.spec
	$(MAKE) bump.rpm.release
	$(MAKE) dist
	rm -rf rpm/BUILD/*
	rm -f rpm/RPMS/*/*
	rm -f rpm/SOURCES/*
	rm -f rpm/SPECS/*
	rm -f rpm/SRPMS/*
	rpmbuild -ta --clean \
		--define "_topdir `pwd`/rpm" \
		--define '_rpmdir %{_topdir}/RPMS' \
		--define '_srcrpmdir %{_topdir}/SRPMS' \
		--define '_sourcedir %{_topdir}/SOURCES' \
		--define '_specdir %{_topdir}/SPECS' \
		--define '_builddir %{_topdir}/BUILD' \
		--define '_build_name_fmt %%{ARCH}/%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm' \
		--define '_tmppath %{_var}/tmp' \
		--define '__spec_prep_pre %{___build_pre}' \
		@PACKAGE@-@VERSION@.tar.bz2

lpm: lpm-build

lpm-build:
	$(MAKE) -$(MAKEFLAGS) bump.rpm.release
	$(MAKE) -$(MAKEFLAGS) dist
	rm -rf lpm
	mkdir lpm
	lpbuild --srcbuild -o lpm @PACKAGE@-@VERSION@.tar.bz2

publish:
	lpbuild --resign lpm/*
	rm -rf lpmtmp
	mkdir lpmtmp
	ln lpm/* lpmtmp
	mv lpmtmp/*.src.lp repository/`lpm -q --distribution`/sources
	mv lpmtmp/*.lp repository/`lpm -q --distribution`/packages
	rm -rf lpmtmp
	lpm --sync repository && rsync -a -v --delete-after repository/. headache:lpmtool.com/www/repositories/courier-imap

bump.rpm.release: @PACKAGE@.spec
	VERSION="x"; \
	test -f $(srcdir)/rpm.release && . $(srcdir)/rpm.release; \
	NEXT_RELEASE=0; \
	test "$$VERSION" = "@VERSION@" && NEXT_RELEASE="$$RELEASE"; \
	RELEASE=`expr $$NEXT_RELEASE + 1`; \
	echo "VERSION=@VERSION@" >$(srcdir)/rpm.release; \
	echo "RELEASE=$$RELEASE" >>$(srcdir)/rpm.release; \
	sed 's/^Release: .*/Release: '$$RELEASE'%{courier_release}/' \
		courier-imap.spec >courier-imap.spec.new ; \
	mv courier-imap.spec.new courier-imap.spec

all-recursive: all-make-check

all-make-check:
	$(MAKE) all-gmake-check FOO=BAR

all-gmake-check:
	@test "$(FOO)" = "BAR" && exit 0 ; echo "" ; \
		echo "ERROR: Something's wrong.  If you're using xBSD, you must use gmake" ; \
		echo "instead of make, as you were told by the INSTALL file (you did read the" ; \
		echo "INSTALL file, did you?)" ; echo "" ; exit 1

lockmail.html: libs/liblock/lockmail.html
	cp -f $(srcdir)/libs/liblock/lockmail.html lockmail.html
maildir.html: libs/maildir/maildir.html
	cp -f $(srcdir)/libs/maildir/maildir.html maildir.html
maildiracl.html: libs/maildir/maildiracl.html
	cp -f $(srcdir)/libs/maildir/maildiracl.html maildiracl.html
maildirkw.html: libs/maildir/maildirkw.html
	cp -f $(srcdir)/libs/maildir/maildirkw.html maildirkw.html
maildirmake.html: libs/maildir/maildirmake.html
	cp -f $(srcdir)/libs/maildir/maildirmake.html maildirmake.html
mailbot.html: libs/maildrop/mailbot.html
	cp -f $(srcdir)/libs/maildrop/mailbot.html mailbot.html
maildrop.html: libs/maildrop/maildrop.html
	cp -f $(srcdir)/libs/maildrop/maildrop.html maildrop.html
reformail.html: libs/maildrop/reformail.html
	cp -f $(srcdir)/libs/maildrop/reformail.html reformail.html
maildropex.html: libs/maildrop/maildropex.html
	cp -f $(srcdir)/libs/maildrop/maildropex.html maildropex.html
maildropfilter.html: libs/maildrop/maildropfilter.html
	cp -f $(srcdir)/libs/maildrop/maildropfilter.html maildropfilter.html
maildropgdbm.html: libs/maildrop/maildropgdbm.html
	cp -f $(srcdir)/libs/maildrop/maildropgdbm.html maildropgdbm.html
makemime.html: libs/rfc2045/makemime.html
	cp -f $(srcdir)/libs/rfc2045/makemime.html makemime.html
reformime.html: libs/rfc2045/reformime.html
	cp -f $(srcdir)/libs/rfc2045/reformime.html reformime.html

# Copyright 2000 Double Precision, Inc.  See COPYING for
# distribution information.

# Need to add sysconftoolize to package:

EXTRA_DIST += sysconftool

# NOTE: when using a complex directory hierarchy it is only necessary to
# include sysconftool in the top level directory, and it is not necessary
# to add sysconftool to EXTRA_DIST in subdirectories.

SYSCONFFILES=$(sysconf_DATA)

# SYSCONFFILES should be all of your .dist files, this is a default.
# You can also change install-configure-local target below, instead.

# If -- in a complex directory hierarchy -- a certain subdirectory does not
# need install-configure, leave it defined as an empty rule to prevent
# sysconftoolize from appending another form template to this Makefile.am

install-configure:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		( case "$$subdir" in .) continue;; \
			*) target=install-configure;; esac ; \
		cd $$subdir ; $(MAKE) install-configure DESTDIR=$(DESTDIR) 2>/dev/null) \
			; done ; exit 0
	$(MAKE) install-configure-local DESTDIR=$(DESTDIR)

# Modify the following target as appropriate:

install-configure-local:
	@list="$(SYSCONFFILES)"; \
		for file in $$list; do \
		file="$(DESTDIR)$(sysconfdir)/$$file" ; \
		case "$$file" in *.dist) \
			@SYSCONFTOOL@ $$file;; esac ; done
upload-beta:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	~/src/sftools/release.pl mrsam $(SFPASSWD) courier imap-devel `echo @VERSION@|sed 's/.*\(........\)/\1/'` @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig

upload-dist:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	set -e; v="@PACKAGE@/@VERSION@/`date +%Y%m%d%H%M%S`"; git tag -s $$v -m 'Tag version @VERSION@'; cd libs; git tag -s $$v -m 'Tag version @VERSION@'
	socksify ~/src/sftools/release.pl mrsam $(SFPASSWD) courier imap @VERSION@ @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig

dollar=$$
id=$(dollar)Id

WWW=$$HOME/www/www.courier-mta.org

htmldoc: INSTALL.html libs/imap/README.html $(MAILDROPHTML) libs/maildrop/maildroptips.html \
	libs/maildir/README.maildirquota.html libs/maildir/README.sharedfolders.html \
	libs/maildir/README.imapkeywords.html libs/imap/BUGS.html libs/imap/README.proxy.html libs/imap/README.html
	for f in $^; do xsltproc --nonet $(srcdir)/imap-format.xsl $$f | perl -ne 's~\$(id):.*\$$~\$(id):'"$$h"'\$$~; print' >$(WWW)/imap/`basename $$f`; done
	rm -rf doc
	mkdir doc
	xsltproc -o doc/ $(srcdir)/imap.xsl $(srcdir)/imap.xml
	for f in doc/*.html; do \
		echo $$f; \
		tidy -i -asxml --doctype transitional \
			--input-encoding utf8 --output-encoding ascii \
			-q --tidy-mark no $$f >$$f.tmp; \
		xsltproc --nonet $(WWW)/xsl/postproc.xsl \
			$$f.tmp > $$f.tmp2; \
		xsltproc --nonet $(srcdir)/imap-format.xsl \
			 $$f.tmp2 >$$f; \
		rm -f $$f.tmp*; done
	mv -f doc/*.html $(WWW)/imap
	cp -f courier-imap.png bg.png $(WWW)/imap
	cp manpage.css html
	rm -rf doc

AUTOMAKE_OPTIONS=dist-bzip2