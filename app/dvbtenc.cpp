/*
*
*    DVB-T Encoder written in c++
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "dvbtenc.hpp"
#include "settings.hpp"

void print_help_msg()
{
	fprintf(stderr,
	"o		ofdmmode 2048, 8192\n"
	"b		bandwidth 6,7,8\n"
	"c		coderate 2,3,4,5,7\n"
	"m		modulation 2,4,6\n"
	"g		guard interval 4,8,16,32\n"
	"a		alpha 1\n"
	"i		cellid (0-65535)\n"
	"v		oversampling 1,2,4,8,16\n"
	"s		outputformat\n"
	"		0 CHAR\n"
	"		1 UCHAR\n"
	"		2 SHORT\n"
	"		3 USHORT\n"	
	"		4 FLOAT\n"
	"f		gain\n"
	"h		help message\n"
	"p		print mpegts bitrate\n"
	"z		print samplerate\n"
	"x		benchmark\n"
	);
	exit(0);
}

static void read_thread(DVBT_pipe *pin, bool benchmark)
{
	struct timeval t1,t2;
	unsigned long bytecnt = 0;
	if(benchmark)
		gettimeofday(&t1, NULL);
	
	bool err = false;
	do{
		DVBT_memory *mem = new DVBT_memory( 0xFFFF );
		unsigned int ret, offset;
		offset = 0;
		ret = 0;
		
		while(offset < mem->size)
		{
			ret = fread(mem->ptr+offset, sizeof(char), mem->size-offset, stdin);
			if(ret <= 0){
				err = true;
				break;
			}
			offset += ret;
		}
		bytecnt += offset;
		
		if(offset < mem->size){
			memset(mem->ptr+offset, 0, mem->size-offset);
			err = true;
		}
		if(!pin->write(mem))
			err = true;
	}
	while(!err);

	if(benchmark){
		gettimeofday(&t2, NULL);
		unsigned long diff = t2.tv_sec*1000000+t2.tv_usec-t1.tv_sec*1000000+t1.tv_usec;
		cerr << "took " << diff << " usec to encode " << bytecnt << " Bytes\nthat's " << (bytecnt*1000000)/(diff*188) << " MPEGts packets/ sec" << endl;
	}
	
	pin->CloseWriteEnd();
}

static void write_thread(DVBT_pipe *pout)
{
	pout->initReadEnd( 0xFFFF );

	bool err = false;
	do{                
		DVBT_memory *mem = pout->read();
		if(!mem || !mem->size)
		{
			err = true;
			break;
		}

		unsigned int ret, offset;
		offset = 0;
		ret = 0;
		while(offset < mem->size)
		{
			ret = fwrite(mem->ptr+offset, sizeof(char), mem->size-offset, stdout);
			if(ret <= 0){
				err = true;
				break;
			}
			offset += ret;
		}
		//write failed ? just exit
		if(offset < mem->size){
			err = true;
		}
		delete mem;
	}
	while(!err);
	pout->CloseReadEnd();
}

/* test function */
int main(int argc, char *argv[])
{
	
	int ofdmmode;
	int bandwidth;
	int coderate;
	int guardinterval;
	int modulation;
	int alpha;
	int cellid;
	int oversampling;
	int outputformat;
	float gain;
	int opt;
	bool benchmark;
	DVBT_settings *dvbtsettings;
	bool print_mpegtsbitrate;
	bool print_samplerate;
	
	ofdmmode = 2048;
	bandwidth = 8;
	coderate = 3;
	guardinterval = 4;
	modulation = 4;
	oversampling = 1;
	alpha = 1;
	cellid = 0;
	gain = 1.0f;
	outputformat = 4;
	benchmark = false;

	print_mpegtsbitrate = false;
	print_samplerate = false;
	
	opterr = 0;
	while ((opt = getopt(argc, argv, "xto:b:c:g:m:a:i:s:f:v:zp?h")) != -1)
	{
		switch (opt)
		{
		case 'o':
			ofdmmode = atoi(optarg);
			break;
		case 'b':
			bandwidth = atoi(optarg);
			break;
		case 'c':
			coderate = atoi(optarg);
			break;
		case 'g':
			guardinterval = atoi(optarg);
			break;
		case 'm':
			modulation = atoi(optarg);
			break;
		case 'a':
			alpha = atoi(optarg);
			break;
		case 'i':
			cellid = atoi(optarg);
			break;
		case 'v':
			oversampling = atoi(optarg);
			break;
		case 's':
			outputformat = atoi(optarg);
			break;
		case 'f':
			gain = atof(optarg);
			break;
		case 'h':
			print_help_msg();
			break;
		case '?':
			fprintf(stderr, "unknown arg %c\n", optopt);
			return 1;
			break;
		case 'p':
			print_mpegtsbitrate = true;
			break;
		case 'z':
			print_samplerate = true;
			break;
		case 'x':
			benchmark = true;
			break;
		default:
			print_help_msg();
		}
	}

	try {
		dvbtsettings = new DVBT_settings(ofdmmode, bandwidth, coderate, guardinterval, modulation, alpha, cellid, oversampling, static_cast<dvbt_data_formats>(outputformat), gain);
	}
	catch(...)
	{
		fprintf(stderr,"failed to init DVBT_Settings\n");
		return 1;
	}
	if(print_mpegtsbitrate){
		cout << (unsigned long)(dvbtsettings->mpegtsbitrate) << endl;
		return 0;
	}
	else if(print_samplerate){
		cout << (unsigned long)(dvbtsettings->symbolrate*dvbtsettings->oversampling) << endl;
		return 0;
	}

	DVBT_pipe *pin = new DVBT_pipe("pin");
	DVBT_pipe *pout = new DVBT_pipe("pout");
	std::thread wt(write_thread, pout);
	std::thread rt(read_thread, pin, benchmark);
	
	DVBT_enc dvbtenc(pin,pout,dvbtsettings);

	dvbtenc.encode();
		
	rt.join();
	wt.join();
	delete pin;
	delete pout;
	return 0;
}

