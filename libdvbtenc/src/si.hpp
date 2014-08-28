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

#ifndef _DVBTENC_SI_H__
#define _DVBTENC_SI_H__

#include <iostream>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include "memory.hpp"
#include "settings.hpp"

using namespace std;

class DVBT_si
{
public:
	DVBT_si(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings);
	~DVBT_si();
	bool encode(int symbol);
private:
	unsigned int mReadSize;
	unsigned int mWriteSize;
	DVBT_pipe *pin;
	DVBT_pipe *pout;
	DVBT_settings* dvbt_settings;
	int *Hq;
};

#endif
