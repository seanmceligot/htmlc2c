#include <getopt.h>
#include <stdio.h>
#include "htmlc2c_util.h"
#include "stringutil.h"
#include "gencp.h"

static char basename[FILENAME_MAX];
static char htmlfile[FILENAME_MAX];
int gencp_verbose;
int gencp_fastcgi;
int gencp_overwrite;
extern int template_verbose;

void usage ();

static int
parse_args (int argc, char **argv)
{
  int c;

  //init global variables
  *basename = 0;

  for (;;)
    {
      static struct option long_options[] = {
        {"verbose", no_argument, &gencp_verbose, 1},
        {"brief", no_argument, &gencp_verbose, 0},
        {"template-verbose", no_argument, &template_verbose, 1},
        {"template-brief", no_argument, &template_verbose, 0},
        {"fastcgi", no_argument, &gencp_fastcgi, 1},
        {"no-fastcgi", no_argument, &gencp_fastcgi, 0},
        {"basename", required_argument, NULL, 'n'},
        {"template-dir", required_argument, NULL, 't'},
        {"overwrite", no_argument, &gencp_overwrite, 'o'},
        {"help", no_argument, NULL, '?'},
        {NULL, 0, NULL, 0}
      };
      /* getopt_long stores the option index here. */
      int option_index = 0;
      c = getopt_long (argc, argv, "vfn:t:?", long_options, &option_index);
      if (c == -1)
        {
          break;
        }
      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          debug ("option %s", long_options[option_index].name);
          if (optarg)
            debug (" with arg %s", optarg);
          debug ("\n");
          break;
        case 't':
          gencp_set_template_dir (optarg);
          break;
        case 'n':
          safe_strncpy (basename, optarg, FILENAME_MAX);
          break;
        case 'v':
          gencp_verbose = TRUE;
          break;
        case 'f':
          gencp_fastcgi = TRUE;
        case 'h':
          usage ();
          return FALSE;
        case '?':
          usage ();
          return FALSE;
        default:
          return FALSE;
        }                       // switch
    }                           //while
  if (optind < argc)
    {
      safe_strncpy (htmlfile, argv[optind++], FILENAME_MAX);
    }
  gencp_be_verbose (gencp_verbose);
  gencp_always_overwrite (gencp_overwrite);
  gencp_use_fastcgi (gencp_fastcgi);
  return TRUE;
}

void
usage ()
{
  printf ("\
Usage: gencp [OPTION] [TEMPLATE_FILE]\n\
Generate c from html and embeded c\n\
\n\
  -n, --basename basename for output files\n\
  -t, --template-dir  locate of impl.c, etc...\n\
  -v, --verbose  for debugging\n\
  -?  --help        display this help and exit\n\
      --version     output version information and exit\n\
\n\
");
}

int
main (int argc, char **argv)
{
  if (!gencp_init ())
    {
      abort ();
    }
  if (!parse_args (argc, argv))
    {
      return 1;
    }
  debug ("htmlfile: %s\n", htmlfile);
  if (!(*basename))
    {
      gencp_start (htmlfile);
    }
  else
    {
      debug ("basename: %s\n", basename);
      gencp (basename, htmlfile);
    }
  return 0;
}
