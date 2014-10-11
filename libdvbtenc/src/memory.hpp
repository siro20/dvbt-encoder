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

using namespace std;

#define DVBTENC_BUFFERS 32

class DVBT_memory
{
public:
	DVBT_memory(int size);
	~DVBT_memory();
	unsigned int size;
	uint8_t *ptr;
};

class DVBT_pipe
{
public:
	~DVBT_pipe();
	//identifyer for debugging, max read end queue size 
	DVBT_pipe(const char *strIdent, unsigned int queueMaxSize);
	//identifyer for debugging, max read end queue size is set to DVBTENC_BUFFERS
	DVBT_pipe(const char *strIdent);
	//init read end for dynamic memory conversions
	void initReadEnd( unsigned int bufferSize );
	//write a buffer to the queue, might fail if the read end has been closed, might block
	bool write(DVBT_memory *ptr);
	//fetch one item from the queue, might block, might return NULL or an empty buffer
	DVBT_memory *read();
	//alloc a buffer with size mReadEndSize
	DVBT_memory *allocMemRead();
	//close read end
	void CloseReadEnd();
	//close write end
	void CloseWriteEnd();
	//get size of read end non-blocking
	size_t read_size();
private:
	unsigned int mQueueMaxSize;
	unsigned int mReadEndSize;
	unsigned int mOffsetOut;
	bool mWriteEndClose;
	bool mReadEndClose;
	const char *mIdent;
	mutex mMutex;
	condition_variable cWaitEmpty;
	condition_variable cWaitFull;
	condition_variable cWaitInit;
    DVBT_memory *memout;
	std::queue<DVBT_memory *> mQueueOut;
};

#endif
