# Installing indimail-mini

A indimail-mini installation doesn't have a mail queue. Instead it gives each new
message to a central server through QMQP.

There are three standard applications of indimail-mini:

* At a large site, mail service is centralized on a few hosts. All other hosts are null clients running indimail-mini. Setting up the null clients is easy; see below.
* A firewall sends all incoming messages to an internal gateway. The firewalls run indimail-mini plus mini-smtpd. There's very little code to audit, and none of it runs as root.
* A computer sends mail to a huge mailing list through a better-connected smarthost. The computer runs indimail for its own local deliveries, but it also has ezmlm configured to use a separate indimail-mini installation for the mailing list. QMQP is typically 100 times faster than SMTP here; for example, you can send a 1000-recipient message through a 28.8 modem in about 10 seconds. 

## How do I set up a QMQP server?

Here's how to set up a central server to offer QMQP service to authorized client hosts.

IndiMail includes a QMQP server, qmail-qmqpd. You also need to have tcpserver installed.

first create /etc/indimail/tcp/qmqp.tcp in tcprules format to allow queueing from the authorized hosts. make sure to deny connections from unauthorized hosts. for example, if queueing is allowed from 1.2.3.\*:

```
1.2.3.:allow
:deny
```

Then create /etc/indimail/tcp/qmqp.cdb:

```
$ sudo qmailctl cdb
building /etc/indimail/tcp/tcp.qmqp.cdb:                   [  OK  ]
```

You can change /etc/indimail/tcp/qmqp.tcp and run qmailctl again at any time. Finally qmail-qmqpd to be run under supervise:

```
sudo /usr/sbin/svctool --qmqp=628 --servicedir=/service --maxdaemons=75 --maxperip=25 \
  --qbase=/var/indimail/queue --qcount=5 --qstart=1 --cntrldir=control --localip=0 \
  --fsync --syncdir --memory=104857600 --min-free=52428800
```

628 is the standard TCP port for QMQP.

## How do I install indimail-mini?

A indimail-mini installation is just like a indimail installation, except that it's much easier to set up:

* You don't need MySQL
* You don't need /var/indimail/alias. A indimail-mini installation doesn't do any local delivery.
* You don't need indimail entries in /etc/group or /etc/passwd. indimail-mini runs with the same privileges as the user sending mail; it doesn't have any of its own files.
* You don't need to start anything from your boot scripts. indimail-mini doesn't have a queue, so it doesn't need a long-running queue manager.
* You don't need to add anything to inetd.conf. A null client doesn't receive incoming mail. 

If you install indimail-mini from the indimail repository (rpm, deb or arch packages), you needn't to anything. If you install from sources, here's what you do need:

* The following binaries are required in the path (for source installation only).
  sendmail
  qmail-inject
  irmail
  predate
  datemail
  mailsubj
  qmail-showctl
  srsfilter
  qmail-qmqpc
  qmail-direct
* shared libs that the above binaries reference. You can use the ldd command (or otool -L command on OSX) (for source installation only).
* A single line /usr/sbin/qmail-qmqpc in /etc/indimail/control/defaultqueue/QMAILQUEUE
* symbolic links to /usr/bin/sendmail from /usr/sbin/sendmail and /usr/lib/sendmail;
* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers;
* a copy of /etc/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision. Everything can be shared across hosts except for /etc/indimail/control/idhost.
* Setup QMAILQUEUE environment variable to have qmail-qmqpc called instead of qmail-queue when any client injects mails in the queue (for source installation only).
  ```
  # mkdir /etc/indimail/control/defaultqueue
  # cd /etc/indimail/control/defaultqueue
  # echo /usr/sbin/qmail-qmqpc > QMAILQUEUE
  ```
* All manual pages for the above binaries (not a hard requirement but good for future reference) (for source installation only).

Everything can be shared across hosts except for /etc/indimail/control/idhost.

Remember that users won't be able to send mail if all the QMQP servers are down. Most sites have two or three independent QMQP servers.

Note that users can still use all the qmail-inject environment variables to control the appearance of their outgoing messages.

## What about firewalls?

You won't need most of the indimail-mini programs on a firewall, but you will need mini-smtpd to accept messages through SMTP. You can survive with a tiny configuration:

* /usr/sbin/qmail-qmqpc;
* /usr/sbin/mini-smtpd;
* a symbolic link to qmail-qmqpc from /usr/sbin/qmail-queue;
* the internal gateway's IP address in /etc/indimail/control/qmqpservers;
* the firewall host's name in /etc/indimail/control/me; and
* the list of acceptable domains in /etc/indimail/control/rcpthosts. 

You don't need to worry about setting up redundant QMQP servers here. If the internal gateway is down, mini-smtpd will temporarily reject the message, and the remote client will automatically try again later.

## What about mailing lists?

If you are installing ezmlm-idx or ezmlm from indimail repository, there is nothing you need to do. Both use ezmlm-queue to queue mails and ezmlm-queue takes care of using qmail-qmqpc if it finds a indimail-mini binary install

Here's how to set up ezmlm to send messages to a smarthost through QMQP:

1. Create a /var/indimail-mini directory.
2. Create a /var/indimail-mini/bin directory.
3. Make a symbolic link to /usr/sbin/qmail-qmqpc from /var/indimail-mini/bin/qmail-queue.
4. Put the smarthost's IP address into /etc/indimail/control/qmqpservers.
5. Compile and install ezmlm with /var/indimail-mini in conf-qmail. 

You don't need to worry about setting up redundant QMQP servers here. If the smarthost is down, the message will stay in the local qmail queue and will be retried later. 
