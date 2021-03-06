/*
 * $Log: process.c,v $
 * Revision 1.3  2020-06-21 12:49:17+05:30  Cprogrammer
 * fixed possible buffer overflow
 *
 * Revision 1.2  2020-04-30 19:49:14+05:30  Cprogrammer
 * fixed compilation error
 *
 * Revision 1.1  2013-05-15 00:33:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "process.h"

#define SET_PROCHDR_DEFAULT(ph) \
		(ph)->pid = 0; \
		(ph)->status = FREE;

int
start_process(struct process_hdr *p)
{

	pid_t           ppid;

	ppid = fork();
	if (ppid < 0) {
		perror("fork");
		exit(1);
	}
	if (ppid == 0) {			// child
		char           *pgname = progname;
		char            path[PATH_MAX + 2];
		char            pwd[PATH_MAX - 1];
		char           *pgargs[20];
		uint            i, j;

		i = 0;
		if (!getcwd(pwd, PATH_MAX - 1)) {
			perror("getcwd");
			fatal("getcwd failed");
		}
		if (!(pgname = strrchr(progname, '/')))
			pgname = progname;
		else
			pgname++;
		pgargs[i++] = pgname;

		pgargs[i] = (char *) xmalloc(3 * sizeof (char));
		SCP(pgargs[i++], "-m");
		pgargs[i++] = p->econf->pattern;

		pgargs[i] = (char *) xmalloc(3 * sizeof (char));
		SCP(pgargs[i++], "-e");
		pgargs[i++] = p->econf->cmd;

		pgargs[i] = (char *) xmalloc(3 * sizeof (char));
		SCP(pgargs[i++], "-r");
		pgargs[i] = (char *) xmalloc(6 * sizeof (char));
		sprintf(pgargs[i++], "%d", p->econf->retry);

		pgargs[i] = (char *) xmalloc(3 * sizeof (char));
		SCP(pgargs[i++], "-s");
		pgargs[i] = (char *) xmalloc(6 * sizeof (char));
		sprintf(pgargs[i++], "%d", p->econf->matchsleep);

		pgargs[i] = (char *) xmalloc(3 * sizeof (char));
		SCP(pgargs[i++], "-i");
		pgargs[i] = (char *) xmalloc(6 * sizeof (char));
		sprintf(pgargs[i++], "%d", p->econf->matchcount);

		if (p->econf->user) {
			pgargs[i] = (char *) xmalloc(3 * sizeof (char));
			SCP(pgargs[i++], "-u");
			pgargs[i++] = p->econf->user;
		}

		if (p->econf->readall) {
			pgargs[i] = (char *) xmalloc(3 * sizeof (char));
			SCP(pgargs[i++], "-b");
		}
		pgargs[i++] = p->econf->watchfile;
		pgargs[i] = NULL;
		snprintf(path, PATH_MAX + 2, "%s/%s", pwd, pgname);
		printf("\nExecuting: %s", path);
		for (j = 0; j < i; j++)
			printf("%s ", pgargs[j]);
		if (execv(path, pgargs) < 0) {
		//this means we did not execute from same directory, so
		//try using BINDIR instead.
			memset(path, 0, sizeof(path));
			snprintf(path, PATH_MAX - 1, "%s/%s", BINDIR, pgname);
			execv(path, pgargs);
		}
		exit(0);
	} else {
		p->pid = ppid;
		p->status = USED;
		return OK;
	}
}



int
new_proc_hdr(struct process_hdr **phdr, struct entry_conf **econf)
{
	if (!*econf)
		return 0;
	(*phdr) = (struct process_hdr *) xmalloc(sizeof (struct process_hdr));
	(*phdr)->econf = *econf;
	SET_PROCHDR_DEFAULT(*phdr);
	return 1;
}


void
create_proc_table(struct entry_conf **conf_table)
{

	uint            cur_proc;
	struct process_hdr **proc_table;

	proc_table = (struct process_hdr **) xmalloc((MAXPROCESS + 1) * sizeof (struct process_hdr *));
	for (cur_proc = 0; conf_table[cur_proc] != NULL; cur_proc++) {
	//printf("Processo %d = %s\n",cur_proc,conf_table[cur_proc]->watchfile);
		if (!new_proc_hdr(&proc_table[cur_proc], &conf_table[cur_proc])) {
			debug("[*] failed to create %dth process structure");
			continue;
		}
		start_process(proc_table[cur_proc]);
	}
	return;
}
