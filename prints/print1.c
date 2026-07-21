#include <unistd.h>

static char buf[32];
static int pos;

void
addchar(char c) {
	if(pos == 32) {
		write(1, buf, pos);
		pos = 0;
	}
	buf[pos++] = c;
}

int
getarg(int which, int a, int b, int c) {
	switch(which) {
	case 0:
		return a;
	case 1:
		return b;
	case 2:
		return c;
	}
	return 0;
}

int
fmtdec(int val) {
	char digits[10];
	int ndig;
	int i;

	if(val == 0) {
		addchar('0');
		return 1;
	}
	ndig = 0;
	while(val != 0) {
		digits[ndig] = (val % 10) + '0';
		val /= 10;
		ndig++;
	}
	for(i = ndig - 1; i >= 0; i--)
		addchar(digits[i]);
	return ndig;
}

int
print(char *fmt, int a, int b, int c, char *s) {
	int n, val;
	char *p, *q;
	int which;

	n = 0;
	which = 0;
	for(p = fmt; *p != '\0'; ++p) {
		if(*p != '%') {
			addchar(*p);
			n++;
		}
		else {
			p++;
			switch(*p) {
			case '%':
				addchar('%');
				n++;
				break;
			case 'c':
				addchar(getarg(which, a, b, c));
				which++;
				n++;
				break;
			case 's':
				for(q = s; *q != '\0'; q++) {
					addchar(*q);
					n++;
				}
				break;
			case 'd':
				val = getarg(which, a, b, c);
				which++;
				n += fmtdec(val);
				break;
			}
		}
	}
	write(1, buf, pos);
	return n;
}