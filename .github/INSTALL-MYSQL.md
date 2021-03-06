# MySQL Installation Notes

Do not read this without reading [INSTALL.md](INSTALL-indimail.md) This is an important file. Most users in my experience have got stuck with MySQL installation and configuration. Following are the three most common issues that users have experienced installing IndiMail If you have done a RPM/DEB/yum/dnf/dpkg/apt-get installation, all these steps are automatically taken care by the post-installation scripts. However, in some cases, even the post-install scripts fail due to frequent incompatible changes made by folks at Oracle & Mariadb.

1. MySQL version : As far as possible use mysql 5.7.x or greater. If you use an unsupported version of MySQL, you might face the following problems
   MySQL fails to startup under supervise. This happens when svctool fails to create a default MySQL database for IndiMail
   during RPM installation or when you run the command

   ```
   svctool --config=mysqldb ...
   ```
   However, this issue is not a big deal as each and every IndiMail program has been coded to create the required MySQl table when the table is accessed.

2. If you have /etc/my.cnf, replace socket=/var/lib/mysql/mysql.sock with socket=/var/run/mysqld/mysqld.sock 
   NOTE: If you change the socket/port then you have to edit /etc/indimail/control/host.\*
   and /etc/indimail/indimail.cnf to change the socket path / port

3. If the user root in mysql does not have a password. This will prevent anyone on the system to connect to MySQL using a password.

## STEP 1  MySQL Startup ##

Create Supervise Directory for starting MySQL. If you already have a running MySQL
server and want to continue to have MySQL started through the system boot scripts,
proceed to STEP 2 below

However if you want to run MySQL under supervise. You will have to disable startup of MySQL
in your system boot scripts.
    
Remove mysqld startup in boot scripts

```
# chkconfig mysqld off (for Ubuntu : update-rc.d -f mysql remove)
# /etc/init.d/mysqld stop
```

For systems using systemctl

a) For mysql-community-server (oracle)

```
# /bin/systemctl stop mysqld.service
# /bin/systemctl disable mysqld.service
```

b) For mariadb-server

```
# /bin/systemctl stop mariadb.service
# /bin/systemctl disable mariadb.service
```

For systems using upstart

```
# echo "manual" > /etc/init/mysqld.override
# /sbin/initctl stop mysqld
```

On FreeBSD

```
# service mysql stop
# service mysql disable
```

On Darwin Mac OSX

```
# launchctl unload /Library/LaunchDaemons/com.oracle.oss.mysql.mysqld.plist
```

Some operating systems also use '/etc/rc.local' or '/etc/init.d/boot.local' to start additional services on startup. You must remove any invocation of mysqld_safe from these scripts.

Create service in supervise to start mysqld

```
# /usr/sbin/svctool --mysql=3306 --servicedir=/service \
    --mysqlPrefix=/usr --databasedir=/var/indimail/mysqldb \
    --config=/etc/indimail/indimail.cnf --default-domain=`uname -n`
```

## STEP 2  MySQL Database Creation ##

Create MySQL Database. You have to either run option a) or option b) depending on whether you want to use an already configured MySQL server on your host or you want svctool to configure a new MySQL server on your host.

a) For creation of a brand new MySQL database, create MySQL Options file /etc/indimail/indimail.cnf with the following line

```
sql_mode="NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES"
```

Create mysql service under supervise

```
# /usr/sbin/svctool --config=mysql --mysqlPrefix=/usr \
    --mysqlport=3306 --mysqlsocket=/var/run/mysqld/mysqld.sock
```

Create a new MySQL database

```
# /usr/sbin/svctool --config=mysqldb --mysqlPrefix=/usr \
    --databasedir=/var/indimail/mysqldb --default-domain=`uname -n` \
    --base_path=/home/mail
```

Create a link to IndiMail's MySQL config file

```
# ln -sf /etc/indimail/indimail.cnf /etc/my.cnf.d/indimail.cnf
```

b) For creation of a database in an existing running copy of MySQL on your server. The below are needed only if you are going to use the existing MySQL database).  You can use either Method 1 or Method 2

**Method 1 - Using svctool**

```
# /usr/sbin/svctool --config=mysqldb --mysqlPrefix=/usr \
    --databasedir=/var/indimail/mysqldb --default-domain=`uname -n` \
    --base_path=/home/mail --stdout | mysql -u user -ppassword
```

Create a link to existing MySQL config file

```
# ln -sf /etc/my.cnf /etc/indimail.cnf
```

NOTE: user and password are the userid and password of a privileged user in your existing MySQL Database)

**Method 2 - Using mysql client**

```
% mysql -u user -ppassword
mysql> CREATE USER indimail identified by 'ssh-1.5-';
mysql> CREATE USER mysql    identified by '4-57343';
mysql> CREATE USER admin    identified by 'benhur20';
mysql> CREATE USER repl     identified by 'slaveserver';
mysql> GRANT ALL on \*.\* to 'mysql';
mysql> GRANT USAGE ON \*.\* TO 'mysql' WITH GRANT OPTION; # for mysqld >= 8
mysql> GRANT SELECT,CREATE,ALTER,INDEX,INSERT,UPDATE,DELETE,CREATE TEMPORARY TABLES, \
       LOCK TABLES ON indimail.\* to 'indimail';
mysql> GRANT RELOAD,SHUTDOWN,PROCESS on \*.\* to admin;
mysql> GRANT REPLICATION SLAVE on \*.\* to repl;
```

Create a link to existing MySQL config file

```
# ln -sf /etc/indimail/indimail.cnf /etc/my.cnf.d/indimail.cnf
```

NOTE: user and password are the userid and password of a privileged user in your existing MySQL Database)
NOTE: If you are using mysql-community-server 8.x and you have old MySQL clients connecting to this db, then you might want to do this

```
mysql> ALTER USER 'indimail'@'%' IDENTIFIED WITH mysql_native_password BY 'ssh-1.5-';
```

The information for IndiMail to connect to IndiMail is maintained in the file

```
/etc/indimail/control/host.mysql    # for non-clustered setup

and

/etc/indimail/control/host.cntrl    # for clustered setup
/etc/indimail/control/host.master   # for clustered setup
```

The format for this file is

```
mysql_host:mysql_user:mysql_pass:mysql_socket[:use_ssl]

or

mysql_host:mysql_user:mysql_pass:mysql_port[:use_ssl]
```

where use\_ssl is the keyword "ssl" or "nossl", or it can be omitted entirely.
