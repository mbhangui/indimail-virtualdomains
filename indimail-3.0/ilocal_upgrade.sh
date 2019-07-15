#!/bin/sh
# $Log: ilocal_upgrade.sh,v $
# Revision 2.25  2019-06-17 18:15:33+05:30  Cprogrammer
# update with mysql_lib control file with either libmsyqlclient or libmariadbclient
#
# Revision 2.24  2019-06-07 19:21:49+05:30  Cprogrammer
# set mysql_lib control file
#
# Revision 2.23  2018-03-25 22:19:07+05:30  Cprogrammer
# removed chmod of variables directory as it is redundant now with read perm for indimail group
#
# Revision 2.22  2018-02-18 22:17:58+05:30  Cprogrammer
# pass argument to do_post_upgrade()
#
# Revision 2.21  2018-02-18 21:42:17+05:30  Cprogrammer
# update cron entries
#
# Revision 2.20  2018-01-09 12:11:40+05:30  Cprogrammer
# removed indimail-mta specific code
#
# Revision 2.19  2018-01-08 10:52:23+05:30  Cprogrammer
# fixed typo
#
# Revision 2.18  2017-12-30 21:50:01+05:30  Cprogrammer
# create environment variable DISABLE_PLUGIN
#
# Revision 2.17  2017-12-26 23:34:03+05:30  Cprogrammer
# update control files only if changed
#
# Revision 2.16  2017-11-22 22:37:32+05:30  Cprogrammer
# logdir changed to /var/log/svc
#
# Revision 2.15  2017-11-06 21:45:42+05:30  Cprogrammer
# fixed upgrade script for posttrans
#
# Revision 2.14  2017-10-22 19:03:41+05:30  Cprogrammer
# overwrite LOGFILTER only if it is already set
#
# Revision 2.13  2017-10-22 18:57:27+05:30  Cprogrammer
# fixed rcs id
#
# Revision 2.12  2017-10-22 15:27:23+05:30  Cprogrammer
# remove redundant indimail.service during upgrade
#
# Revision 2.11  2017-04-21 10:24:04+05:30  Cprogrammer
# run upgrade script only on post
#
# Revision 2.10  2017-04-16 19:55:04+05:30  Cprogrammer
# changed qmail-greyd path to /usr/sbin
#
# Revision 2.9  2017-04-14 00:16:35+05:30  Cprogrammer
# added permissions for roundcube to accces certs, spamignore
#
# Revision 2.8  2017-04-11 03:44:57+05:30  Cprogrammer
# documented steps involved in upgrade
#
# Revision 2.7  2017-04-05 14:11:14+05:30  Cprogrammer
# upgraded soft mem to 536870912
#
# Revision 2.6  2017-04-03 15:56:50+05:30  Cprogrammer
# create FIFODIR
#
# Revision 2.5  2017-03-31 21:17:37+05:30  Cprogrammer
# fix DEFAULT_HOST, QMAILDEFAULTHOST, envnoathost, defaulthost settings
#
# Revision 2.4  2017-03-30 23:29:04+05:30  Cprogrammer
# added chgrp
#
# Revision 2.3  2017-03-29 19:31:59+05:30  Cprogrammer
# added rsa2048.pem, dh2048.pem
#
# Revision 2.2  2017-03-29 14:45:42+05:30  Cprogrammer
# fixed upgrade for v2.1
#
# Revision 2.1  2017-03-28 19:21:18+05:30  Cprogrammer
# upgrade script for indimail 2.1
#
#
# $Id: ilocal_upgrade.sh,v 2.25 2019-06-17 18:15:33+05:30 Cprogrammer Exp mbhangui $
#
PATH=/bin:/usr/bin:/usr/sbin:/sbin
chgrp=$(which chgrp)
chmod=$(which chmod)
chown=$(which chown)
rm=$(which rm)
cp=$(which cp)

check_update_if_diff()
{
	val=`cat $1 2>/dev/null`
	if [ ! " $val" = " $2" ] ; then
		echo $2 > $1
	fi
}

