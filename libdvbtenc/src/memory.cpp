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

using namespace std;

DVBT_memory::DVBT_memory( int in_multiple_of, int out_multiple_of )
{
	int i;
	i = DVBTENC_APROX_BUF_SIZE / in_multiple_of;
	if(!i) i=1;
	this->in_size = i * in_multiple_of;
	this->out_size = i * out_multiple_of;
	this->in = new uint8_t[this->in_size];
	this->out = new uint8_t[this->out_size];
	memset(this->in, 0, this->in_size);
	memset(this->out, 0, this->out_size);
	this->in_multiple_of = in_multiple_of;
	this->out_multiple_of = out_multiple_of;
}

DVBT_memory::DVBT_memory( int in_multiple_of, int out_multiple_of, bool fixed )
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
	this->in_size = i * in_multiple_of;
	this->out_size = i * out_multiple_of;
	this->in = new uint8_t[this->in_size];
	this->out = new uint8_t[this->out_size];
	this->in_multiple_of = in_multiple_of;
	this->out_multiple_of = out_multiple_of;
}

DVBT_memory::~DVBT_memory( )
{
	delete[] this->in;
	delete[] this->out;
}

int DVBT_memory::write(FILE *fd)
{
	int ret,offset,err;
	offset = 0;
	err = 0;
	ret = 0;
	if(!fd)
		throw std::runtime_error("DVBT_memory::write invalid file descriptor");

	while(offset < this->out_size)
	{
		ret = fwrite(this->out+offset, sizeof(char), this->out_size-offset, fd);
		if(ret <= 0){
			err = 1;
			break;
		}
		offset += ret;
	}
	//write failed ? just exit
	if(offset < this->out_size){
		err = 1;
	}
	return err;
}


int DVBT_memory::read(FILE* fd)
{
	int ret,offset,err;
	offset = 0;
	err = 0;
	ret = 0;
	if(!fd)
		throw std::runtime_error("DVBT_memory::read invalid file descriptor");
	while(offset < this->in_size)
	{
		ret = fread(this->in+offset, sizeof(char), this->in_size-offset, fd);
		if(ret <= 0){
			err = 1;
			break;
		}
		offset += ret;
	}
	
	if(offset < this->in_size){
		memset(this->in+offset, 0, this->in_size-offset);
		err = 1;
	}
	return err;
}
