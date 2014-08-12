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

#define DVBTENC_BUFFERS 6
#define DVBTENC_APROX_BUF_SIZE 1024 * 4

class DVBT_threadsafe_queue
{
private:
	std::queue<uint8_t *> mQueue;
	mutex mMutex;
	condition_variable cWaitEmpty;
	condition_variable cWaitFull;

public:
	void push(uint8_t *data)
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while(mQueue.size() == DVBTENC_BUFFERS)
		{
			cWaitFull.wait(lock);
		}
		mQueue.push(data);
		if(mQueue.size() == 1)
			cWaitEmpty.notify_one();
	}

	bool empty()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		return mQueue.empty();
	}

	uint8_t *front()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while(mQueue.empty())
		{
			cWaitEmpty.wait(lock);
		}
		return mQueue.front();
	}

	void pop()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		mQueue.pop();
		if(mQueue.size() == DVBTENC_BUFFERS - 1)
			cWaitFull.notify_one();
	}
};

class DVBT_memory
{
public:
	~DVBT_memory();
	DVBT_memory( FILE *fd_i, FILE *fd_o, int in_multiple_of, int out_multiple_of, bool fixed );
	int in_size;
	int out_size;
	int in_multiple_of;
	int out_multiple_of;
	uint8_t * get_in();
	uint8_t* get_out();
	void free_in(uint8_t *ptr);
	void free_out(uint8_t* ptr);

private:
	bool write(uint8_t *ptr);
	bool read(uint8_t *ptr);
	void read_thread();
	void write_thread();
	thread rt;
	thread wt;
	FILE *fd_in;
	FILE *fd_out;
	DVBT_threadsafe_queue inQueue;
	DVBT_threadsafe_queue outQueue;
};

#endif
