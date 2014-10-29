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
#include <fstream>
#include <chrono>

using namespace std;

DVBT_memory::DVBT_memory(int Size)
{
	size = Size;
	ptr = 0;
	if(Size)
		ptr = new uint8_t[Size];
}

DVBT_memory::~DVBT_memory()
{
	if(size)
		delete[] ptr;
}

void DVBT_pipe::initReadEnd( unsigned int bufferSize )
{
	std::unique_lock<std::mutex> lock(this->mMutex);

	this->mReadEndSize = bufferSize;
	this->cWaitInit.notify_one();
}

bool DVBT_pipe::write(DVBT_memory *memin)
{
	
	if(this->mReadEndSize == 0)
	{
		std::unique_lock<std::mutex> lock(this->mMutex);
		while(this->mReadEndSize == 0)
		{
			this->cWaitInit.wait(lock);
		}
	}
	if(this->mReadEndClose)
		return false;
	
	// special case
	if( memin->size == this->mReadEndSize )
	{
		std::unique_lock<std::mutex> lock(this->mMutex);
		while(this->mQueueOut.size() == this->mQueueMaxSize)
		{
			this->cWaitFull.wait(lock);
		}
		this->mQueueOut.push( memin );
		if(this->mQueueOut.size() == 1)
			this->cWaitEmpty.notify_one();
		return true;
	}
	
	unsigned int offset_in = 0;
	while( offset_in < memin->size )
	{
		if(!this->memout)
			this->memout = new DVBT_memory( this->mReadEndSize );

		int copybytes = min(memin->size - offset_in, this->mReadEndSize - this->mOffsetOut);

		memcpy(this->memout->ptr + this->mOffsetOut, memin->ptr + offset_in, copybytes);
		
		offset_in += copybytes;
		this->mOffsetOut += copybytes;
		
		if( this->mOffsetOut == this->mReadEndSize )
		{
			std::unique_lock<std::mutex> lock(this->mMutex);
			while((this->mQueueOut.size() == this->mQueueMaxSize) && !this->mReadEndClose)
			{
				this->cWaitFull.wait(lock);
			}
			if(this->mReadEndClose){
				delete memin;
				return false;
			}
			this->mQueueOut.push( memout );

			if(this->mQueueOut.size() == 1){
				this->cWaitEmpty.notify_one();
			}
			this->mOffsetOut = 0;
			this->memout = 0;
		}
	}
	delete memin;
	return true;
}

DVBT_memory *DVBT_pipe::read()
{
	if(this->mWriteEndClose && this->mQueueOut.empty())
		return NULL;

	std::unique_lock<std::mutex> lock(this->mMutex);

	while(this->mQueueOut.empty() && !this->mWriteEndClose)
	{
		this->cWaitEmpty.wait(lock);
	}
	if(this->mWriteEndClose && this->mQueueOut.empty())
		return NULL;
	
	DVBT_memory *mem = this->mQueueOut.front();
	this->mQueueOut.pop();
	if(this->mQueueOut.size() == (this->mQueueMaxSize - 1))
		this->cWaitFull.notify_one();

	return mem;
}

size_t DVBT_pipe::read_size()
{
	if(this->mWriteEndClose && this->mQueueOut.empty())
		return 0;
	std::unique_lock<std::mutex> lock(this->mMutex);

	return this->mQueueOut.size();
}

void DVBT_pipe::CloseReadEnd()
{
	std::unique_lock<std::mutex> lock(this->mMutex);

	this->mReadEndClose = true;
	this->cWaitFull.notify_one(); 
}

void DVBT_pipe::CloseWriteEnd()
{
	std::unique_lock<std::mutex> lock(this->mMutex);

	if(this->memout)
	{
		memset(this->memout->ptr+this->mOffsetOut, 0, this->mReadEndSize - this->mOffsetOut);
		
		while((this->mQueueOut.size() == this->mQueueMaxSize) && !this->mReadEndClose)
		{
			this->cWaitFull.wait(lock);
		}
		if(this->mReadEndClose){
			return;
		}
		this->mQueueOut.push( memout );
		if(this->mQueueOut.size() == 1){
			this->cWaitEmpty.notify_one();
		}
		this->mOffsetOut = 0;
		this->memout = 0;
	}
	this->mWriteEndClose = true;
	this->cWaitEmpty.notify_one(); 
}

DVBT_memory *DVBT_pipe::allocMemRead()
{
	return new DVBT_memory(this->mReadEndSize);
}

DVBT_pipe::DVBT_pipe(const char *strIdent)
{
	this->mReadEndSize = 0;
	this->mOffsetOut = 0;
	this->memout = 0;
	this->mWriteEndClose = false;
	this->mReadEndClose = false;
	this->mIdent = strIdent;
	this->mQueueMaxSize = DVBTENC_BUFFERS;

}

DVBT_pipe::DVBT_pipe(const char *strIdent, unsigned int queueMaxSize)
{
	this->mReadEndSize = 0;
	this->mOffsetOut = 0;
	this->memout = 0;
	this->mWriteEndClose = false;
	this->mReadEndClose = false;
	this->mIdent = strIdent;
	this->mQueueMaxSize = queueMaxSize;
}

DVBT_pipe::~DVBT_pipe()
{
	if(this->memout)
		delete this->memout; 
}
