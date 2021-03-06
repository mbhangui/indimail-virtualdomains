/*
 * $Id: util.h,v 1.2 2019-06-03 06:48:20+05:30 Cprogrammer Exp mbhangui $
 */
#include <stralloc.h>

int             check_local_user(char *user);
int             fixup_local_name(char *addr);
int             check_email_addr(char *addr);
int             check_indimail_alias(char *, stralloc *);
int             open_lang(char *lang);
int             open_colortable();
char           *strstart(char *sstr, char *tstr);
char           *safe_getenv(char *var);
char           *get_html_text(char *index);
char           *get_color_text(char *index);
void            upperit(char *instr);
void            ack(char *msg, char *extra);
void            show_counts();

/*
 * prototypes for sorting functions in util.c 
 */
int             sort_init();
int             sort_add_entry(char *, char);
unsigned char  *sort_get_entry(int);
void            sort_cleanup();
void            sort_dosort();
void            str_replace(char *, char, char);

void            qmail_button(char *modu, char *command, char *user, char *dom, time_t mytime, char *png);

int             quota_to_bytes(char[], char *);	//jhopper prototype
int             quota_to_megabytes(char[], char *);	//jhopper prototype

void            print_user_index(char *action, int colspan, char *user, char *dom, time_t mytime);
char           *cgiurl(char *action);
