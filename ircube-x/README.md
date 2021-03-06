# RoundCube Webmail Installation with IndiMail

These instructions will work on CentOS, RHEL, Fedora. For Debian/Ubuntu and other distros, please use your knowledge to make changes accordingly. In this guide, replace indimail.org with your own hostname.

## Non SSL Version Install/Configuration (look below for SSL config)

1. Install RoundCube. On older systems, use the yum command

   ```
   $ sudo dnf -y install roundcubemail php-mysqlnd
   ```

2. Connect to MySQL using a privileged user. IndiMail installation creates a privileged mysql user 'mysql'. It does not have the user 'root'. Look at the variable `PRIV_PASS` in `/usr/sbin/svctool` to know the password.

   a) Create RoundCube Database and roundcube user

   ```
   $ /usr/bin/mysql -u mysql -p mysql
   MySQL> create database RoundCube_db;
   MySQL> create user roundcube identified by 'subscribed';
   MySQL> GRANT ALL PRIVILEGES on RoundCube_db.* to roundcube;
   MySQL> FLUSH PRIVILEGES;
   MySQL> QUIT;
   ```

   b) Initialize RoundCube Database

   ```
   $ /usr/bin/mysql -u roundcube -p RoundCube_db < /usr/share/roundcubemail/SQL/mysql.initial.sql
   ```

3. Copy /etc/roundcubemail/config.inc.php.sample to /etc/roundcubemail/config.inc.php

   ```
   $ sudo cp /etc/roundcubemail/config.inc.php.sample /etc/roundcubemail/config.inc.php
   ```

   Edit the lines in /etc/roundcubemail/config.inc.php

   ```
   $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
   #
   # To use SSL/TLS connection, enter the hostname with prefix "ssl://" or
   # "tls://"
   # And if you want a drop-down list like it's explained in the comments you
   # need something like this:
   #$config['default_host'] = array('mail.example.com', 'webmail.example.com', 'ssl://mail.example.com:993');
   #
   # In order to show nice labels instead of the host names in the drop-down
   # box write it this way:
   #
   #$config['default_host'] = array(
   #  'mail.example.com' => 'Default Server',
   #  'webmail.example.com' => 'Webmail Server',
   #  'ssl://mail.example.com:993' => 'Secure Webmail Server'
   #);
   #
   $config['smtp_server'] = 'localhost';
   $config['smtp_port'] = 587;
   $config['smtp_user'] = '%u';
   $config['smtp_pass'] = '%p';
   $config['support_url'] = 'http://indimail.sourceforge.net';
   $config['product_name'] = 'IndiMail Webmail';
   $config['plugins'] = array(
       'archive',
       'sauserprefs',
       'markasjunk2',
       'iwebadmin',
   );
   ```

   NOTE: the iwebadmin plugin will not work for postmaster account or IndiMail users having QA\_ADMIN privileges. man vmoduser(1)

   This file should have read permission for apache group

   ```
   $ sudo chown root:apache /etc/roundcubemail/config.inc.php
   $ sudo chmod 640 /etc/roundcubemail/config.inc.php
   ```

   For `markasjunk2` to work you need to set permission for `apache` to write `/etc/indimail/spamignore`

4. Edit the lines in `/etc/roundcubemail/defaults.inc.php`

   ```
   $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
   $config['imap_auth_type'] = 'LOGIN';
   $config['smtp_auth_type'] = 'LOGIN';
   ```

   This file should have read permission for `apache` group

   ```
   $ sudo chown root:apache /etc/roundcubemail/defaults.inc.php
   $ sudo chmod 640 /etc/roundcubemail/defaults.inc.php
   ```

5. Restore indimail plugins for roundcube

   ```
   $ cd /tmp
   $ wget http://downloads.sourceforge.net/indimail/indimail-roundcube-1.0.tar.gz # This file
   $ cd /
   $ sudo tar xvfz /tmp/indimail-roundcube-1.0.tar.gz usr/share/roundcubemail/plugins
   $ /usr/bin/mysql -u roundcube -p RoundCube_db <
       /usr/share/roundcubemail/plugins/sauserprefs/sauserprefs.sql
   ```

