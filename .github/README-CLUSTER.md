# IndiMail Cluster

File README-CLUSTER, Version 3.2
DATE: Tue Jun  9 01:05:52 IST 2020

IndiMail supports a cluster of servers for a domain. A cluster is a set of servers having the same domain. In other words, a clustered domain is a domain which has been extended across more than one server.

You can either have a normal non-clustered domain or a clustered domain. To achieve high performance needed for high volume servers, IndiMail uses MySQL to store some of the information in MySQL tables. All indimail programs depend on one or more control file to figure out how to extract this information stored in MySQL tables. The control file **host.mysql** is required regardless of whether the domain is clustered or not.

For a clusterd domain you require the additional control files

* host.master
* host.cntrl
* mcdinfo

The **mcdinfo** control file is a special file. If you update it on one server, it automatically gets updated on all servers that are part of the IndiMail cluster. This allows a server and it's users to be immediately know to all servers.

Refer to this diagram for the architecture ![diagram](indimail_arch.png "IndiMail Architecture")

There are five types of servers

Server Type|Description
-----------|-----------
clusterinfo|This is the host which has a MySQL database for storing the location of each user. Location information is hostid and ip address. If this host is going to serve as a pure MySQL database server only, you need not install IndiMail on this server.
Mailstore |A mailstore is a host which has disk space to host user's mailboxes. Each mailstore can have its own set of users. You can call each host a 'node'. IndiMail allows you to extend a domain across multiple mailstores. Each mailstore needs its own MySQL database to store information for its own set of users. You may install a MySQL database on each of the mailstore. You need to install IndiMail on each mailstore. You can also have a non-indimail mailstore like Lotus Notes, M$ Exchange, etc. In such a case, you will have to add the users manually to the clusterinfo database using **hostcntrl(8)** utility.
Relay Server|A relay server acts as an SMTP gateway between the user and the mailstore or between the user and the internet. A relay server can also carry out tasks like virus/spam filtering, maintain your security policy, access lists. You need to install IndiMail on each relay server.
IMAP/POP3 Gateway|These servers run IndiMail's IMAP/POP3 Proxies You need to install IndiMail on each IMAP/POP3 proxy server.
Webmail|This servers run a webmail software (like squirellmail, roundcube, etc). In case your webmail software connects to localhost on the IMAP/POP3 port, you will need to install IndiMail for providing IMAP/POP3 proxy on localhost.

A server can be a Mailstore + Relay Server + Webmail. But you do need one **clusterinfo** server to store information for all **Mailstores**.

IndiMail has been deployed, tested by an ISP to provide service to 6 milllion users using 8 mailstores, 4 servers serving as incoming relay servers, 4 servers serving as webmail (custom PHP code), 3 servers as outoing relay servers and one server with just MySQL installed, serving as the clusterinfo server.

The control files used to figure out the MySQL server storing IndiMail configuration & information depends on whether you are running a clustered or a non-clustered domain. Read below to know more about this.

**Non-Clustered Domain**

A mailstore requires a MySQL database to store its local user infomation. For a non-clustered domain IndiMail needs to connect to a single MySQL database. You can create the control file host.mysql which has the IP address or hostname for the MySQL database. If this file is missing, IndiMail program will use 'localhost' my default.

```
# echo localhost > /etc/indimail/control/host.mysql

localhost : MySQL host to which all programs will connect)
```

NOTE: you can also use

```
host:user:password:socket[:ssl]
or
host:user:password:port[:ssl]

```
format. Here the optional use_ssl is "ssl" or "nossl" for host.mysql (for IndiMail 1.6.9 and above)

```
# echo "localhost:indimail:ssh-1.5-:/var/run/mysqld/mysqld.sock:ssl" > host.mysql
```

**Clustered Domain**

A clustered domain has multiple mailstores configured with the same domain. Each mailstore requires a MySQL database to store its local user information. To have a catalogue of the multiple mailstores, you require an additional MySQL database called the clusterinfo. In a clustered domain setup, each mailstore needs to have the control files host.cntrl and host.master. These control files has the IP address or hostname of the clusterinfo MySQL database. The clusterinfo database also stores each user's mailstore IP. This is required to ensure that mails for a clustered domain reaches the correct mailstore on which the user account exists.

Adding the first Mailstore in a cluster

There are three files required on all hosts serving mailboxes (mailstores) for clustered domain

* host.cntrl
* host.master
* mcdinfo

```
echo hostcntrl_host > /etc/indimail/control/host.mysql

hostcntrl_host - MySQL host to which all programs will connect)

or

echo "hostcntrl_host:user:password:mysql socket/port" > /etc/indimail/control/host.mysql
```

For a clustered domain you need to have two additional files

* host.cntrl
* host.master

Additionally, IndiMail supports the use of Master-Slave architecture in MySQL.

