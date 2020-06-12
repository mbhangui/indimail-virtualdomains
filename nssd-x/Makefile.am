@SET_MAKE@
AUTOMAKE_OPTIONS = foreign no-dependencies

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

DEFS=@DEFS@ -Dlint -I. -I/usr/include/qmail @auth_inc@
COMMONSOURCES= pwd.c spwd.c grp.c sock.c misc.c util.c load_mysql.c

libnss_nssddir=@libdir@
libnss_nssd_LTLIBRARIES = libnss_nssd.la
libnss_nssd_la_SOURCES=$(COMMONSOURCES) 
libnss_nssd_la_LIBADD = $(LIB_QMAIL)
libnss_nssd_la_DEPENDENCIES = common.h config.h nsvs.h
libnss_nssd_la_LDFLAGS = -module -version-info @LIBVER@:0:0 -export-symbols $(srcdir)/@OS@.sym

nssdsbindir=@prefix@/sbin
nssdsbin_PROGRAMS = nssd
nssd_SOURCES = nssd.c nssd_misc.c util.c config.c load_mysql.c
nssd_CFLAGS  = -Wall @auth_inc@ @PTHREAD_CFLAGS@
nssd_LDFLAGS = @auth_libs@ @PTHREAD_CFLAGS@ @PTHREAD_LIBS@
nssd_LDADD = 
nssd_DEPENDENCIES = config.h

nssdlibexecdir=@libexecdir@
nssdlibexec_PROGRAMS = check_getpw
check_getpw_SOURCES=check_getpw.c
check_getpw_LDADD=
man_MANS=nssd.8 nssd.conf.5
doc_list = COPYING AUTHORS README

noinst_HEADERS = nssd.h

MOSTLYCLEANFILES=nssd.8 nssd.conf.5

pwd.$(OBJEXT): nsvs.h common.h config.h
grp.$(OBJEXT): nsvs.h common.h config.h
misc.$(OBJEXT): nsvs.h common.h config.h
sock.$(OBJEXT): nsvs.h common.h config.h
spwd.$(OBJEXT): nsvs.h common.h config.h
util.$(OBJEXT): common.h config.h
load_mysql.$(OBJEXT): config.h load_mysql.c

nssd-nssd.$(OBJEXT): nssd.h common.h config.h
nssd-config.$(OBJEXT): nssd.h common.h config.h
nssd-nssd_misc.$(OBJEXT): nssd.h common.h config.h
nssd-util.$(OBJEXT): common.h config.h

nssd.8: nssd.9
	cat $^ | sed s}INDIMAIL}$(prefix)}g > $@
nssd.conf.5: nssd.conf.9
	cat $^ | sed s}INDIMAIL}$(prefix)}g > $@

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

install-data-local:  COPYING README AUTHORS
	$(mkinstalldirs)  $(DESTDIR)@datarootdir@/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) $$f $(DESTDIR)@datarootdir@/doc/$$f.nssd || exit 1 ;\
	done