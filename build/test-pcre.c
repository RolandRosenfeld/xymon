#ifdef PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#else
#include <pcre.h>
#endif

int main(int argc, char *argv[])
{
#ifdef PCRE2
	pcre2_code *result;
	int err;
	PCRE2_SIZE errofs;
	result = pcre2_compile("xymon.*", PCRE2_ZERO_TERMINATED, PCRE2_CASELESS, &err, &errofs, NULL);
#else
	pcre *result;
	const char *errmsg;
	int errofs;
	result = pcre_compile("xymon.*", PCRE_CASELESS, &errmsg, &errofs, NULL);
#endif

	return 0;
}

