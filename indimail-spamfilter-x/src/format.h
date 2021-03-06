#ifndef FORMAT_H
#define FORMAT_H

#include "configfile.h"

typedef const char *FIELD;

/* Global variables */

extern char *spam_header_name;
extern char *spam_subject_tag;
extern char *unsure_subject_tag;

/* needed by bogoconfig.c */

extern char *header_format;
extern char *terse_format;
extern char *log_update_format;
extern char *log_header_format;
extern FIELD *spamicity_tags;
extern FIELD *spamicity_formats;

extern void set_terse_mode_format(int mode);

extern bool set_spamicity_tags(const char *val);
extern bool set_spamicity_formats(const char *val);
extern bool set_spamicity_fields(FIELD *strings, const char *val);

/* Function Prototypes */

extern char *format_header(char *buff, size_t size);
extern char *format_terse(char *buff, size_t size);
extern char *format_log_header(char *buff, size_t size);
extern void  format_set_counts(uint _wrd, uint _msg);
extern char *format_log_update(char *buff, size_t size, const char *reg, const char *unreg);
#endif
