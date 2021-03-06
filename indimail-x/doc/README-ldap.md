# Creating a shared Address Book for your server

## Install openldap

```
% sudo yum install openldap openldap-servers openldap-clients
```

## Configure openldap

You need to start slapd to implement openldap. slapd uses configurion
file /etc/openldap/slapd.conf

The following slapd.conf file contains the basic configurations required to establish a
shared address book on a secure network, however there are no access controls yet defined;
security is covered later on. The encrypted root password (rootpw) should be substituted where
necessary. You can use slappasswd to generate the password

```
% slappasswd -s secret
{SSHA}gDPX3cS87+B31mAF5zHCGtEJBYSuqrN/

#
# See slapd.conf(5) for details on configuration options.
# This file should NOT be world readable.
#
include    /etc/openldap/schema/corba.schema
include    /etc/openldap/schema/core.schema
include    /etc/openldap/schema/cosine.schema
include    /etc/openldap/schema/duaconf.schema
include    /etc/openldap/schema/dyngroup.schema
include    /etc/openldap/schema/inetorgperson.schema
include    /etc/openldap/schema/java.schema
include    /etc/openldap/schema/misc.schema
include    /etc/openldap/schema/nis.schema
include    /etc/openldap/schema/openldap.schema
include    /etc/openldap/schema/ppolicy.schema
include    /etc/openldap/schema/collective.schema

#######################################################################
# ldbm and/or bdb database definitions
#######################################################################

database    bdb
suffix      "o=indimail.org"
checkpoint  1024 15
rootdn      "cn=Manager,o=indimail.org"
rootpw      {SSHA}gDPX3cS87+B31mAF5zHCGtEJBYSuqrN/
# Cleartext passwords, especially for the rootdn, should
# be avoided.  See slappasswd(8) and slapd.conf(5) for details.
# Use of strong authentication encouraged.
# rootpw      secret
# rootpw      {crypt}ijFYNcSNctBYg

# The database directory MUST exist prior to running slapd AND 
# should only be accessible by the slapd and slap tools.
# Mode 700 recommended.
directory    /var/lib/ldap

# Sample access control policy:
#    Root DSE: allow anyone to read it
#    Subschema (sub)entry DSE: allow anyone to read it
#    Other DSEs:
#    Allow self write access
#    Allow authenticated users read access
#    Allow anonymous users to authenticate
#    Directives needed to implement policy:
# access to dn.base="" by * read
# access to dn.base="cn=Subschema" by * read
# access to *
#    by self write
#    by users read
#    by anonymous auth
#
# if no access controls are present, the default policy
# allows anyone and everyone to read anything but restricts
# updates to rootdn.  (e.g., "access to * by * read")
#
# rootdn can always read and write EVERYTHING!

access to dn.subtree="ou=addressbook,o=indimail.org"
       by anonymous auth
       by self write
       by users read
access to dn.subtree="ou=users,o=indimail.org"
       by anonymous read
       by self write
       by users read
access to *
       by anonymous auth
       by self read
       by users read

# Indices to maintain for this database
index objectClass                       eq,pres
#index ou,cn,mail,surname,givenname      eq,pres,sub
#index uidNumber,gidNumber,loginShell    eq,pres
#index uid,memberUid                     eq,pres,sub
#index nisMapName,nisMapEntry            eq,pres,sub
```

Make user that /etc/openldap/slapd.conf is owned by ldap and has write permissions for
ldap user.

As of now I prefer openldap using slapd.conf and not slapd.d for configuration.

```
% sudo /bin/rm -r /etc/openldap/slapd.d
```

## slapd Startup

My favourite method happens to be using djb's supervise and hence is one of the
core compoment of the IndiMail package

Create the run files in /tmp temporarily

```
% cat /tmp/run1
#!/bin/sh
exec /usr/sbin/slapd -u ldap -f /etc/openldap/slapd.conf -d 0 2>&1

% cat /tmp/run2 
#!/bin/sh
exec /var/indimail/bin/setuidgid qmaill \
/var/indimail/bin/multilog t /var/log/indimail/slapd.389
```

create /service/.slapd.389 so that svscan does not discover this new service yet

```
% sudo mkdir -p /service/.slapd.389/log
% sudo mv /tmp/run2 /service/.slapd.389/log/run
% sudo mv /tmp/run1 /service/.slapd.389/run
% sudo chmod +x /service/.slapd.389/run /service/.slapd.389/log/run
```

rename .slapd.389 to slapd.389 and wait for svscan to discover and start slapd

```
% sudo mv /service/.slapd.389 /service/slapd.389

% /var/indimail/bin/svstat /service/slapd.389
/service/slapd.389/: up (pid 4069) 4 seconds
```

