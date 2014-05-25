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
	float bitrate, transfersize, transfertime;
	int ret;
	unsigned char buf[1024*4];
	struct timeval t1,t2;
	transfersize = 0;
	gettimeofday(&t1, NULL);
	while(1){
		ret = fread(buf,sizeof(char),sizeof(buf),stdin);
		if(ret <= 0)
			break;
			
		ret = fwrite(buf,sizeof(char),sizeof(buf),stdout);
		if(ret <= 0)
			break;
		transfersize += ret;
	}
	gettimeofday(&t2, NULL);
	transfertime = (t2.tv_sec*1000000+t2.tv_usec)-(t1.tv_sec*1000000+t1.tv_usec);
	bitrate = transfersize / transfertime; //in MB/s
	fprintf(stderr, "took %f seconds to transfer %f MB.\n", transfertime/1000000, transfersize/1000000);
        fprintf(stderr, "that's %f MByte/s or %f MBit/s.\n", bitrate, bitrate*8);
                       
	return 0;
}
