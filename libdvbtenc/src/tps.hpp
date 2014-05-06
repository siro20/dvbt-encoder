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

#ifndef _DVBTENCODER_TPS_H__
#define _DVBTENCODER_TPS_H__

#include <iostream>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <stdexcept>
#include "settings.hpp"

using namespace std;

class DVBT_tps
{
public:
	DVBT_tps(DVBT_settings *dvbt_settings);
	~DVBT_tps();
	uint8_t tpsarray[4][68];
	float dpsktpsarray[4][68];
private:
	void generate_bch_code(unsigned char d_tps_data[67]);
};

#endif
