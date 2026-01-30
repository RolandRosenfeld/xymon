/*----------------------------------------------------------------------------*/
/* Xymon message daemon.                                                      */
/*                                                                            */
/* Client backend module for IRIX                                             */
/*                                                                            */
/* Copyright (C) 2005-2011 Henrik Storner <henrik@hswn.dk>                    */
/*                                                                            */
/* This program is released under the GNU General Public License (GPL),       */
/* version 2. See the file "COPYING" for details.                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

static char irix_rcsid[] = "$Id$";

void handle_irix_client(char *hostname, char *clienttype, enum ostype_t os, 
			void *hinfo, char *sender, time_t timestamp,
			char *clientdata)
{
#ifdef PCRE2
	static pcre2_code *memptn = NULL;
#else
	static pcre *memptn = NULL;
#endif
	char *timestr;
	char *uptimestr;
	char *clockstr;
	char *msgcachestr;
	char *whostr;
	char *psstr;
	char *topstr;
	char *dfstr;
	char *msgsstr;
	char *netstatstr;
	// char *sarstr;
	char *ifstatstr;
	char *portsstr;

	char fromline[1024];

	sprintf(fromline, "\nStatus message received from %s\n", sender);

	splitmsg(clientdata);

	timestr = getdata("date");
	uptimestr = getdata("uptime");
	clockstr = getdata("clock");
	msgcachestr = getdata("msgcache");
	whostr = getdata("who");
	psstr = getdata("ps");
	topstr = getdata("top");
	dfstr = getdata("df");
	msgsstr = getdata("msgs");
	netstatstr = getdata("netstat");
	ifstatstr = getdata("ifstat");
	// sarstr = getdata("sar");
	portsstr = getdata("ports");

	unix_cpu_report(hostname, clienttype, os, hinfo, fromline, timestr, uptimestr, clockstr, msgcachestr, 
			whostr, 0, psstr, 0, topstr);
	unix_disk_report(hostname, clienttype, os, hinfo, fromline, timestr, "Available", "Capacity", "Mounted", dfstr);
	unix_procs_report(hostname, clienttype, os, hinfo, fromline, timestr, "COMMAND", NULL, psstr);
	unix_ports_report(hostname, clienttype, os, hinfo, fromline, timestr, 3, 4, 5, portsstr);

	msgs_report(hostname, clienttype, os, hinfo, fromline, timestr, msgsstr);
	file_report(hostname, clienttype, os, hinfo, fromline, timestr);
	linecount_report(hostname, clienttype, os, hinfo, fromline, timestr);
	deltacount_report(hostname, clienttype, os, hinfo, fromline, timestr);

	unix_netstat_report(hostname, clienttype, os, hinfo, fromline, timestr, netstatstr);
	unix_ifstat_report(hostname, clienttype, os, hinfo, fromline, timestr, ifstatstr);
	/* unix_sar_report(hostname, clienttype, os, hinfo, fromline, timestr, sarstr); */

	if (topstr) {
		char *memline, *eoln = NULL;
		int res;
#ifdef PCRE2
		pcre2_match_data *ovector;
#else
		int ovector[20];
#endif
		char w[20];
#ifdef PCRE2
		PCRE2_SIZE l;
#endif
		long memphystotal = -1, memphysused = -1, memphysfree = 0,
		     memacttotal = -1, memactused = -1, memactfree = -1,
		     memswaptotal = -1, memswapused = -1, memswapfree = 0;

		if (!memptn) {
			memptn = compileregex("^Memory: (\\d+)M max, (\\d+)M avail, (\\d+)M free, (\\d+)M swap, (\\d+)M free swap");
		}

#ifdef PCRE2
		ovector = pcre2_match_data_create(20, NULL);
#endif
		memline = strstr(topstr, "\nMemory:");
		if (memline) {
			memline++;
			eoln = strchr(memline, '\n'); if (eoln) *eoln = '\0';

#ifdef PCRE2
			res = pcre2_match(memptn, memline, strlen(memline), 0, 0, ovector, NULL);
#else
			res = pcre_exec(memptn, NULL, memline, strlen(memline), 0, 0, ovector, (sizeof(ovector)/sizeof(int)));
#endif
		}
		else res = -1;

		if (res > 1) {
#ifdef PCRE2
			l = sizeof(w);
			pcre2_substring_copy_bynumber(ovector, 1, w, &l);
#else
			pcre_copy_substring(memline, ovector, res, 1, w, sizeof(w));
#endif
			memphystotal = atol(w);
		}
		if (res > 2) {
#ifdef PCRE2
			l = sizeof(w);
			pcre2_substring_copy_bynumber(ovector, 2, w, &l);
#else
			pcre_copy_substring(memline, ovector, res, 2, w, sizeof(w));
#endif
			memactfree = atol(w);
			memacttotal = memphystotal;
			memactused = memphystotal - memactfree;
		}
		if (res > 3) {
#ifdef PCRE2
			l = sizeof(w);
			pcre2_substring_copy_bynumber(ovector, 3, w, &l);
#else
			pcre_copy_substring(memline, ovector, res, 3, w, sizeof(w));
#endif
			memphysfree = atol(w);
			memphysused = memphystotal - memphysfree;
		}

		if (res > 4) {
#ifdef PCRE2
			l = sizeof(w);
			pcre2_substring_copy_bynumber(ovector, 4, w, &l);
#else
			pcre_copy_substring(memline, ovector, res, 4, w, sizeof(w));
#endif
			memswaptotal = atol(w);
		}
		if (res > 5) {
#ifdef PCRE2
			l = sizeof(w);
			pcre2_substring_copy_bynumber(ovector, 5, w, &l);
#else
			pcre_copy_substring(memline, ovector, res, 5, w, sizeof(w));
#endif
			memswapfree = atol(w);
		}
		memswapused = memswaptotal - memswapfree;
#ifdef PCRE2
		pcre2_match_data_free(ovector);
#endif

		if (eoln) *eoln = '\n';

		unix_memory_report(hostname, clienttype, os, hinfo, fromline, timestr,
				   memphystotal, memphysused, memacttotal, memactused, memswaptotal, memswapused);
	}

	splitmsg_done();
}

