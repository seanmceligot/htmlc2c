#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pcre.h>
#include <libtemplate.h>
#include "htmlc2c_util.h"
#include "stringutil.h"

#define BUFSIZE 1024

static int gencp_verbose;
static int gencp_fastcgi;
static int gencp_overwrite;
static FILE *out;
static char gencp_template_dir[FILENAME_MAX];
static pcre *code_re;
static pcre *script_re;
static pcre *code_end_re;
static pcre *script_end_re;

extern int template_verbose;

typedef enum {TAG_NONE, TAG_CODE, TAG_SCRIPT} TagType ;

void
gencp_set_template_dir (char *dir)
{
  safe_strncpy (gencp_template_dir, dir, FILENAME_MAX);
  debug ("template_dir set to %s\n", gencp_template_dir);
}

void
gencp_use_fastcgi (BOOL on)
{
  gencp_fastcgi = on;
}

void
gencp_be_verbose (BOOL on)
{
  gencp_verbose = on;
}

void
gencp_always_overwrite (BOOL on)
{
  gencp_overwrite = on;
}
static void
get_basename (const char *filename, char *basename)
{
  char *dot;

  strcpy (basename, filename);
  dot = strrchr (basename, '.');
  if (dot == NULL)
    {
      return;
    }
  *dot = 0;
}

static pcre *
compile (char *pattern)
{
  const char *error;
  int erroffset;
  pcre *re;

  fprintf (stderr, "%s\n", pattern);
  re = pcre_compile (pattern, 0, &error, &erroffset, NULL);
  if (!re)
    {
      fprintf (stderr, "PCRE compilation failed at offset %d: %s\n",
               erroffset, error);
      return NULL;
    }
  return re;
}

BOOL
gencp_init ()
{
  char *code_pattern = "\\<code\\ class=\\\"cgi\\\"\\>";
  char *script_pattern =
    "\\<%";
  char *code_end_pattern = "</code>";
  char *script_end_pattern = "%>";
  code_re = compile (code_pattern);
  if (!code_re)
    {
      return FALSE;
    }
  script_re = compile (script_pattern);
  if (!script_re)
    {
      return FALSE;
    }
  code_end_re = compile (code_end_pattern);
  if (!code_end_re)
    {
      return FALSE;
    }
  script_end_re = compile (script_end_pattern);
  if (!script_end_re)
    {
      return FALSE;
    }
  strcpy (gencp_template_dir, "/usr/share/htmlc2c");
  gencp_verbose = FALSE;
  gencp_fastcgi = FALSE;
  gencp_overwrite = FALSE;
  return TRUE;
}

static char *
nextline (FILE * fin)
{
  static char inbuf[BUFSIZE];
  char *lineptr = fgets (inbuf, BUFSIZE, fin);
  return lineptr;
}

