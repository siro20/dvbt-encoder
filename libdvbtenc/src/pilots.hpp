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

#ifndef _DVBTENCODER_PILOTS_H__
#define _DVBTENCODER_PILOTS_H__

#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include "settings.hpp"
#include "tps.hpp"

using namespace std;

class DVBT_pilots
{
public:
	DVBT_pilots(int frame, int symbol, DVBT_tps *dvbt_tps, DVBT_settings *dvbt_settings);
	~DVBT_pilots();
	
	void encode(dvbt_complex_t *in, dvbt_complex_t *out);

	int LEN_TPSPILOTS_2K;
	int LEN_TPSPILOTS_8K;
	int LEN_CONTPILOTS_2K;
	int LEN_CONTPILOTS_8K;
	int LEN_SCATPILOTS_2K;
	int LEN_SCATPILOTS_8K;
private:
	void generate_prbs(unsigned char *d_wk, int ofdmcarriers);

	DVBT_settings *dvbt_settings;
	int *data_pointer;
	dvbt_complex_t *channels;
};

#endif
