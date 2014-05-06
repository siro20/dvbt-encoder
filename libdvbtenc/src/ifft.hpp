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

#ifndef _DVBTENC_IFFT_H__
#define _DVBTENC_IFFT_H__

extern "C" {
#include <fftw3.h>
}
#include <iostream>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include "memory.hpp"
#include "settings.hpp"


using namespace std;

class DVBT_ifft
{
public:
	DVBT_ifft(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings);
	~DVBT_ifft();
	int encode();
private:
	FILE *fd_in;
	FILE *fd_out;
	void fftshift( dvbt_complex_t *in, dvbt_complex_t *out );
	int in_multiple_of;
	int out_multiple_of;
	DVBT_memory *mem;
	DVBT_settings* dvbt_settings;
	fftwf_plan p;
	dvbt_complex_t *buf;
	dvbt_complex_t *tmp;
};

#endif
