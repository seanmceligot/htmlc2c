#include <string.h>
#include <stdio.h>

/*
	 * replace matchlen chars at start with changeto
	 * start with grow or shift by strlen(changeto) - deletelen
	 */
int
replace_insert (char *start, unsigned int deletelen, char *changeto, int
		maxgrow, char *tempbuf)
{
  int stringlen = strlen (start);
  unsigned int changelen = strlen (changeto);
  int diff = changelen - deletelen;
  int growth = 0;
  if (diff == 0)
    {
      strncpy (start, changeto, changelen);
      if (changelen > stringlen)
	{
	  start[changelen] = 0;
	}
    }
  else
    {
      growth += diff;
      if (growth > maxgrow)
	{
	  fprintf (stderr, "maxlen exceeded\n");
	  return growth;
	}
      else
	{
	  char *endptr = start + deletelen;
	  strcpy (tempbuf, endptr);
	  strcpy (endptr + diff, tempbuf);
	  strncpy (start, changeto, changelen);
	  if (changelen > stringlen)
	    {
	      start[changelen] = 0;
	    }
	}
    }
  return growth;

}

/*
	* tempbuf must hold maxlen chars (a little less actually)
*/
int
replace (char *orig, char *find, char *changeto, int maxlen, char *tempbuf)
{
  int count = 0;
  char *ptr = orig;
  int matchlen = strlen (find);
  int changelen = strlen (changeto);
  int textlen = strlen (orig);
  int maxgrow = maxlen - textlen;
  char *match = strstr (ptr, find);
  while (match)
    {
      ptr = match;
      maxgrow -= replace_insert (ptr, matchlen, changeto, maxgrow, tempbuf);
      ptr += changelen;
      match = strstr (ptr, find);
      count++;
    }
  return count;
}

void
safe_strncpy (char *dest, const char *src, int maxlen)
{
  strncpy (dest, src, maxlen);
  dest[maxlen - 1] = 0;
}
