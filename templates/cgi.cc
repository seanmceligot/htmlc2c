#include <stdio.h>

%(header)s

void init() {
}
// <Top2>
void docgi ()
{
	puts ("Content-type: text/html\n");
// </Top2>

${i:docgitmp}
}

void shutdown() {
}


int main (int argc, char *argv[])
{
	init ();
	docgi ();
	shutdown ();
}
