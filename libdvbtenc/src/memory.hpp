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
#include <thread>
#include <queue>
#include <condition_variable>
#include <chrono>

using namespace std;

class DVBT_memory
{
public:
	~DVBT_memory();
	DVBT_memory( FILE *fd_i, FILE *fd_o, int in_multiple_of, int out_multiple_of, bool fixed );
	int in_size;
	int out_size;
	int in_multiple_of;
	int out_multiple_of;
	void free_out(uint8_t *ptr);
	uint8_t* get_out();
	void free_in(uint8_t *ptr);
	uint8_t* get_in();

private:
	bool write(uint8_t *ptr);
	bool read(uint8_t *ptr);
	void read_thread();
	void write_thread();
	thread rt;
	thread wt;
	FILE *fd_in;
	FILE *fd_out;
	queue<uint8_t *> in_data;
	queue<uint8_t *> in_free;
	queue<uint8_t *> out_data;
	queue<uint8_t *> out_free;
	mutex in_mutex;
	mutex out_mutex;
	mutex in_signal_mutex;
	mutex out_signal_mutex;
	condition_variable in_cond_var;
	condition_variable out_cond_var;
	bool readerr;
	bool writeerr;
};

#define DVBTENC_APROX_BUF_SIZE 1024
#define DVBTENC_BUFFERS 5

#endif
