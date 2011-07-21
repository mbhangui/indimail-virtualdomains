# $Log: svscanboot.sh,v $
# Revision 1.10  2011-07-21 13:18:05+05:30  Cprogrammer
# create lockfile to enable working with systemd
#
# Revision 1.9  2011-05-26 23:37:41+05:30  Cprogrammer
# removed readproctitle
#
# Revision 1.8  2009-06-18 19:47:48+05:30  Cprogrammer
# do wait only for job in background
#
# Revision 1.7  2008-07-25 16:54:33+05:30  Cprogrammer
# set SCANINTERVAL in all cases
# use wait in all cases
#
# Revision 1.6  2008-07-15 19:54:07+05:30  Cprogrammer
# removed /usr/X11R6/bin from path
#
# Revision 1.5  2003-10-12 01:14:37+05:30  Cprogrammer
# added SCANINTERVAL
#
# Revision 1.4  2003-01-05 23:53:11+05:30  Cprogrammer
# skip directories if not present
#
# Revision 1.3  2002-12-28 09:21:30+05:30  Cprogrammer
# added option of specifying multiple directories as command line arguments
#
# Revision 1.2  2002-09-26 20:56:02+05:30  Cprogrammer
# made service directory configurable
#
# $Id: svscanboot.sh,v 1.10 2011-07-21 13:18:05+05:30 Cprogrammer Exp mbhangui $

PATH=QMAIL/bin:/usr/local/bin:/bin:/sbin:/usr/bin:/usr/sbin

exec </dev/null
exec >/dev/null
exec 2>/dev/null

if [ -f  QMAIL/control/scaninterval ] ; then
	SCANINTERVAL=`cat QMAIL/control/scaninterval`
else
	SCANINTERVAL=300
fi
if [ $# -eq 0 ] ; then
	if [ -d /service1 ] ; then
		SERVICEDIR=/service1
	elif [ -d /service2 ] ; then
		SERVICEDIR=/service2
	else
		SERVICEDIR=/service
	fi
else
	SERVICEDIR=$1
fi
QMAIL/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log $SERVICEDIR/.svscan/log
if [ -d /var/lock/subsys ] ; then
	touch /var/lock/subsys/svscan
fi
env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL SCANLOG="" \
	QMAIL/bin/svscan $SERVICEDIR
