# LOGALERT

Lastchange: 2005 08 30


THANKS for downloading and (so far :-) reading this! I hope you enjoy and
would like to receive your feedback.


## DESCRIPTION

logalert is a logfile monitoring tool which executes a specific action whenever it matches a
string (pattern) occurrence. 

It reads an entire file [or starts at the end, just like 'tail -f'], keeps track of any
changes, waiting for a specific string [a syslog process error, a user login, ...] and fires an
action you define when that happens.

Log rotation or temporary removal ? No problem, we can deal with that ! System administrators
normaly configure a logrotation process to rotate file as they grown or based on certain time
periods [daily, weekly, ...] truncating or temporily removing them - logalert keeps track of
them even so.

## LICENSE

Please refer to LICENSE for license details.

## INSTALATION

Please refer to INSTALL for instalation details.

## DOCUMENTATION

You can always find the latest documentation at:

http://logalert.sourceforge.net

Or you can look at doc/ directory where you'll find this releases documents.

## EXTRAS

Look at contrib/ directory, you'll find some extra-goodies. For example, there you'll
find a perl script 'sendmail' if you plan to configure some auto-mailing actions.


## EXAMPLES

1.Imagine you have a VPN in your firewall server and would like to restart it everytime it goes down. The process related to the VPN reports it's information via syslog to

`/var/log/vpn`

... and everytime a problem occurs, it dumps the message: 

```
    'Error: problem found, quiting...' 
```

You already have a script in /etc/init.d/startprocess that [re]starts that guy, so you could:

```
./logalert --match='[Ee]rror: problem found, quiting...' --exec='/etc/init.d/startprocess \
   restart' /var/log/process.log
```

2. Ok, you have dozens of files you whish to monitor ?
   Just create a configuration file like this:

```
logalert.conf
---------------
filename /var/log/messages
{
        match           = /[Ee][Rr]{2}or: VPN has lost/
        exec            = /etc/init.d/vpn restart
        match_sleep     = 5
        retry           = 3
        user            = vpnuser
}

filename /var/log/mail
{
        match           = /Bounce from:/
        exec            = /home/willy/alertme.sh
        match_sleep     = 5
        retry           = 3
        user            = willy
}

[...]
```

And start logalert in parent mode:

`./logalert -c /etc/logalert.conf`
