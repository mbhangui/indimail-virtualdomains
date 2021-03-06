#include "unicode_config.h"
#include "courier-unicode.h"

#include "eastasianwidth.h"
#include "linebreaktab_internal.h"

#include <stdlib.h>

unicode_eastasia_t unicode_eastasia(char32_t c)
{
	size_t b=0;
	size_t e=sizeof(unicode_eastasia_tab)/sizeof(unicode_eastasia_tab[0]);

	while (b < e)
	{
		size_t n=b + (e-b)/2;

		if (c >= unicode_eastasia_tab[n][0])
		{
			if (c <= unicode_eastasia_tab[n][1])
				return unicode_eastasia_v[n];
			b=n+1;
		}
		else
		{
			e=n;
		}
	}

	return UNICODE_EASTASIA_N;
}

int unicode_wcwidth(char32_t c)
{
	switch (unicode_lb_lookup(c)) {
	case UNICODE_LB_BK:
	case UNICODE_LB_CR:
	case UNICODE_LB_LF:
	case UNICODE_LB_CM:
	case UNICODE_LB_NL:
	case UNICODE_LB_WJ:
	case UNICODE_LB_ZW:
		return 0;
	default:
		break;
	}

	switch (unicode_eastasia(c)) {
	case UNICODE_EASTASIA_F:
	case UNICODE_EASTASIA_W:
		return 2;
	}

	return 1;
}

size_t unicode_wcwidth_str(const char32_t *c)
{
	size_t w=0;

	while (*c)
		w += unicode_wcwidth(*c++);


	return w;
}
