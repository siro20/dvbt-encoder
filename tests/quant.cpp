#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h>

#include "quant.hpp"
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
	);
	exit(0);
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
	DVBT_settings *dvbtsettings;

	oversampling = 1;
	alpha = 1;
	cellid = 0;
	gain = 1.0f;
	
	opterr = 0;
	while ((opt = getopt(argc, argv, "to:b:c:g:m:a:i:s:f:z:")) != -1)
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
        break;
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
	DVBT_quant dvbtquant(stdin,stdout,dvbtsettings);

	while(!dvbtquant.encode())
	{};

	return 0;
}

