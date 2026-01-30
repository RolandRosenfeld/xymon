/*----------------------------------------------------------------------------*/
/* Xymon monitor library.                                                     */
/*                                                                            */
/* This is a library module, part of libxymon.                                */
/* It contains routines for matching names and expressions                    */
/*                                                                            */
/* Copyright (C) 2002-2011 Henrik Storner <henrik@storner.dk>                 */
/*                                                                            */
/* This program is released under the GNU General Public License (GPL),       */
/* version 2. See the file "COPYING" for details.                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

static char rcsid[] = "$Id$";

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifdef PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#else
#include <pcre.h>
#endif

#include "libxymon.h"

#ifdef PCRE2
pcre2_code *compileregex_opts(const char *pattern, uint32_t flags)
#else
pcre *compileregex_opts(const char *pattern, int flags)
#endif
{
#ifdef PCRE2
	pcre2_code *result;
	char errmsg[120];
	int err;
	PCRE2_SIZE errofs;
#else
	pcre *result;
	const char *errmsg;
	int errofs;
#endif

	dbgprintf("Compiling regex %s\n", pattern);
#ifdef PCRE2
	result = pcre2_compile(pattern, strlen(pattern), flags, &err, &errofs, NULL);
#else
	result = pcre_compile(pattern, flags, &errmsg, &errofs, NULL);
#endif
	if (result == NULL) {
#ifdef PCRE2
		pcre2_get_error_message(err, errmsg, sizeof(errmsg));
		errprintf("pcre compile '%s' failed (offset %zu): %s\n", pattern, errofs, errmsg);
#else
		errprintf("pcre compile '%s' failed (offset %d): %s\n", pattern, errofs, errmsg);
#endif
		return NULL;
	}

	return result;
}

#ifdef PCRE2
pcre2_code *compileregex(const char *pattern)
#else
pcre *compileregex(const char *pattern)
#endif
{
#ifdef PCRE2
	return compileregex_opts(pattern, PCRE2_CASELESS);
#else
	return compileregex_opts(pattern, PCRE_CASELESS);
#endif
}

#ifdef PCRE2
pcre2_code *multilineregex(const char *pattern)
#else
pcre *multilineregex(const char *pattern)
#endif
{
#ifdef PCRE2
	return compileregex_opts(pattern, PCRE2_CASELESS|PCRE2_MULTILINE);
#else
	return compileregex_opts(pattern, PCRE_CASELESS|PCRE_MULTILINE);
#endif
}

#ifdef PCRE2
int matchregex(const char *needle, pcre2_code *pcrecode)
#else
int matchregex(const char *needle, pcre *pcrecode)
#endif
{
#ifdef PCRE2
	pcre2_match_data *ovector;
#else
	int ovector[30];
#endif
	int result;

	if (!needle || !pcrecode) return 0;

#ifdef PCRE2
	ovector = pcre2_match_data_create(30, NULL);
	result = pcre2_match(pcrecode, needle, strlen(needle), 0, 0, ovector, NULL);
	pcre2_match_data_free(ovector);
#else
	result = pcre_exec(pcrecode, NULL, needle, strlen(needle), 0, 0, ovector, (sizeof(ovector)/sizeof(int)));
#endif
	return (result >= 0);
}

#ifdef PCRE2
void freeregex(pcre2_code *pcrecode)
#else
void freeregex(pcre *pcrecode)
#endif
{
	if (!pcrecode) return;

#ifdef PCRE2
	pcre2_code_free(pcrecode);
#else
	pcre_free(pcrecode);
#endif
}

#ifdef PCRE2
int namematch(const char *needle, char *haystack, pcre2_code *pcrecode)
#else
int namematch(const char *needle, char *haystack, pcre *pcrecode)
#endif
{
	char *xhay;
	char *tokbuf = NULL, *tok;
	int found = 0;
	int result = 0;
	int allneg = 1;

	if ((needle == NULL) || (*needle == '\0')) return 0;

	if (pcrecode) {
		/* Do regex matching. The regex has already been compiled for us. */
		return matchregex(needle, pcrecode);
	}

	if (strcmp(haystack, "*") == 0) {
		/* Match anything */
		return 1;
	}

	/* Implement a simple, no-wildcard match */
	xhay = strdup(haystack);

	tok = strtok_r(xhay, ",", &tokbuf);
	while (tok) {
		allneg = (allneg && (*tok == '!'));

		if (!found) {
			if (*tok == '!') {
				found = (strcmp(tok+1, needle) == 0);
				if (found) result = 0;
			}
			else {
				found = (strcmp(tok, needle) == 0);
				if (found) result = 1;
			}
		}

		/* We must check all of the items in the haystack to see if they are all negative matches */
		tok = strtok_r(NULL, ",", &tokbuf);
	}
	xfree(xhay);

	/* 
	 * If we didn't find it, and the list is exclusively negative matches,
	 * we must return a positive result for "no match".
	 */
	if (!found && allneg) result = 1;

	return result;
}

