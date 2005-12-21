//
// Copyright (c) 2005 -- Daniel Wallner
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int worldgen(char *path, double latitude, double longitude, double scale)
{
  FILE *outfile;

  outfile = fopen(path, "w");

  if (!outfile)
    {
      fprintf(stderr, "unable to open output file '%s': %s\n",
	      path,strerror(errno));
      return 0;
    }

  fprintf(outfile, "%lf\n0\n0\n%lf\n", scale, -scale);
  fprintf(outfile, "%lf\n", longitude);
  fprintf(outfile, "%lf\n", latitude);

  fclose(outfile);

  return 1;
}

int koordread(char *path)
{
  printf( " koordread: %s\n", path);

  FILE *infile = fopen(path, "r");

  char mapname[264];
  double latitude;
  double longitude;
  double scale;
  long count;
  long linenumber = 0;

  if (!infile)
    {
      fprintf(stderr, "unable to open input file '%s': %s\n",
	      path,strerror(errno));
      return 1;
    }

  while ((count = fscanf(infile, "%s %lf %lf %lf\n",
			 mapname, &latitude, &longitude, &scale)) &&
    	 count != EOF)
    {
      linenumber++;

      if (count != 4)
	{
          fprintf(stderr, "Error reading line %ld\n", linenumber);
          fprintf(stderr, "   map name: '%s'\n", mapname);
          continue;
	}

      char worldname[264];
      char *extchar;
      int is_image;

      fprintf(stderr, "Line: '%s' '%lf' '%lf' '%lf'\n",
	      mapname, latitude, longitude, scale);

      strcpy(worldname, mapname);
      extchar = strrchr(worldname, '.');

      is_image =
	strstr(worldname, ".jpg") ||
	strstr(worldname, ".png") ||
	strstr(worldname, ".gif");

      if (extchar && is_image)
	{
	  fprintf(stderr, " is image with extention '%s'\n",extchar);

	  strcpy(extchar, ".wld");

	  fprintf(stderr, " generating '%s'\n", worldname);

	  worldgen(worldname, latitude, longitude, scale);
	}
      else
	{
	  fprintf(stderr, " no image extension, skipping '%s'\n", mapname);
	}
    }

  fclose(infile);

  return 0;
}

int main(int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: worldgen infile\n");
      return 1;
    }

  return koordread(argv[1]);
}
