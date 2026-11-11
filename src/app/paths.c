/* The paths of the file picker: a name on a folder, and . and .. taken
 * out. */
#include <string.h>
#include "app.h"

void
path_join(char *out, const char *dir, const char *name)
{
	size_t n;

	out[0] = 0;
	if(rootlen(name) == 0)
		strcpy(out, dir);
	else if(path_sep == '\\' && name[1] != ':' && dir[0] && dir[1] == ':') {
		out[0] = dir[0];	/* \X on the drive of the folder */
		out[1] = ':';
		out[2] = 0;
	}
	n = strlen(out);
	if(n && !issep(out[n - 1]) && out[n - 1] != ':' && rootlen(name) == 0)
		out[n++] = path_sep;
	out[n] = 0;
	strncat(out, name, PATH_MAX_ - n - 1);
}

void
path_normalize(char *p)
{
	size_t r = rootlen(p), n;
	char *w = p + r, *s = p + r, *e;

	if(path_sep == '\\') {
		for(e = p; *e; e++)
			if(*e == '/')
				*e = '\\';
	}
	while(*s) {
		while(issep(*s))
			s++;
		if(*s == 0)
			break;
		for(e = s; *e && !issep(*e); e++)
			;
		n = (size_t)(e - s);
		if(n == 1 && s[0] == '.')
			;
		else if(n == 2 && s[0] == '.' && s[1] == '.') {
			while(w > p + r && !issep(w[-1]))
				w--;
			if(w > p + r)
				w--;
		} else {
			if(w > p + r)
				*w++ = path_sep;
			memmove(w, s, n);
			w += n;
		}
		s = e;
	}
	*w = 0;
}
