/*
*
*    simulates a fixed bandwidth channel
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
#include <sys/time.h>

int main(int argc, char *argv[]) {
	long bitrate;
	int ret;
	long waittime;
	unsigned char buf[188];
	struct timeval t1,t2;
	if( argc < 2 )
	{
		printf("to less arguments given\n%s <bitrate>\n",argv[0]);
		return EXIT_FAILURE;
	}
	bitrate = atol(argv[1]);
	
	//bitrate/8 == byte per second
	//byte per second / 188 == mpeg ts packets per second
	//time between packets == 1/(bitrate/(8*188)) == 8*188/bitrate [s]
	waittime = (long)(8*188*1000000/bitrate);
	fprintf(stderr, "using bitrate %ld, %ld usecs between packets\n",bitrate, waittime);
	gettimeofday(&t1, NULL);
	while(1){
		ret = fread(buf,sizeof(char),188,stdin);
		if(ret <= 0)
			break;
			
		ret = fwrite(buf,sizeof(char),188,stdout);
		if(ret <= 0)
			break;
		
		while(1){
			long diff;
			gettimeofday(&t2, NULL);
			diff = t2.tv_sec*1000000+t2.tv_usec-waittime-t1.tv_sec*1000000+t1.tv_usec;
			if(diff >= 0){
				break;
			}
			usleep(-diff);
		}
		gettimeofday(&t1, NULL);
	}

	return 0;
}
