/*
	 * replace matchlen chars at start with changeto
	 * start with grow or shift by strlen(changeto) - deletelen
	 */
int
replace_insert (char *start, unsigned int deletelen, char *changeto, int
		maxgrow, char *tempbuf);
/*
	* tempbuf must hold maxlen chars (a little less actually)
*/
int
replace (char *orig, char *find, char *changeto, int maxlen, char *tempbuf);

void safe_strncpy (char *dest, const char *src, int maxlen);
