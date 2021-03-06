#include <stdio.h>
#include "config.h"

/**
 * A little utility function to generate header info.
 *
 * Yes, it would be possible to do this using more "native" autoconf
 * features, but I personally find this approach to be cleaner.
 *
 * The output of this program is generally written to art_config.h,
 * which is installed in libart's include dir.
 **/

static void
die (char *why)
{
  fprintf (stderr, "gen_art_config: %s\n", why);
  exit (1);
}

int
main (int argc, char **argv)
{
  printf ("/* Automatically generated by gen_art_config.c */\n"
	  "\n"
	  "#define ART_SIZEOF_CHAR %d\n"
	  "#define ART_SIZEOF_SHORT %d\n"
	  "#define ART_SIZEOF_INT %d\n"
	  "#define ART_SIZEOF_LONG %d\n"
	  "\n",
	  sizeof(char), sizeof(short), sizeof(int), sizeof(long));

  if (sizeof(char) == 1)
    printf ("typedef unsigned char art_u8;\n");
  else
    die ("sizeof(char) != 1");

  if (sizeof(short) == 2)
    printf ("typedef unsigned short art_u16;\n");
  else
    die ("sizeof(short) != 2");

  if (sizeof(int) == 4)
    printf ("typedef unsigned int art_u32;\n");
  else if (sizeof(long) == 4)
    printf ("typedef unsigned long art_u32;\n");
  else
    die ("sizeof(int) != 4 and sizeof(long) != 4");

  return 0;
}