#ifdef PCRE2
int patternmatch(char *datatosearch, char *pattern, pcre2_code *pcrecode)
#else
int patternmatch(char *datatosearch, char *pattern, pcre *pcrecode)
#endif
{
	if (pcrecode) {
		/* Do regex matching. The regex has already been compiled for us. */
		return matchregex(datatosearch, pcrecode);
	}

	if (strcmp(pattern, "*") == 0) {
		/* Match anything */
		return 1;
	}

	return (strstr(datatosearch, pattern) != NULL);
}

#ifdef PCRE2
pcre2_code **compile_exprs(char *id, const char **patterns, int count)
#else
pcre **compile_exprs(char *id, const char **patterns, int count)
#endif
{
#ifdef PCRE2
	pcre2_code **result = NULL;
#else
	pcre **result = NULL;
#endif
	int i;

#ifdef PCRE2
	result = (pcre2_code **)calloc(count, sizeof(pcre2_code *));
#else
	result = (pcre **)calloc(count, sizeof(pcre *));
#endif
	for (i=0; (i < count); i++) {
		result[i] = compileregex(patterns[i]);
		if (!result[i]) {
			errprintf("Internal error: %s pickdata PCRE-compile failed\n", id);
#ifdef PCRE2
			for (i=0; (i < count); i++) if (result[i]) pcre2_code_free(result[i]);
#else
			for (i=0; (i < count); i++) if (result[i]) pcre_free(result[i]);
#endif
			xfree(result);
			return NULL;
		}
	}

	return result;
}

#ifdef PCRE2
int pickdata(char *buf, pcre2_code *expr, int dupok, ...)
#else
int pickdata(char *buf, pcre *expr, int dupok, ...)
#endif
{
	int res, i;
#ifdef PCRE2
	pcre2_match_data *ovector;
#else
	int ovector[30];
#endif
	va_list ap;
	char **ptr;
	char w[100];
#ifdef PCRE2
	PCRE2_SIZE l;
#endif

	if (!expr) return 0;

#ifdef PCRE2
	ovector = pcre2_match_data_create(30, NULL);
	res = pcre2_match(expr, buf, strlen(buf), 0, 0, ovector, NULL);
	if (res < 0) {
		pcre2_match_data_free(ovector);
		return 0;
	}
#else
	res = pcre_exec(expr, NULL, buf, strlen(buf), 0, 0, ovector, (sizeof(ovector)/sizeof(int)));
	if (res < 0) return 0;
#endif

	va_start(ap, dupok);

	for (i=1; (i < res); i++) {
		*w = '\0';
#ifdef PCRE2
		l = sizeof(w);
		pcre2_substring_copy_bynumber(ovector, i, w, &l);
#else
		pcre_copy_substring(buf, ovector, res, i, w, sizeof(w));
#endif
		ptr = va_arg(ap, char **);
		if (dupok) {
			if (*ptr) xfree(*ptr);
			*ptr = strdup(w);
		}
		else {
			if (*ptr == NULL) {
				*ptr = strdup(w);
			}
			else {
				dbgprintf("Internal error: Duplicate match ignored\n");
			}
		}
	}

	va_end(ap);
#ifdef PCRE2
	pcre2_match_data_free(ovector);
#endif

	return 1;
}


int timematch(char *holidaykey, char *tspec)
{
	int result;

	result = within_sla(holidaykey, tspec, 0);

	return result;
}