check_libmysqlclient_lib()
{
	if [ -f /etc/indimail/control/mysql_lib ] ; then
		mysqllib=`cat /etc/indimail/control/mysql_lib 2>/dev/null`
		if [ -f $mysqllib ] ; then
			return 0
		fi
	fi
	# upgrade MYSQL_LIB for dynamic loading of libmysqlclient
	prev_num=0
	for i in `ls -t /usr/lib*/libmysqlclient.so.*.*.* 2>/dev/null`
	do
		file=`basename $i`
		num=`echo $file | cut -d. -f3`
		if [ $num -gt $prev_num ] ; then
			prev_num=$num
			mysqllib=$i
		fi
	done
	if [ -z "$mysqllib" -a -f /etc/debian_version ] ; then
	# upgrade MYSQL_LIB for dynamic loading of libmysqlclient
	prev_num=0
	for i in `ls -t /usr/lib/*-linux-gnu/libmariadbclient.so.*.*.* 2>/dev/null`
	do
		file=`basename $i`
		num=`echo $file | cut -d. -f3`
		if [ $num -gt $prev_num ] ; then
			prev_num=$num
			mysqllib=$i
		fi
	done
	fi
	#
	# this is crazy. Both mariadb and oracle are breaking things
	# around
	# MariaDB-shared, mariadb-connector-c
	# MariaDB-Compat
	# mysql-community-libs
	#
	if [ -z "$mysqllib" ] ; then
		if [ -d /etc/debian_version ] ; then
			dir="/usr/lib/*-linux-gnu/libmysqlclient.so.*.*.* \
				/usr/lib/*-linux-gnu/libmariadbclient.so.*.*.* \
				/usr/lib*/mysql/libmariadbclient.so.*.*.*"
		else
			dir="/usr/lib*/libmariadb.so.* \
				/usr/lib*/libmysqlclient.so.*.*.* \
				/usr/lib*/mysql/libmysqlclient.so.*.*.*"
		fi
		for i in `ls -t $dir 2>/dev/null`
		do
			file=`basename $i`
			num=`echo $file | cut -d. -f3`
			if [ $num -gt $prev_num ] ; then
				prev_num=$num
				mysqllib=$i
			fi
		done
	fi
	if [ -n "$mysqllib" -a -f $mysqllib ] ; then
		check_update_if_diff /etc/indimail/control/mysql_lib $mysqllib
	fi
	return 0
}

do_post_upgrade()
{
date
echo "Running $1 - $Id: ilocal_upgrade.sh,v 2.25 2019-06-17 18:15:33+05:30 Cprogrammer Exp mbhangui $"
# Fix CERT locations
for i in /service/qmail-imapd* /service/qmail-pop3d* /service/proxy-imapd* /service/proxy-pop3d*
do
	check_update_if_diff $i/variables/TLS_CACHEFILE /etc/indimail/certs/couriersslcache
	check_update_if_diff $i/variables/TLS_CERTFILE /etc/indimail/certs/servercert.pem
done
for i in /service/qmail-poppass* /service/indisrvr.*
do
	check_update_if_diff $i/variables/CERTFILE /etc/indimail/certs/servercert.pem
done

if [ -f /service/fetchmail/variables/QMAILDEFAULTHOST ] ; then
	$rm -f /service/fetchmamil/variables/QMAILDEFAULTHOST
fi
# changed fifo location from /etc/indimail/inquery to /var/indimail/inquery
for i in /service/inlookup.infifo /service/qmail-imapd* /service/qmail-pop3d* \
	/service/qmail-smtpd.25 /service/qmail-smtpd.465 /service/qmail-smtpd.587
do
	if [ -d $i ] ; then
		check_update_if_diff $i/variables/FIFODIR /var/indimail/inquery
	fi
done

# add for roundcube/php to access certs
/usr/bin/getent group apache > /dev/null && /usr/sbin/usermod -aG qmail apache || true
if [ -f /etc/indimail/control/spamignore ] ; then
	$chgrp apache /etc/indimail/control/spamignore
	$chmod 664 /etc/indimail/control/spamignore
fi

# copy updated cron entries
if [ -f /etc/indimail/cronlist.i -a -d /etc/cron.d ] ; then
	diff /etc/indimail/cronlist.i /etc/cron.d/cronlist.i >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		$cp /etc/indimail/cronlist.i /etc/cron.d/cronlist.i
	fi
fi

# upgrade libmysqlclient path in /etc/indimail/control/mysql_lib
check_libmysqlclient_lib
} 

case $1 in
	post|posttrans)
	do_post_upgrade $1
	;;
esac