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

#ifndef _DVBTENC_MEMORY_H__
#define _DVBTENC_MEMORY_H__

#include <iostream>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include <stdexcept>

using namespace std;

class DVBT_memory
{
public:
	DVBT_memory( int in_multiple_of, int out_multiple_of, bool fixed );
	DVBT_memory( int in_multiple_of, int out_multiple_of );
	~DVBT_memory();
	int read(FILE* fd);
	int write(FILE* fd);
	uint8_t *in;
	uint8_t *out;
	int in_size;
	int out_size;
	int in_multiple_of;
	int out_multiple_of;
private:

protected:
};

#define DVBTENC_APROX_BUF_SIZE 1024

#endif