6. Change iwebadmin path in /usr/share/roundcubemail/iwebadmin/config.inc.php

   ```
   $rcmail_config['iwebadmin_path'] = 'http://127.0.0.1/cgi-bin/iwebadmin';
   ```

7. Change `sauserprefs_db_dsnw` and `sauserprefs_whitelist_cmd` in `/usr/share/roundcubemail/sauserprefs/config.inc.php`

   ```
   $rcmail_config['sauserprefs_db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
   $rcmail_config['sauserprefs_whitelist_cmd'] = '/usr/libexec/indimail/bogo-learn %m %u %i whitelist %t';
   ```

8. change `pdo_mysql.default_socket` `/etc/php.ini`

   For some reason `pdo_mysql` uses wrong mysql socket on some systems. Uses `/var/lib/mysql/mysql.sock` instead of `/var/run/mysqld/mysqld.sock`. You need to edit the file `/etc/php.ini` and define `pdo_mysql.default_socket`

   `pdo_mysql.default_socket=/var/run/mysqld/mysqld.sock`

   You can verifiy if the path has been correctly entered by executing the below command. The command should return without any error

   ```
   $ php -r "new PDO('mysql:host=localhost;dbname=RoundCube_db', 'roundcube', 'subscribed');"
   ```

9. HTTPD config

   a) Edit file /etc/httpd/conf.d/roundcubemail.conf and edit the following lines

      ```
      #
      # Round Cube Webmail is a browser-based multilingual IMAP client
      #

      Alias /indimail /usr/share/roundcubemail

      # Define who can access the Webmail
      # You can enlarge permissions once configured

      <Directory /usr/share/roundcubemail/>
          <IfModule mod_authz_core.c>
              # Apache 2.4
              Require ip 127.0.0.1
              Require all granted 
              Require local
          </IfModule>
          <IfModule !mod_authz_core.c>
              # Apache 2.2
              Order Deny,Allow
              Deny from all
              Allow from 127.0.0.1
              Allow from ::1
          </IfModule>
      </Directory>
      ```

      This file should be owned by root

      ```
      $ sudo chown root:root /etc/httpd/conf.d/roundcubemail.conf
      $ sudo chmod 644 /etc/httpd/conf.d/roundcubemail.conf
      ```

   b) Restart httpd

      `$ sudo service httpd restart`

10. Login to webmail at http://localhost/indimail

## SSL/TLS Config

1. Install RoundCube

   ```
   $ sudo dnf -y install roundcubemail php-mysqlnd
   ```

2. Connect to MySQL using a privileged user

   a) Create RoundCube Database and roundcube user

   ```
   $ /usr/bin/mysql -u mysql -p mysql
   MySQL> create database RoundCube_db;
   MySQL> create user roundcube identified by 'subscribed';
   MySQL> GRANT ALL PRIVILEGES on RoundCube_db.* to roundcube;
   MySQL> FLUSH PRIVILEGES;
   MySQL> QUIT;
   ```

   b) Initialize RoundCube Database

   ```
   /usr/bin/mysql -u roundcube -p RoundCube_db < /usr/share/roundcubemail/SQL/mysql.initial.sql
   ```