## Access Control

The standard access controls for the server defines that everyone can read the directory
entries, but only the manager (administrator) can write to the directories. To add the LDIF
file the manager is authenicating on the command line with the
"-D 'cn=Manager,o=indimail.org' -W" string.

The section below details the access controls based on the users authentication and basic
anonymous access. The default access controls (below) have been defined to deny everyone
access, however people are allowed to bind to the server to authenticate. All authenticated
users are allowed to change their own details, and all of the entries in the
"ou=addressbook,o=indimail.org" directory; anonymous access it disallowed.

```
#disallow bind_anon
access to dn.subtree="ou=addressbook,o=indimail.org"
       by anonymous auth
       by self write
       by users read
access to *
       by anonymous auth
       by self read
       by users read
```

An access control list may be prone to user syntax errors and will not be accepted by the LDAP
server, so the configuration should be tested before it is loaded.

`% /etc/init.d/slapd configtest`

If the configuration passes integrity testing, the server can be restarted.

`% /var/indimail/bin/svc -u /service/slapd.389`

The new security access controls now prevent unauthorised access to the directory service, so
simple user objects must be prepared that will allow people to authenticate with the server.

The user objects will be imported into the LDAP server using an LDIF file. Remember that
everything in an LDIF file is human readable so plain text passwords are a VERY BAD idea,
especially if you are following this guide for an organisation; no plain text passwords please.

The slappasswd application can be used to create a hashed value of a users password, these are
saved to store in a text file. This does not mean they are completely safe, it just means they
can not be easily read. An attacker can still subject the password value to a brute force
attack, but it would take them an awfully long time. Physical security is still important.

```
% slappasswd -s manny
{SSHA}HEarGJkMA9WqFyg9YfNzQHQTYsIcqEtm
```

The default algorithm for the hashed password is SSHA, this can be changed at the command line
to other formats; the default type (SSHA) is recommended.

```
% slappasswd -h {MD5} -s manny
{MD5}j+eKwOqr8vR0sN46lo4WXg==
```

AddressBook Entries
-------------------

Information can be imported and exported into an LDAP directory service using the LDAP Data
Interchange Format (LDIF) as defined in RFC2849. An LDIF file specifies the contents of a
directory entry in a human readable text format, this allows quick manipulation of a file to
re-import similar entries into the directory.

Now that the LDAP server has been configured and is running, we can conduct a simple search
of the naming context to see our directory information before we start to import our entries.
The "namingContexts" should be similar to the example below.

```
% ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
# extended LDIF
#
# LDAPv3
# base <> with scope base
# filter: (objectclass=*)
# requesting: namingContexts

dn:
namingContexts: o=indimail.org

# search result
search: 2
result: 0 Success

# numResponses: 2
# numEntries: 1
```

The following LDIF file will create the hierarchical directory service structure that we will
be using for indimail's address book. The first entry is that of the base directory and the
second entry is for the Manager's (administrator) account. The last two entries are the two
organisational units that we will use to store the authorised users (for adding security later)
and the address book entries.

The bolded entries should be changed to suit your configuration requirements.

```
% cat addressbook.ldif
# Domain entry
dn: o=indimail.org
o: indimail
dc: indimail
objectclass: dcObject
objectclass: organization

# Manager entry
dn: cn=Manager,o=indimail.org
cn: Manager
objectclass: organizationalRole

# Users
dn: ou=users,o=indimail.org
ou: users
objectClass: top
objectClass: organizationalUnit

# Addressbook entry
dn: ou=addressbook,o=indimail.org
ou: addressbook
objectClass: top
objectClass: organizationalUnit

% ldapadd -x -D 'cn=Manager,o=indimail.org' -W -f addressbook.ldif
Enter LDAP Password:
adding new entry "o=indimail.org"
adding new entry "cn=Manager,o=indimail.org"
adding new entry "ou=users,o=indimail.org"
adding new entry "ou=addressbook,o=indimail.org"
```

The following LDAP search is requesting a listing of all entries starting from the
base "o=indimail.org". This should return all of the entries that where added in the
previous step.

```
% ldapsearch -x -b 'o=indimail.org' '(objectclass=*)'

# indimail.com
dn: o=indimail.org
objectClass: top
objectClass: dcObject
objectClass: organization
o: Home LDAP Network
dc: indimail

# Manager, indimail.com
dn: cn=Manager,o=indimail.org
objectClass: organizationalRole
cn: Manager

# users, indimail.com
dn: ou=users,o=indimail.org
ou: users
objectClass: top
objectClass: organizationalUnit

# addressbook, indimail.com
dn: ou=addressbook,o=indimail.org
ou: addressbook
objectClass: top
objectClass: organizationalUnit
```

