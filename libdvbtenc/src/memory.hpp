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

#define DVBTENC_BUFFERS 10
#define DVBTENC_APROX_BUF_SIZE 1024 * 4

class DVBT_memory
{
public:
	DVBT_memory(int size);
	~DVBT_memory();
	unsigned int size;
	uint8_t *ptr;
};

/*
class DVBT_threadsafe_queue
{
public:
	DVBT_threadsafe_queue(int maximumSize);
	void push(DVBT_memory *data);
	bool empty();
	DVBT_memory *front();
	void pop();
private:
	std::queue<DVBT_memory *> mQueue;
	mutex mMutex;
	condition_variable cWaitEmpty;
	condition_variable cWaitFull;
	int mMaximumSize;
};
*/
class DVBT_pipe
{
public:
	~DVBT_pipe();
	DVBT_pipe(unsigned int queueMaxSize);
	DVBT_pipe();
	void initReadEnd( unsigned int bufferSize );
	bool write(DVBT_memory *ptr);
	DVBT_memory *read();
	DVBT_memory *allocMemRead();
	void CloseReadEnd();
	void CloseWriteEnd();
private:
	unsigned int mQueueMaxSize;
	unsigned int mReadEndSize;
	unsigned int mOffsetOut;
	bool mWriteEndClose;
	bool mReadEndClose;
	mutex mMutex;
	condition_variable cWaitEmpty;
	condition_variable cWaitFull;
	condition_variable cWaitInit;
    DVBT_memory *memout;
	std::queue<DVBT_memory *> mQueueOut;
};

#endif
