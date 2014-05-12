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

//might block
uint8_t* DVBT_memory::get_in()
{
	std::unique_lock<std::mutex> lock(this->in_signal_mutex);
	uint8_t *ptr;
	if((this->in_data.size() == 0) && this->readerr)
		return NULL;
	fprintf(stderr,"get_in() in_data.size %d\n",this->in_data.size());
	while(this->in_data.size() == 0)
	{
		this->in_cond_var.wait(lock);
		if(this->readerr)
			return NULL;
	}
	fprintf(stderr,"get_in>>\n");
	this->in_mutex.lock();
	ptr = this->in_data.front();
	this->in_data.pop();
	this->in_mutex.unlock();
    usleep(100);
	return ptr;
}

void DVBT_memory::free_in(uint8_t *ptr)
{
	fprintf(stderr,"free_in() in_free.size %d\n",this->in_free.size());
	this->in_mutex.lock();
	this->in_free.push(ptr);
	this->in_mutex.unlock();
}

//might block
uint8_t* DVBT_memory::get_out()
{
	fprintf(stderr,"get_out() out_free.size %d\n",this->out_free.size());
	std::unique_lock<std::mutex> lock(this->out_signal_mutex);
	uint8_t *ptr;
	if(this->writeerr)
		return NULL;
	while(this->out_free.size() == 0)
	{
		this->out_cond_var.wait(lock);
		if(this->writeerr)
			return NULL;
	}
	
	fprintf(stderr,"get_out>>\n");
	this->out_mutex.lock();
	ptr = this->out_free.front();
	this->out_free.pop();
	this->out_mutex.unlock();
	usleep(100);
	return ptr;
}

void DVBT_memory::free_out(uint8_t *ptr)
{
	fprintf(stderr,"free_out out_data.size %d\n",this->out_data.size());
	this->out_mutex.lock();
	this->out_data.push(ptr);
	this->out_mutex.unlock();
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
	std::unique_lock<std::mutex> lock(this->in_signal_mutex);
	uint8_t *ptr;
	do{
		while(this->in_free.size() == 0)
		{
			in_cond_var.wait_for(lock,chrono::milliseconds(1));
		}
		
		this->in_mutex.lock();
		ptr = this->in_free.front();
		this->in_free.pop();
		this->in_mutex.unlock();
		this->readerr = this->read(ptr);
		
		this->in_mutex.lock();
		this->in_data.push(ptr);
		this->in_mutex.unlock();
		
		this->in_cond_var.notify_one();
		usleep(100);
		fprintf(stderr,"read_thread notify\n");
	}
	while(!this->readerr && !this->writeerr);
	this->in_cond_var.notify_one();
	fprintf(stderr,"read_thread <<\n");
}

void DVBT_memory::write_thread()
{
	std::unique_lock<std::mutex> lock(this->out_signal_mutex);
	uint8_t *ptr;
	do{
		if((this->out_data.size() == 0) && this->readerr)
		{
			this->writeerr = true;
			break;
		}
		while(this->out_data.size() == 0)
		{
			out_cond_var.wait_for(lock,chrono::milliseconds(1));
		}
		
		this->out_mutex.lock();
		ptr = this->out_data.front();
		this->out_data.pop();
		this->out_mutex.unlock();
		
		this->writeerr = this->write(ptr);
		
		this->out_mutex.lock();
		this->out_free.push(ptr);
		this->out_mutex.unlock();
		
		this->out_cond_var.notify_one();
		usleep(1000);
		fprintf(stderr,"write_thread notify\n");
	}
	while(!this->writeerr);
	this->out_cond_var.notify_one();
	fprintf(stderr,"write_thread <<\n");
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
	readerr = false;
	writeerr = false;
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
	for(int i=0;i<DVBTENC_BUFFERS;i++)
	{
		delete[] this->in_free.front();
		delete[] this->out_free.front();
		this->in_free.pop();
		this->out_free.pop();
	}
	this->rt.join();
	this->wt.join();
}
