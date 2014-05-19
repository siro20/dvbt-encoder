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
	uint8_t *ptr;
	//this mutex is locked as long as this->in_data.size() == 0
	if( this->in_data.size() == 0 ){
		std::unique_lock<std::mutex> l(this->in_signal_mutex);
		while( this->in_data.size() == 0 ){
			in_wait_empty.wait(l);
		}
	}
	this->in_mutex.lock();
	ptr = this->in_data.front();
	this->in_data.pop();
	this->in_mutex.unlock();
	
	if(ptr==NULL)
	{
		//notify write thread
		this->out_mutex.lock();
		this->out_data.push(NULL);
		this->out_mutex.unlock();
		this->rt.join();
		this->wt.join();
	}
	if(debug) std::cerr << "get_in new in_data.size " << this->in_data.size() << endl;
	return ptr;
}

void DVBT_memory::free_in(uint8_t *ptr)
{
	this->in_mutex.lock();
	this->in_free.push(ptr);
	this->in_mutex.unlock();
	if(debug) std::cerr << "free_in new in_free.size " << this->in_free.size() << endl;
}

//might block
uint8_t* DVBT_memory::get_out()
{
	uint8_t *ptr;
	if( this->out_free.size() == 0 ){
		std::unique_lock<std::mutex> l(this->out_signal_mutex);
		while( this->out_free.size() == 0 ){
			out_wait_empty.wait(l);
		}
	}
	this->out_mutex.lock();
	ptr = this->out_free.front();
	this->out_free.pop();
	this->out_mutex.unlock();
	if(debug) std::cerr << "get_out new out_free.size " << this->out_free.size() << endl;

	return ptr;
}

void DVBT_memory::free_out(uint8_t *ptr)
{
	this->out_mutex.lock();
	this->out_data.push(ptr);
	this->out_mutex.unlock();
	if(debug) std::cerr << "free_out new out_data.size " << this->out_data.size() << endl;
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
		while(this->in_free.size() == 0)
		{
			usleep(100);
		}
		
		this->in_mutex.lock();
		ptr = this->in_free.front();
		this->in_free.pop();
		this->in_mutex.unlock();
		err = this->read(ptr);
		
		this->in_mutex.lock();
		this->in_data.push(ptr);
		if(err)
		{
			this->in_data.push(NULL);
		}
		this->in_mutex.unlock();
		
		{
			std::unique_lock<std::mutex> l(this->in_signal_mutex);
			in_wait_empty.notify_one();
		}
		if(debug) std::cerr << "read_thread notify" << endl;
	}
	while(!err);
	std::unique_lock<std::mutex> l(this->in_signal_mutex);
	in_wait_empty.notify_one();
	if(debug) std::cerr << "read_thread terminated" << endl;
}

void DVBT_memory::write_thread()
{
	uint8_t *ptr;
	bool err;
	do{
		while(this->out_data.size() == 0)
		{
			usleep(100);
		}
		
		this->out_mutex.lock();
		ptr = this->out_data.front();
		this->out_data.pop();
		this->out_mutex.unlock();
		
		if(ptr == NULL)
			break;
		
		err = this->write(ptr);
		
		this->out_mutex.lock();
		this->out_free.push(ptr);
		this->out_mutex.unlock();
		
		{
			std::unique_lock<std::mutex> l(this->out_signal_mutex);
			out_wait_empty.notify_one();
		}
		if(debug) std::cerr << "write_thread notify" << endl;

	}
	while(!err);
	std::unique_lock<std::mutex> l(this->out_signal_mutex);
	out_wait_empty.notify_one();
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

	for(i=0;i<DVBTENC_BUFFERS;i++)
	{
		this->in_free.push(new uint8_t[this->in_size]);
		this->out_free.push(new uint8_t[this->out_size]);
	}

	this->rt = thread(&DVBT_memory::read_thread, this);
	this->wt = thread(&DVBT_memory::write_thread, this);
}

DVBT_memory::~DVBT_memory()
{
	for(int i=0;i<this->in_free.size();i++)
	{
		delete[] this->in_free.front();
		this->in_free.pop();
	}
	for(int i=0;i<this->out_free.size();i++)
	{
		delete[] this->out_free.front();
		this->out_free.pop();
	}
}