3. Copy /etc/roundcubemail/config.inc.php.sample to /etc/roundcube.inc.php

   ```
   $ sudo cp /etc/roundcubemail/config.inc.php.sample /etc/roundcubemail/config.inc.php
   ```

   Edit the lines in /etc/roundcubemail/config.inc.php

   ```
   $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
   #
   # To use SSL/TLS connection, enter the hostname with prefix "ssl://" or
   # "tls://"
   # And if you want a drop-down list like it's explained in the comments you
   # need something like this:
   #$config['default_host'] = array('mail.example.com', 'webmail.example.com', 'ssl://mail.example.com:993');
   #
   # In order to show nice labels instead of the host names in the drop-down
   # box write it this way:
   #
   #$config['default_host'] = array(
   #  'mail.example.com' => 'Default Server',
   #  'webmail.example.com' => 'Webmail Server',
   #  'ssl://mail.example.com:993' => 'Secure Webmail Server'
   #);
   #
   $config['default_host'] = 'ssl://indimail.org';
   $config['default_port'] = 993;
   $config['imap_auth_type'] = 'LOGIN';
   $config['imap_conn_options'] = array(
     'ssl'         => array(
        'verify_peer'       => false,
        'verify_peer_name'  => false,
      ),
   );
   $config['smtp_server'] = 'localhost';
   $config['smtp_port'] = 587;
   $config['smtp_user'] = '%u';
   $config['smtp_pass'] = '%p';
   $config['smtp_auth_type'] = 'PLAIN';
   $config['smtp_conn_options'] = array(
     'ssl'         => array(
       'verify_peer'       => false,
       'verify_peer_name'  => false,
     ),
   );
   $config['force_https'] = true;
   $config['support_url'] = 'http://indimail.sourceforge.net';
   $config['product_name'] = 'IndiMail Webmail';
   $config['useragent'] = 'IndiMail Webmail/'.RCUBE_VERSION;
   $config['plugins'] = array(
       'archive',
       'sauserprefs',
       'markasjunk2',
       'iwebadmin',
   );
   ```

   NOTE: the iwebadmin plugin will not work for postmaster account or IndiMail users having `QA_ADMIN` privileges. man vmoduser(1)

   This file should have read permission for `apache` group

   ```
   $ sudo chown root:apache /etc/roundcubemail/config.inc.php
   $ sudo chmod 640 /etc/roundcubemail/config.inc.php
   ```

4. Check permissions of `/etc/roundcubemail/defaults.inc.php` i.e.

   This file should have read permission for `apache` group

   ```
   $ sudo chown root:apache /etc/roundcubemail/defaults.inc.php
   $ sudo chmod 640 /etc/roundcubemail/defaults.inc.php
   ```

5. Restore indimail plugins for roundcube

   ```
   $ cd /tmp
   $ wget http://downloads.sourceforge.net/indimail/indimail-roundcube-ssl-1.0.tar.gz # This file
   $ cd /
   $ sudo tar xvfz /tmp/indimail-roundcube-ssl-1.0.tar.gz usr/share/roundcubemail/plugins
   $ /usr/bin/mysql -u roundcube -p RoundCube_db <
      /usr/share/roundcubemail/plugins/sauserprefs/sauserprefs.sql
   ```

6. Change iwebadmin path in `/usr/share/roundcubemail/plugins/iwebadmin/config.inc.php`

   ```
   $rcmail_config['iwebadmin_path'] = 'https://127.0.0.1/cgi-bin/iwebadmin';
   ```

7. Change `sauserprefs_db_dsnw` and `sauserprefs_whitelist_cmd` in `/usr/share/roundcubemail/sauserprefs/config.inc.php`

   ```
   $rcmail_config['sauserprefs_db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
   $rcmail_config['sauserprefs_whitelist_cmd'] = '/usr/libexec/indimail/bogo-learn %m %u %i whitelist %t';
   ```

8. change `pdo_mysql.default_socket` `/etc/php.ini`

   For some reason `pdo_mysql` uses wrong mysql socket on some systems. Uses `/var/lib/mysql/mysql.sock` instead of `/var/run/mysqld/mysqld.sock`. You need to edit the file /etc/php.ini and define pdo_mysql.default_socket

   ```
   pdo_mysql.default_socket= /var/run/mysqld/mysqld.sock
   ```

   You can verifiy if the path has been correctly entered by executing the below command. The command should return without any error

   ```
   php -r "new PDO('mysql:host=localhost;dbname=RoundCube_db', 'roundcube', 'subscribed');"
   ```

