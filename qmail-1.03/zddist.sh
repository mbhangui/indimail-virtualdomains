#
# $Log: zddist.sh,v $
# Revision 1.3  2016-06-17 20:19:44+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.2  2004-01-03 13:44:24+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:40:49+05:30  Cprogrammer
# Initial revision
#
#
echo 'Distribution of ddelays for successful deliveries sorted on doneby

Meaning of each line: The first pct% of successful deliveries
all happened within doneby seconds. The average ddelay was avg.
'
(
echo doneby avg pct
LIBEXEC/ddist |sort -n -r
) | QMAIL/bin/columnt