static BOOL
getmatch (char *lineptr, int *out_offset, int *out_len, pcre * re,
          int linelen)
{
  const int RCOUNT = 3;
  int result[RCOUNT];
  int resultcount =
    pcre_exec (re, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
  int end;

  if ((resultcount < 0) && (resultcount != PCRE_ERROR_NOMATCH))
    {
      fprintf (stderr, "Matching error %d (%s)\n", resultcount, lineptr);
    }
  else if (resultcount > 0)
    {
      *out_offset = result[0];
      end = result[1];
      *out_len = end - *out_offset;
      return TRUE;
    }
  return FALSE;
}

static BOOL
getcodeend (char *lineptr, int *out_offset, int *out_len, TagType tag_type)
{
  int linelen = strlen (lineptr);
	if (tag_type == TAG_SCRIPT) {
  	return getmatch (lineptr, out_offset, out_len, script_end_re, linelen);
	} else if (tag_type == TAG_CODE) {
  	return getmatch (lineptr, out_offset, out_len, code_end_re, linelen);
	}
	return FALSE;
}
static BOOL
getcodestart (char *lineptr, int *out_offset, int *out_len)
{
  int linelen = (int) strlen (lineptr);
  if (getmatch (lineptr, out_offset, out_len, code_re, linelen))
    {
      return TAG_CODE;
    }
  if (getmatch (lineptr, out_offset, out_len, script_re, linelen)) {
					return TAG_SCRIPT;
	}
	return TAG_NONE;
}

static void
htmlputc (char ch)
{
  switch (ch)
    {
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
static void
printhtml (char *lineptr)
{
  if (*lineptr == '\n')
    {
      return;
    }
  fputs ("  puts(\"", out);
  while (*lineptr)
    {
      htmlputc (*lineptr);
      lineptr++;
    }
  fputs ("\");\n", out);
}

static void
printnhtml (char *lineptr, int len)
{
  int i;
  fputs ("  fputs(\"", out);
  for (i = 0; i < len; i++)
    {
      htmlputc (*lineptr);
      lineptr++;
    }
  fputs ("\", stdout);\n", out);
}

static void
printcode (char *lineptr)
{
  fprintf (out, "  %s\n", lineptr);
}

static void
printncode (char *lineptr, int len)
{
  int i;
  for (i = 0; i < len; i++)
    {
      fputc (lineptr[i], out);
    }
}
void
parse (FILE * fin)
{
  char *tmpname = "docgi.tmp";
  char *lineptr;
  int out_offset;
  int out_len;
	TagType tag_type;
  out = fopen (tmpname, "w");
  if (out == NULL)
    {
      fprintf (stderr, "could not open (%s)\n", tmpname);
    }
  while ((lineptr = nextline (fin)) != NULL)
    {
      if ((tag_type = getcodestart (lineptr, &out_offset, &out_len)))
        {
          printnhtml (lineptr, out_offset);
          lineptr += out_offset + out_len;
          while (!getcodeend (lineptr, &out_offset, &out_len, tag_type))
            {
              printcode (lineptr);
              lineptr = nextline (fin);
              if (lineptr == NULL)
                {
                  return;
                }
            }
          printncode (lineptr, out_offset);
          lineptr += out_offset + out_len +1;
          printhtml (lineptr);
        }
      else
        {
          printhtml (lineptr);
        }
    }
  fclose (out);
}

static void
writeTemplate (Template * libtemplate, const char *tin, const char *fname,
               const char *ext, BOOL overwrite)
{
  char inname[FILENAME_MAX];
  char outname[FILENAME_MAX];
  FILE *in;
  FILE *exists;

  if (!overwrite)
    {
      overwrite = gencp_overwrite;
    }
  strcpy (inname, gencp_template_dir);
  strcat (inname, "/");
  strcat (inname, tin);
  strcpy (outname, fname);
  strcat (outname, ext);

  in = fopen (inname, "r");
  if (in == NULL)
    {
      fprintf (stderr, "could not open (%s)\n", inname);
    }
  if (!overwrite)
    {
      exists = fopen (outname, "r");
      if (exists)
        {
          fclose (exists);
          fprintf (stderr, "not overwriting %s\n", outname);
          return;
        }
    }
  fprintf (stderr, "%s -> %s\n", inname, outname);
  out = fopen (outname, "w");
  if (out == NULL)
    {
      fprintf (stderr, "could not open (%s)\n", outname);
    }
  template_parse (libtemplate, in, out);
}

static void
uppercase (char *s)
{
  while (*s != 0)
    {
      if (islower (*s))
        {
          *s = *s - ('a' - 'A');
        }
      s++;
    }
}
void
gencp (const char *basename, const char *htmlfile)
{
  Template *libtemplate;
  char *upname;
  FILE *htmlin = fopen (htmlfile, "r");

  parse (htmlin);
  fclose (htmlin);
  template_init ();
  libtemplate = template_new ();
  template_addkeyvalue (libtemplate, "name", basename);
  upname = strdup (basename);
  uppercase (upname);

  template_addkeyvalue (libtemplate, "upname", upname);
  writeTemplate (libtemplate, "impl.c", basename, ".c", FALSE);
  writeTemplate (libtemplate, "impl.h", basename, ".h", FALSE);
  writeTemplate (libtemplate, "impl_docgi.c", basename, "_docgi.c", TRUE);
  template_destroy (libtemplate);
  template_shutdown ();
  free (upname);
}

void
gencp_start (const char *htmlfile)
{
  char basename[FILENAME_MAX];
  printf ("%s\n", htmlfile);
  get_basename (htmlfile, basename);
  printf ("%s\n", basename);
  gencp (basename, htmlfile);
}