9. HTTPD config

   a) Edit file `/etc/httpd/conf.d/roundcubemail.conf` and edit the following lines i.e.

      ```
      #
      # Round Cube Webmail is a browser-based multilingual IMAP client
      #

      Alias /indimail /usr/share/roundcubemail

      # Define who can access the Webmail
      # You can enlarge permissions once configured

      <Directory /usr/share/roundcubemail/>
          <IfModule mod_authz_core.c>
              # Apache 2.4
              Require ip 127.0.0.1
              Require all granted 
              Require local
          </IfModule>
          <IfModule !mod_authz_core.c>
              # Apache 2.2
              Order Deny,Allow
              Deny from all
              Allow from 127.0.0.1
              Allow from ::1
          </IfModule>
      </Directory>
      ```

      This file should be owned by `root`

      ```
      $ sudo chown root:root /etc/httpd/conf.d/roundcubemail.conf
      $ sudo chmod 644 /etc/httpd/conf.d/roundcubemail.conf
      ```

   b) This is assuming you have already generated indimail cert after indimail installation. If not execute the following command. We will assume that your host is indimail.org

      ```
      $ sudo /usr/sbin/svctool --postmaster=postmaster@indimail.org --config=cert" --common_name=indimail.org
      ```

      Edit the file /etc/httpd/conf.d/ssl.conf i.e.

      ```
      ServerName indimail.org:443
      SSLCertificateFile /etc/indimail/certs/servercert.pem
      ```

      Now apache server needs access to servercert.pem. Add `apache` user to the `qmail` group. You can chose either of the below two options. Options 2 is less secure, as it gives httpd access to qmail files

      **Option 1**

      ```
      $ sudo chgrp apache /etc/indimail/certs/servercert.pem
      $ sudo chmod 640 /etc/indimail/certs/servercert.pem
      ```

      **Option 2**

      ```
      $ sudo usermod -aG qmail apache
      ```

      Now you should see `apache` getting `qmail` group access

      ```
      $ grep "qmail:x:" /etc/group
      qmail:x:1002:qscand,apache
      ```


   c) Edit file `/etc/php.ini`. For some funny reason, the cert needs to be mentioned. i.e.

      ```
      openssl.cafile=/etc/indimail/certs/servercert.pem
      openssl.capath=/etc/pki/tls/certs
      ```

      Run the following command to get the cert locations. `[ini_cafile]` should point to servercert.pem location.

      ```
      $ php -r "print_r(openssl_get_cert_locations());"
      Array
      (
          [default_cert_file] => /etc/pki/tls/cert.pem
          [default_cert_file_env] => SSL_CERT_FILE
          [default_cert_dir] => /etc/pki/tls/certs
          [default_cert_dir_env] => SSL_CERT_DIR
          [default_private_dir] => /etc/pki/tls/private
          [default_default_cert_area] => /etc/pki/tls
          [ini_cafile] => /etc/indimail/certs/servercert.pem
          [ini_capath] => /etc/pki/tls/certs
      )
      ```

   d) Follow instructions to setup https [here](https://wiki.centos.org/HowTos/Https)

   e) Restart httpd

      ```
      $ sudo service httpd restart
      ```

   f) It appears that in PHP 5.6.0, functions are now validating SSL certificates (in a variety of ways). First, it appears to fail for untrusted certificates (i.e. no matching CA trusted locally), and secondly, it appears to fail for mismatched hostnames in the request and certificate.

      Verify that php is using the correct certificate with proper CN. Use the program testssl.php download from the location you downloaded this [README.md](README.md) file.

      In Step 9b you created a certificate with common_name as indimail.org. Use the same host that you gave when creating the certificate.

      ```
      $ php ./testssl.php indimail.org
      Success
      ```

10. Login to webmail

    a) edit /etc/hosts and edit the line for localhost i.e.

       `127.0.0.1 localhost indimail.org`

    b) Restart httpd

       `$ sudo service httpd restart`

    c) Login to webmail at [https://indimail.org/indimail](https://indimail.org/indimail)

NOTE: Replace indimail.org with domain that you have configured

1) Apache is running a threaded MPM, but your PHP Module is not compiled to be threadsafe in Fedora

   Replace

   `LoadModule mpm_event_module modules/mod_mpm_event.so`

   with

   `LoadModule mpm_prefork_module modules/mod_mpm_prefork.so`
