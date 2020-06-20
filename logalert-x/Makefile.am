@SET_MAKE@
ACLOCAL_AMFLAGS = -I m4
version=@VERSION@
release=@release@
COFLAGS=-z+05:30
DEFS = @DEFS@ -fno-strict-aliasing -Wno-implicit-function-declaration -Wno-unused-function -Wno-return-type
INSTALL_PROGRAM = ${INSTALL}          -c
install_sh_PROGRAM = $(install_sh)    -c
INSTALL_SCRIPT = ${INSTALL}           -c
install_sh_SCRIPT = $(install_sh)     -c

LIBS = @LEXLIB@ @LIBPCRE_LIBS@

bin_PROGRAMS=logclient incrmesg logalert showbytes
sbin_PROGRAMS=logsrv rpclog

man_MANS = logalert.8 logsrv.8 rpclog.8 incrmesg.1 logclient.1 showbytes.1
doc_DOCS = COPYING README.md

MOSTLYCLEANFILES=lex.c parser.h parser.c logalert_perm_list debian/Makefile

noinst_SCRIPTS = logalert.spec logalert-rpmlintrc

DISTCLEANFILES = *~

SYSCONFDIR = @sysconfdir@
sysconf_DATA = logalert_perm_list

AM_LFLAGS  = -Pc
AM_YFLAGS  = -d

edit = sed \
	-e 's,@indimaildir\@,$(prefix),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@DESTDIR\@,$(DESTDIR),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@HOST\@,$(host),g' \
	-e 's,@DATE\@,$(DATE),g' \
	-e 's,@prefix\@,$(prefix),g'

logalert_SOURCES = lex.c utils.c log.c parser.y process.c monitor.c action.c sig.c user.c main.c 
logalert_LDADD = $(LIBS)

logclient_SOURCES=logclient.c common.c tls.c
logclient_LDADD = $(LIB_INDIMAIL) $(LIB_SSL) $(LIB_QMAIL)

logsrv_SOURCES=logsrv.c common.c tls.c
logsrv_LDADD = $(LIB_INDIMAIL) $(LIB_SSL) $(LIB_QMAIL) $(LIB_RPC)

rpclog_LDADD=$(LIB_RPC)

incrmesg_SOURCES=incrmesg.c common.c
incrmesg_LDADD = $(LIB_INDIMAIL)

showbytes_SOURCES=showbytes.c

#$(am__skiplex) $(SHELL) $(YLWRAP) $< $(LEX_OUTPUT_ROOT).c $@ -- flex
lex.c: lex.l parser.h
	 $(LEX) $(LFLAGS) -o$@ $^

parser.h: parser.c
logalert-rpmlintrc: logalert-rpmlintrc.in
	$(edit) logalert-rpmlintrc.in > $@
logalert.spec: logalert.spec.in ChangeLog conf-version conf-release
	(cat $@.in;./catChangeLog) | $(edit) > $@
logsrv.8: logsrv.man
	$(edit) logsrv.man > $@
logclient.1: logclient.man
	$(edit) logclient.man > $@
debian/Makefile: debian/Makefile.in
	$(edit) $@.in > $@
logalert_perm_list: logalert_perm_list.in
	$(edit) $@.in > $@
install-data-hook:
	$(mkinstalldirs)         $(DESTDIR)$(SYSCONFDIR)
	$(INSTALL_DATA) logalert_perm_list $(DESTDIR)$(SYSCONFDIR)/logalert_perm_list  || exit 1