# INDIMAIL DOMAIN LIMITS                                         (1/15/2004)

## OVERVIEW

IndiMail can set certain limits for domains.  Limits can optionally be stored in a mysql table (limits) instead when indimail is configured with --enable-domain-limits. 

Items that can be limited include:

* Default Quota for new users (default_quota, in bytes)
* Default Maximum Message Count Quota for new users (default_maxmsgcount)
* Disable POP Access (disable_pop)
* Disable IMAP Access (disable_imap)
* Disable Dialup Access (disable_dialup)
* Disable Password Changing (disable_password_changing)
* Disable External Relay/Roaming Users (disable_external_relay)
* Disable SMTP AUTHORIZATION (disable_smtp)

Quota for Entire Domain (quota, in megabytes)
Maximum Message Count for Entire Domain (maxmsgcount)

In addition, you can set administrator specific limits on the domain administrator (usually postmaster), such as:

* Maximum Number of POP/IMAP Accounts (maxpopaccounts)
* Maximum Number of Forwards (maxforwards)
* Maximum Number of Autoresponders (maxautoresponders)
* Maximum Number of Mailing Lists (maxmailinglists))

## CONFIGURATION

Modification of domain limits is done with vmoddomlimits.  If no arguments are given, then usage is displayed.  The -S argument will show the current domain limits. For example, if I wanted to set limits on test.com, I could issue:

```
$ vmoddomlimits -P 11 -L 2 -g p -q 10485760 test.com
```

The above command will limit an administrator logged into QmailAdmin (postmaster) to 11 pop accounts, and 2 mailing lists. It will also deny pop access, and set the default quota to 10MB (in bytes).  Here's the output of vmoddomlimits -S test.com:

```
Domain: test.com
--
Max Pop Accounts: 11
Max Aliases: -1
Max Forwards: -1
Max Autoresponders: -1
Max Mailinglists: 2
GID Flags:
  NO_POP
Flags (for commandline): p
Flags for non postmaster accounts:
  pop account:            DENY_CREATE  DENY_MODIFY  DENY_DELETE  
  alias:                  ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  forward:                ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  autoresponder:          ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  mailinglist:            ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  mailinglist users:      ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  mailinglist moderators: ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  quota:                  ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
  default quota:          ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE 
Domain Quota: 0 MB
Default User Quota: 10485760 bytes
Max Domain Messages: 0
Default Max Messages per User: 0
```
	
If you want to edit or show the ~indimail/etc/vlimits.default file, just use the -d option with vmoddomlimits program and leave off the domain argument.

To set unique limits on a single user, you may still manually set the gid flag using vmoduser. The domain limits are applied in addition to the gid flag limits. So, if you want to *remove* limits for a certain user, like allow POP access when it is turned off for a whole domain, you'll need to set the **V\_OVERRIDE** flag via vmoduser -o user@test.com. The V_OVERRIDE flag will override any domain limits that are in place. The user's gid flag limits will still apply.

## CREDITS

The original vlimits idea and code was done by Brian Kolaci (bk@kola.com) for vpopmail. This document was originally written by Bill Shupp (hostmaster@shupp.org) for vpopmail.
