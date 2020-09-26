COFLAGS=-z+05:30
prefix=/usr
sysconfdir=/etc/indimail
shareddir=/usr/share/indimail
libexecdir=/usr/libexec/indimail
mandir=/usr/share/man
logdir=/var/log/svc
pam_confdir=/etc/pam.d
servicedir=/service
version=$(shell cat conf-version)
release := $(shell cat conf-release 2>/dev/null||echo "1.1")
nssd_version=$(shell cat ../nssd-x/conf-version)
pam_multi_version=$(shell cat ../pam-multi-x/conf-version)
DATE=$(shell date)
# if you change edit, change it in debian/Makefile.in too
edit = sed \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@pam_confdir\@,$(pam_confdir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@nssd_version\@,$(nssd_version),g' \
	-e 's,@pam_multi_version\@,$(pam_multi_version),g' \
	-e 's,@DATE\@,$(DATE),g'

all: indimail-auth.spec debian/Makefile

indimail-auth.spec: indimail-auth.spec.in catChangeLog Makefile \
conf-version conf-release doc/ChangeLog ../nssd-x/conf-version ../pam-multi-x/conf-version
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
debian/Makefile: debian/Makefile.in conf-version conf-release \
../nssd-x/conf-version ../pam-multi-x/conf-version
	(cat $@.in; sh ./catChangeLog) | $(edit) > $@
	$(edit) $@.in > $@