```
echo hostcntrl_master > /etc/indimail/control/host.master (for a clustered domain, the master host)
echo hostcntrl_slave  > /etc/indimail/control/host.cntrl  (for a clustered domain, the slave host)
```

where hostcntrl_master is the IP address of MySQL Master having the host control information.
where hostcntrl_slaver is the IP address of MySQL Slave  having the host control information.

NOTE: you can also use host:user:password:socket or host:user:password:port format for host.mysql, host.cntrl, host.master (for IndiMail 1.6.9 and above)

In case you don't desire a master-slave architecture, hostcntrl_master and hostcntrl_slave can be the same

You also need to have a file called mcdinfo having information for all hosts which will host the domain indimail.org

e.g. run the following command to create a clustered domain with just two mail clusters on hosts 192.168.2.115 and 192.168.2.116. The cluster information for all users will be maintained on the MySQL host at 192.168.2.110. Updating the file on any host updates the file mcdinfo on all hosts. You can use any text editor to edit this file.

```
(
echo "domain   indimail.org                     1"
echo "server   192.168.2.115" # MySQL Server for Mail Store1
echo "mdahost  192.168.2.115" # Mail Store1
echo "port     3306"
echo "use_ssl  1"
echo "database indimail"
echo "user     indimail"
echo "pass     ssh-1.5-"
echo
echo "domain   indimail.org                     1"
echo "server   192.168.2.116" # MySQL Server for Mail Store2
echo "mdahost  192.168.2.116" # Mail Store2
echo "port     3306"
echo "use_ssl  0"
echo "database indimail"
echo "user     indimail"
echo "pass     ssh-1.5-"
echo
) > /etc/indimail/control/mcdinfo
```

The above can also be achieved by executing the following commands

```
% /usr/bin/dbinfo -i -S 192.168.2.115 -D indimail -U indimail -P ssh-1.5- \
    -p 3306 -d indimail.org -m 192.168.2.115
% /usr/bin/dbinfo -i -S 192.168.2.116 -D indimail -U indimail -P ssh-1.5- \
    -p 3306 -d indimail.org -m 192.168.2.116
```

-- NOTE ----------------------------------------------------------------------------

The above example has one MySQL server for both the mailstores for storing the user
information. For better performance you may want to have each mailstore have its own
MySQL server on the mailstore itself. In that case you can execute the following
commands

```
% /usr/bin/dbinfo -i -S 192.168.2.115 -D indimail -U indimail -P ssh-1.5- \
    -p 3306 -d indimail.org -m 192.168.2.115
% /usr/bin/dbinfo -i -S 192.168.2.116 -D indimail -U indimail -P ssh-1.5- \
    -p 3306 -d indimail.org -m 192.168.2.116
```

You can use the command dbinfo -s to show this information.

The cluster information is maintained in the MySQL table dbinfo. You can modify the table dbinfo directly or use the '-i' option to the dbinfo commmand. The immediate next call to 'dbinfo -s' will create the file mcdinfo. In a cluster you need to update mcdinfo on any of the hosts. The entries to mcdinfo on all hosts part of the cluster automatically get updated.

Every host has to be assigned a unique identifier called hostid Each hostid is associated with the IP address of the mail store host. You can use the command vhostid to add, insert, delete or update this information. The hostid for a mailstore can be stored in the conrol file /etc/indimail/control/hostid.  In case this control file is not there, IndiMail will use the gethostid(2) system call to get the hostid. The mapping of hostid of a mailstore to its IP address is maintained in the table host_table.

When you add a user to a host in the cluster, the user gets added to a MySQL table hostcntrl. This table maintains the username, domain and the hostid on which the user's mailbox lies.

The SMTP, IMAP/POP3 processes all look into the tables hostcntrl and host_table to figure out the host which has the user's mailbox. You can specify the -m or -h argument to vadduser to create users on specific hosts.

**Adding a New Mailstore to a Cluster**

1. use vadddomain on the new node to add domain
2. Create the files host.cntrl, host.master in /etc/indimail/control
3. Use dbinfo -i to add entry for the new node
4. assign a hostid to the mailstore (e.g. 1000)
   ```
   % sudo vhostid -i 192.168.2.115 1000
   % sudo vhostid -i 192.168.2.116 1001
   ```

**Adding a New Relay Server/Proxy IMAP/Proxy POP3 Server to a cluster**

1. Create the files host.cntrl, host.master in /etc/indimail/control
2. run dbinfo -s
3. On host which will have SMTP/qmail-send, have the environment variable
   ROUTES=dynamic for qmail-send. i.e

   ```
   # echo dynamic > /service/qmail-send.25/variables/ROUTES
   % svc -d /service/qmail-send.25; svc -u /service/qmail-send.25
   ```

Remember that if your webmail server uses IMAP/POP3, you will need to run imap/pop3 proxy on the standard IMAP/POP3 ports and have the control files host.cntrl and host.master
