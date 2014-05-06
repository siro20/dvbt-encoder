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

#ifndef _DVBTENC_H__
#define _DVBTENC_H__

#include <iostream>
#include <fstream>

#include "ed.hpp"
#include "rs.hpp"
#include "ce.hpp"
#include "ii.hpp"
#include "sm.hpp"
#include "si.hpp"
#include "ifft.hpp"
#include "oi.hpp"
#include "chan.hpp"
#include "settings.hpp"

using namespace std;

typedef void(*funcs)(FILE*,FILE*,DVBT_settings*);

class DVBT_enc
{
public:
	DVBT_enc(FILE *in, FILE *out, DVBT_settings* dvbt_settings);
	void encode();
	~DVBT_enc();
private:
	char* randomString();
	int next_fd;
	FILE* out;
	DVBT_settings* dvbt_settings;
};

#endif



