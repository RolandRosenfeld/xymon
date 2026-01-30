/*----------------------------------------------------------------------------*/
/* Xymon monitor library.                                                     */
/*                                                                            */
/* Copyright (C) 2004-2011 Henrik Storner <henrik@hswn.dk>                    */
/*                                                                            */
/* This program is released under the GNU General Public License (GPL),       */
/* version 2. See the file "COPYING" for details.                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef __LOADALERTS_H__
#define __LOADALERTS_H__

#include <time.h>
#include <stdio.h>

/* The clients probably don't have the pcre headers */
#if defined(LOCALCLIENT) || !defined(CLIENTONLY)
#ifdef PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#else
#include <pcre.h>
#endif

typedef enum { A_PAGING, A_NORECIP, A_ACKED, A_RECOVERED, A_DISABLED, A_NOTIFY, A_DEAD } astate_t;

typedef struct activealerts_t {
	/* Identification of the alert */
	char *hostname;
	char *testname;
	char *location;
	char *osname;
	char *classname;
	char *groups;
	char ip[IP_ADDR_STRLEN];

	/* Alert status */
	int color, maxcolor;
	unsigned char *pagemessage;
	unsigned char *ackmessage;
	time_t eventstart;
	time_t nextalerttime;
	astate_t state;
	int cookie;

	struct activealerts_t *next;
} activealerts_t;

/* These are the criteria we use when matching an alert. Used both generally for a rule, and for recipients */
enum method_t { M_MAIL, M_SCRIPT, M_IGNORE };
enum msgformat_t { ALERTFORM_TEXT, ALERTFORM_PLAIN, ALERTFORM_SMS, ALERTFORM_PAGER, ALERTFORM_SCRIPT, ALERTFORM_NONE };
enum recovermsg_t { SR_UNKNOWN, SR_NOTWANTED, SR_WANTED };
typedef struct criteria_t {
	int cfid;
	char *cfline;
	char *pagespec;		/* Pages to include */
#ifdef PCRE2
	pcre2_code *pagespecre;
#else
	pcre *pagespecre;
#endif
	char *expagespec;	/* Pages to exclude */
#ifdef PCRE2
	pcre2_code *expagespecre;
#else
	pcre *expagespecre;
#endif
	char *dgspec;		/* Display groups to include */
#ifdef PCRE2
	pcre2_code *dgspecre;
#else
	pcre *dgspecre;
#endif
	char *exdgspec;		/* Display groups to exclude */
#ifdef PCRE2
	pcre2_code *exdgspecre;
#else
	pcre *exdgspecre;
#endif
	char *hostspec;		/* Hosts to include */
#ifdef PCRE2
	pcre2_code *hostspecre;
#else
	pcre *hostspecre;
#endif
	char *exhostspec;	/* Hosts to exclude */
#ifdef PCRE2
	pcre2_code *exhostspecre;
#else
	pcre *exhostspecre;
#endif
	char *svcspec;		/* Services to include */
#ifdef PCRE2
	pcre2_code *svcspecre;
#else
	pcre *svcspecre;
#endif
	char *exsvcspec;	/* Services to exclude */
#ifdef PCRE2
	pcre2_code *exsvcspecre;
#else
	pcre *exsvcspecre;
#endif
	char *classspec;
#ifdef PCRE2
	pcre2_code *classspecre;
#else
	pcre *classspecre;
#endif
	char *exclassspec;
#ifdef PCRE2
	pcre2_code *exclassspecre;
#else
	pcre *exclassspecre;
#endif
	char *groupspec;
#ifdef PCRE2
	pcre2_code *groupspecre;
#else
	pcre *groupspecre;
#endif
	char *exgroupspec;
#ifdef PCRE2
	pcre2_code *exgroupspecre;
#else
	pcre *exgroupspecre;
#endif
	int colors;
	char *timespec;
	char *extimespec;
	int minduration, maxduration;	/* In seconds */
	enum recovermsg_t sendrecovered, sendnotice;
} criteria_t;

/* This defines a recipient. There may be some criteria, and then how we send alerts to him */
typedef struct recip_t {
	int cfid;
	criteria_t *criteria;
	enum method_t method;
	char *recipient;
	char *scriptname;
	enum msgformat_t format;
	time_t interval;		/* In seconds */
	int stoprule, unmatchedonly, noalerts;
	struct recip_t *next;
} recip_t;

extern int load_alertconfig(char *configfn, int alertcolors, int alertinterval);
extern void dump_alertconfig(int showlinenumbers);
extern void set_localalertmode(int localmode);

extern int stoprulefound;
extern recip_t *next_recipient(activealerts_t *alert, int *first, int *anymatch, time_t *nexttime);
extern int have_recipient(activealerts_t *alert, int *anymatch);

extern void alert_printmode(int on);
extern void print_alert_recipients(activealerts_t *alert, strbuffer_t *buf);
#endif

#endif

