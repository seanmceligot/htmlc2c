#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pcre.h>
#include <libtemplate.h>

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#ifndef BOOL
#define BOOL int
#endif

#define BUFSIZE 1024

FILE* out;

void usage (char *argv0)
{
	printf ("usage %s filename", argv0);
	exit (1);
}

void get_basename (const char *filename, char* basename)
{
	char* dot;

	strcpy(basename,filename);
	dot = strrchr (basename, '.');
	if (dot == NULL) {
		return;
	}
	*dot = 0;
}

pcre *code_re;
pcre *script_re;
pcre *end_re;

pcre *compile (char *pattern)
{
	const char *error;
	int erroffset;
	pcre* re;

	fprintf (stderr, "%s\n", pattern);
	re = pcre_compile (pattern, 0, &error, &erroffset, NULL);
	if (!re) {
		fprintf (stderr, "PCRE compilation failed at offset %d: %s\n", erroffset,
						 error);
		return NULL;
	}
	return re;
}

BOOL gencp_init ()
{
	char *code_pattern = "\\<code\\ class=\\\"cgi\\\"\\>";
	char *script_pattern =
	"\\<script\\ type=\\\"text\\/c\\+\\+\\\"\\ class=\\\"cgi\\\"\\>";
	char *end_pattern = "</code|script>";
	code_re = compile (code_pattern);
	script_re = compile (script_pattern);
	end_re = compile (end_pattern);
	return (code_re != NULL) && (script_re != NULL) && (end_re != NULL);
}

char *nextline (FILE * fin)
{
	static char inbuf[BUFSIZE];
	char *lineptr = fgets (inbuf, BUFSIZE, fin);
	return lineptr;
}

BOOL getmatch (char *lineptr, int *out_offset, int *out_len, pcre * re,
							 int linelen)
{
	const int RCOUNT = 3;
	int result[RCOUNT];
	int resultcount =
		pcre_exec (re, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
	int end;

	if ((resultcount < 0) && (resultcount != PCRE_ERROR_NOMATCH)) {
		fprintf (stderr, "Matching error %d (%s)\n", resultcount, lineptr);
	} else if (resultcount > 0) {
		*out_offset = result[0];
		end = result[1];
		*out_len = end - *out_offset;
		return TRUE;
	}
	return FALSE;
}

BOOL getcodeend (char *lineptr, int *out_offset, int *out_len)
{
	int linelen = strlen (lineptr);
	return getmatch (lineptr, out_offset, out_len, end_re, linelen);
}

BOOL getcodestart (char *lineptr, int *out_offset, int *out_len)
{
	int linelen = (int) strlen (lineptr);
	if (getmatch (lineptr, out_offset, out_len, code_re, linelen)) {
		return TRUE;
	}
	return getmatch (lineptr, out_offset, out_len, script_re, linelen);
}
void htmlputc (char ch)
{
	switch (ch) {
	case '\r':
	case '\n':
		break;
	case '\"':
		fputs ("\\\"", out);
		break;
	case '\'':
		fputs ("\\\'", out);
		break;
	default:
		putc (ch, out);
	}


}
void printhtml (char *lineptr)
{
	if (strcmp (lineptr, "\n") == 0) {
		return;
	}
	fputs ("  puts(\"", out);
	while (*lineptr) {
		htmlputc (*lineptr);
		lineptr++;
	}
	fputs ("\");\n", out);
}
void printnhtml (char *lineptr, int len)
{
				int i;
	fputs ("  puts(\"", out);
	for (i = 0; i < len; i++) {
		htmlputc (*lineptr);
		lineptr++;
	}
	fputs ("\");\n", out);
}
void printcode (char *lineptr)
{
	fprintf (out, "  %s\n", lineptr);
}
void printncode (char *lineptr, int len)
{
				int i;
	for (i = 0; i < len; i++) {
		fputc (lineptr[i], out);
	}
}
void parse (FILE * fin)
{
	char *tmpname = "docgi.tmp";
	char *lineptr;
	int out_offset;
	int out_len;

	out = fopen (tmpname, "w");
	if (out == NULL) {
		fprintf (stderr, "could not open (%s)\n", tmpname);
	}
	while ((lineptr = nextline (fin)) != NULL) {
		if (getcodestart (lineptr, &out_offset, &out_len)) {
			printnhtml (lineptr, out_offset - 1);
			lineptr += out_offset + out_len;
			do {
				printcode (lineptr);
				lineptr = nextline (fin);
				if (lineptr == NULL) {
					return;
				}
			} while (!getcodeend (lineptr, &out_offset, &out_len));
			printncode (lineptr, out_offset);
			fputc ('\n', out);
			lineptr += out_offset + out_len;
		} else {
			printhtml (lineptr);
		}
	}
	fclose (out);
}
void writeTemplate (Template * libtemplate, const char *tin, const char *fname, const char *ext, const BOOL overwrite)
{
//char *templatedir = "/usr/share/cerverpages";
	char *template_dir = "templates";
	char inname[256];
	char outname[256];
	FILE* in;
	FILE* exists;

	strcpy (inname, template_dir);
	strcat (inname, "/");
	strcat (inname, tin);
	strcpy (outname, fname);
	strcat (outname, ext);

	in = fopen (inname, "r");
	if (in == NULL) {
		fprintf (stderr, "could not open (%s)\n", inname);
	}
	exists = fopen (outname, "r");
	if (exists) {
		fclose (exists);
		if (!overwrite) {
			fprintf (stderr, "not overwriting %s\n", outname);
			return;
		}
	}
	fprintf(stderr, "%s -> %s\n", inname, outname);
	out = fopen (outname, "w");
	if (out == NULL) {
		fprintf (stderr, "could not open (%s)\n", outname);
	}
	libtemplate->out = out;
	template_parse (libtemplate, in);
}
void uppercase(char* s) {
				while(*s != 0) {
								if (islower(*s)) {
												*s = *s-('a'-'A');
								}
								s++;
				}
}
void gencp(const char *basename, const char *htmlfile)
{
	Template* libtemplate;
	char* upname;
	FILE *htmlin = fopen (htmlfile, "r");
	
	parse (htmlin);
	fclose (htmlin);
	template_init();
	libtemplate = template_new ();
	template_addkeyvalue (libtemplate, "name", basename);
	upname = strdup (basename);
	uppercase(upname);

	template_addkeyvalue (libtemplate, "upname", upname);
	writeTemplate (libtemplate, "impl.c", basename, ".c", FALSE);
	writeTemplate (libtemplate, "impl.h", basename, ".h", FALSE);
	writeTemplate (libtemplate, "impl_docgi.c", basename, "_docgi.c", TRUE);
	template_destroy(libtemplate);
	template_shutdown();
	free(upname);
}
void gencp_start(const char* htmlfile) {
	char basename[FILENAME_MAX];

	if (!gencp_init ()) {
					exit(1);
	}
	printf ("%s\n", htmlfile);
	get_basename (htmlfile, basename);
	printf ("%s\n", basename);
	gencp(basename, htmlfile);
}
int main (int argc, char **argv)
{
	/*int arg = 1;
	if (argc < arg) {
		usage (argv[0]);
	}
	const char *htmlfile = argv[arg];
	*/
	const char* htmlfile = "test.html";
	gencp_start(htmlfile);
	return 0;
}
