/*
*
*    compares two files containing floatingpoint values using a given margin
*    Copyright (C) 2014  Patrick Rudolph <siro@das-labor.org>
*
*    This program is free software; you can redistribute it and/or modify it under the terms 
*    of the GNU General Public License as published by the Free Software Foundation; either version 3 
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
*    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License along with this program; 
*    if not, see <http://www.gnu.org/licenses/>.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
	FILE *source, *dest;
	int l1,l2;
	float s,d,distance;
	int ret,l;

	if( argc < 4 )
	{
		printf("to less arguments given\n%s <file1> <file2> <distance>\n",argv[0]);
		return EXIT_FAILURE;
	}
	if( (source=fopen(argv[1],"r")) == NULL) {
		printf("failed to open %s\n",argv[1]);
		return EXIT_FAILURE;
	}
	if( (dest=fopen(argv[2],"r")) == NULL) {
		printf("failed to open %s\n",argv[2]);
		return EXIT_FAILURE;
	}
	distance = atof(argv[3]);
	l = 0;
	while(1){
		ret = fread(&s,sizeof(float),1,source);
		if(ret <= 0)
			break;
			
		ret = fread(&d,sizeof(float),1,dest);
		if(ret <= 0)
			break;
			
		if( fabsf(s-d) > distance )
		{
			printf("mismatch item %d %f != %f\n", l, s, d);
			fclose(source);
			fclose(dest);
			return EXIT_FAILURE;
		}
		l++;
	}
	printf("no errors found in %d items\n",l);
	fclose(source);
	fclose(dest);
	return 0;
}
