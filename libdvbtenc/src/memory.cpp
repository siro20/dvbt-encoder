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

#include <iostream>
#include "memory.hpp"
#include <unistd.h>

using namespace std;

static bool debug = false;
//might block
uint8_t* DVBT_memory::get_in()
{
	uint8_t *ptr = this->inQueue.front();
	this->inQueue.pop();
	
	if(ptr==NULL)
	{
		//notify write thread
		this->outQueue.push(NULL);
		this->rt.join();
		this->wt.join();
	}

	return ptr;
}

//might block
void DVBT_memory::free_out(uint8_t* ptr)
{
	this->outQueue.push(ptr);
}

uint8_t* DVBT_memory::get_out()
{
	return new uint8_t[this->out_size];
}

void DVBT_memory::free_in(uint8_t *ptr)
{
	if(ptr)
		delete[] ptr;
}

bool DVBT_memory::write(uint8_t *ptr)
{
	int ret,offset;
	bool err;
	offset = 0;
	err = false;
	ret = 0;

	while(offset < this->out_size)
	{
		ret = fwrite(ptr+offset, sizeof(char), this->out_size-offset, this->fd_out);
		if(ret <= 0){
			err = true;
			break;
		}
		offset += ret;
	}
	//write failed ? just exit
	if(offset < this->out_size){
		err = true;
	}
	return err;
}

bool DVBT_memory::read(uint8_t *ptr)
{
	int ret,offset;
	bool err;
	offset = 0;
	err = false;
	ret = 0;
	while(offset < this->in_size)
	{
		ret = fread(ptr+offset, sizeof(char), this->in_size-offset, this->fd_in);
		if(ret <= 0){
			err = true;
			break;
		}
		offset += ret;
	}
	
	if(offset < this->in_size){
		memset(ptr+offset, 0, this->in_size-offset);
		err = true;
	}
	return err;
}

void DVBT_memory::read_thread()
{
	uint8_t *ptr;
	bool err;
	do{
		ptr = new uint8_t[this->in_size];
		err = this->read(ptr);
		
		this->inQueue.push(ptr);
		if(err)
		{
			this->inQueue.push(NULL);
		}

		if(debug) std::cerr << "read_thread notify" << endl;
	}
	while(!err);

	if(debug) std::cerr << "read_thread terminated" << endl;
}

void DVBT_memory::write_thread()
{
	uint8_t *ptr;
	bool err;
	do{
		ptr = this->outQueue.front();
		this->outQueue.pop();

		if(ptr == NULL)
			break;
		
		err = this->write(ptr);
		delete[] ptr;
	}
	while(!err);

	if(debug) std::cerr << "write_thread terminated" << endl;
}

DVBT_memory::DVBT_memory( FILE *fd_i, FILE *fd_o, int in_multiple_of, int out_multiple_of, bool fixed )
{
	int i;
	if(fixed)
	{
		i = 1;
	}
	else
	{
		i = DVBTENC_APROX_BUF_SIZE / in_multiple_of;
		if(!i) i=1;
	}
	this->fd_in = fd_i;
	this->fd_out = fd_o;
	this->in_size = i * in_multiple_of;
	this->out_size = i * out_multiple_of;
	this->in_multiple_of = in_multiple_of;
	this->out_multiple_of = out_multiple_of;

	this->rt = thread(&DVBT_memory::read_thread, this);
	this->wt = thread(&DVBT_memory::write_thread, this);
}

DVBT_memory::~DVBT_memory()
{
}