Now that we have defined and imported our directory scheme, we can create user entries
to populate the addressbook. The following is a simple example LDIF entry for a contact.

The first line (dn:) designates where about in the directory the entry will belong when its
imported, this should be changed to suit your needs.

```
% cat newcontact.ldif
dn:uid=mbhangui,ou=addressbook,o=indimail.org
uid: mbhangui
cn: Manvendra Bhangui
gn: Manvendra
sn: Bhangui
o: Coconut Farms Pvt. Ltd.
l: Mandaivelli
street: #94 Banana Republic
st: TN
postalCode: 600028
pager: +91 44 5555 xxxx
homePhone: +91 44 5555 xxxx
telephoneNumber: +91 44 5555 xxxx
facsimileTelephoneNumber: +91 44 5555 xxxx
mobile: +91 99401xxxxx
mail: m.bhangui@gmail.com
objectClass: top
objectClass: inetOrgPerson
userPassword: {MD5}j+eKwOqr8vR0sN46lo4WXg==
```

The contents of the LDIF file can be added into the directory service using the "ldapadd"
command below.

```
% ldapadd -x -D 'cn=Manager,o=indimail.org' -W -f newcontact.ldif
Enter LDAP Password:
adding new entry "uid=mbhangui,ou=addressbook,o=indimail.org"
```

You can now use ldapsearch with authentication to test

```
% ldapsearch -x -b 'o=indimail.org' \
    -D "uid=mbhangui,ou=AddressBook,o=indimail.org" '(objectclass=*)' \
    -s sub -w manny
```

If you do not require an address book and just require a basic user object to use for
authenticaton, a basic user object can be created and imported into the LDAP server.
This file uses the "UID" (User ID) string to distinguish the object and the contents are
all that we need to create a basic authentication mechanism.

It should also be noted that this object is stored in the "users" organisational unit, which
is located outside of the address book directory.

```
% cat useraccount.ldif 
dn:uid=mbhangui,ou=users,o=indimail.org
uid: mbhangui
o:indimail.org
userPassword: {MD5}j+eKwOqr8vR0sN46lo4WXg==
objectClass: top
objectClass: account
objectClass: simpleSecurityObject
```

```
% ldapadd -x -D 'cn=Manager,o=indimail.org' -W -f useraccount.ldif
Enter LDAP Password:
adding new entry "uid=mbhangui,ou=users,o=indimail.org"
```

For mbhangui to authenticate to the server, one needs to pass
"uid=mbhangui,ou=users,o=indimail.org" as username along with the the plain text value of
password, the hashed value is only for storage purposes. 

```
% ldapsearch -x -b 'ou=addressbook,o=indimail.org' \
      -D "uid=mbhangui,ou=users,o=indimail.org" '(objectclass=*)' -s sub -w manny
```

NOTE: You can use ldap-checkpwd(8) if you want IndiMail to authenticate against ldap

## Backing up LDAP Database

```
% sudo /var/indimail/bin/svc -d /service/slapd.389
% slapcat -vl /etc/openldap/backup_slapd.ldif
% sudo /var/indimail/bin/svc -u /service/slapd.389
```

To Import

```
% sudo /var/indimail/bin/svc -d /service/slapd.389
% slapadd -vl /etc/openldap/backup_slapd.ldif
% chown ldap.ldap /var/lib/ldap/*
% sudo /var/indimail/bin/svc -u /service/slapd.389
```

## Email Client Settings

The last steps in setting up the shared address book is to configure the users email clients
to access the LDAP server.

The following table contains some of the information needed to configure the client
applications. Note the username will need to be written as the complete "distinguished name"
value so the server knows which object to authenticate.

Remember, not all clients can write to the address book, so use the phpLDAPadmin application
to add and manage the entries as needed.

Table|Description
-----|-----------
LDAP Server|your_host_IP:389
Search Base|ou=addressbook,o=indimail.org
Login Method|use distinguished name (if listed)
Username|uid=user,ou=addressbook,o=indimail.org
Password|As entered in newcontact.ldif file (plain text version)
Secure Connection|Never (unless encryption has been configured)

In the above replace 'user' with the actual user created in ldap by using ldapadd

checkpassword authentication

```
declare -x LDAP_BASE="ou=users,o=indimail.org"
declare -x LDAP_FILTER="(&(uid=%u)(o=%h))"
% printf "mbhangui@indimail.org\0manny\0\0" | ldap-checkpwd /bin/ls -ld /tmp 3<&0
% echo $?
0
```
