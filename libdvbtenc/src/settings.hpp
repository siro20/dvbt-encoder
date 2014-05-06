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

#ifndef _DVBTENCODER_SETTINGS_H__
#define _DVBTENCODER_SETTINGS_H__

#include <iostream>
#include <cstdio>
#include <stdexcept>

using namespace std;

enum dvbt_data_formats{
	CHAR,
	UCHAR,
	SHORT,
	USHORT,
	FLOAT
};

typedef struct dvbt_complex{
	float x;
	float y;
}dvbt_complex_t;

typedef struct { char x; char y; } dvbt_complex_char_t;
typedef struct { unsigned char x; unsigned char y; }  dvbt_complex_uchar_t;
typedef struct { short int x; short int y; }  dvbt_complex_short_t;
typedef struct { unsigned short int  x; unsigned short int  y; }  dvbt_complex_ushort_t;

class DVBT_settings
{
public:
	DVBT_settings(int ofdmmode, int bandwidth, int coderate, int guardinterval, int modulation, int alpha, int cellid, int oversampling, dvbt_data_formats outputformat, float gain);
	~DVBT_settings();
	float mpegtsbitrate;
	float symbolrate;
	int ofdmsymbolspersuperframe;
	int tspacketspersuperframe;
	int bytespersuperframe;
	int bitspersuperframe;
	int guardcarriers;
	int ofdmcarriers;
	int ofdmuseablecarriers;
	int outputformat;
	int oversampling;
	int alpha;
	int guardinterval;
	int coderate;
	int cellid;
	int modulation;
	int bandwidth;
	int ofdmmode;
	float gain;
	float normalisation;
	int DVBT_MPEG_BYTES_RS_PACKET;
	int DVBT_MPEG_BYTES_TS_PACKET;
	int DVBT_SYMBOLS_FRAME;
	int DVBT_FRAMES_SUPERFRAME;
	int DVBT_II_DEPTH;
private:
protected:

};

#endif
