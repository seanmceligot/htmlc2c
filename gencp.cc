#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pcre.h>
#include <libtemplate.h>


char *templatedir = "templates";
//char *templatedir = "/usr/share/cerverpages";
char *outdir = ".";
FILE *out = stdout;

void usage (char *argv0)
{
	printf ("usage %s filename", argv0);
	exit (1);
}

void get_basename (const char *filename, char* basename)
{
	strcpy(basename,filename);
	char *dot = strrchr (basename, '.');
	if (dot == NULL) {
		return;
	}
	*dot = 0;
}

char *code_pattern = "\\<code\\ class=\\\"cgi\\\"\\>";
char *script_pattern =
	"\\<script\\ type=\\\"text\\/c\\+\\+\\\"\\ class=\\\"cgi\\\"\\>";
char *end_pattern = "</code|script>";
pcre *code_re;
pcre *script_re;
pcre *end_re;

pcre *compile (char *pattern)
{
	const char *error;
	int erroffset;
	fprintf (stderr, "%s\n", pattern);
	pcre *re = pcre_compile (pattern, 0, &error, &erroffset, NULL);
	if (!re) {
		fprintf (stderr, "PCRE compilation failed at offset %d: %s\n", erroffset,
						 error);
		return NULL;
	}
	return re;
}

bool gencp_init ()
{
	code_re = compile (code_pattern);
	script_re = compile (script_pattern);
	end_re = compile (end_pattern);
	return (code_re != NULL) && (script_re != NULL) && (end_re != NULL);
}
const int BUFSIZE = 1024;
char inbuf[BUFSIZE];

char *nextline (FILE * fin)
{
	char *lineptr = fgets (inbuf, BUFSIZE, fin);
	return lineptr;
}

bool getmatch (char *lineptr, int *out_offset, int *out_len, pcre * re,
							 int linelen)
{
	const int RCOUNT = 3;
	int result[RCOUNT];
	int resultcount =
		pcre_exec (re, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
	if ((resultcount < 0) && (resultcount != PCRE_ERROR_NOMATCH)) {
		fprintf (stderr, "Matching error %d (%s)\n", resultcount, lineptr);
	} else if (resultcount > 0) {
		*out_offset = result[0];
		const int end = result[1];
		*out_len = end - *out_offset;
		return true;
	}
	return false;
}

bool getcodeend (char *lineptr, int *out_offset, int *out_len)
{
	int linelen = strlen (lineptr);
	return getmatch (lineptr, out_offset, out_len, end_re, linelen);
}

bool getcodestart (char *lineptr, int *out_offset, int *out_len)
{
	int linelen = (int) strlen (lineptr);
	if (getmatch (lineptr, out_offset, out_len, code_re, linelen)) {
		return true;
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
	fputs ("  puts(\"", out);
	for (int i = 0; i < len; i++) {
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
	for (int i = 0; i < len; i++) {
		fputc (lineptr[i], out);
	}
}
void parse (FILE * fin)
{
	char *tmpname = "docgi.tmp";
	out = fopen (tmpname, "w");
	if (out == NULL) {
		fprintf (stderr, "could not open (%s)\n", tmpname);
	}
	char *lineptr;
	int out_offset;
	int out_len;
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
void writeTemplate (Template * libtemplate, const char *tin, const char *fname, const char *ext, const bool overwrite)
{
	char *template_dir = "templates";
	char inname[256];
	strcpy (inname, template_dir);
	strcat (inname, "/");
	strcat (inname, tin);
	char outname[256];
	strcpy (outname, fname);
	strcat (outname, ext);

	FILE *in = fopen (inname, "r");
	if (in == NULL) {
		fprintf (stderr, "could not open (%s)\n", inname);
	}
	FILE *exists = fopen (outname, "r");
	if (exists) {
		fclose (exists);
		if (!overwrite) {
			fprintf (stderr, "not overwriting %s\n", outname);
			return;
		}
	}
	fprintf(stderr, "%s -> %s\n", inname, outname);
	FILE *out = fopen (outname, "w");
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
void html2cgi (const char *basename, const char *htmlfile)
{
	FILE *htmlin = fopen (htmlfile, "r");
	parse (htmlin);
	fclose (htmlin);
	template_init();
	Template *libtemplate = template_new ();
	template_addkeyvalue (libtemplate, "name", basename);
	char *upname = strdup (basename);
	uppercase(upname);

	template_addkeyvalue (libtemplate, "upname", upname);
	writeTemplate (libtemplate, "impl.cc", basename, ".cc", false);
	writeTemplate (libtemplate, "impl.h", basename, ".h", false);
	writeTemplate (libtemplate, "impl_docgi.cc", basename, "_docgi.cc", true);
	template_destroy(libtemplate);
	template_shutdown();
	free(upname);
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
	if (!gencp_init ()) {
					exit(1);
	}
	printf ("%s\n", htmlfile);
	char basename[FILENAME_MAX];
	get_basename (htmlfile, basename);
	printf ("%s\n", basename);
	html2cgi (basename, htmlfile);
}
