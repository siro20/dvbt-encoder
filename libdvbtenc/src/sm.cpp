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

#include "sm.hpp"

using namespace std;

DVBT_sm::DVBT_sm(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->dvbt_settings = dvbt_settings;

	this->mReadSize = this->dvbt_settings->ofdmuseablecarriers;
	this->mWriteSize = this->dvbt_settings->ofdmuseablecarriers * sizeof(dvbt_complex_t);

	this->pin = pin;
	this->pout = pout;
	
	this->pin->initReadEnd( this->mReadSize );

	this->lookup = new dvbt_complex_t[(1<<this->dvbt_settings->modulation)];
	for(unsigned int i=0; i < (1<<this->dvbt_settings->modulation);i++)
	{
		if(this->dvbt_settings->modulation == 2)
		{
			//MSB I0 -> 0x02, I1 -> 0x01
			this->lookup[i].x = (i&2 ? -1.0f : 1.0f) / sqrtf( 2.0f ); // y0
			this->lookup[i].y = (i&1 ? -1.0f : 1.0f) / sqrtf( 2.0f ); // y1
		}
		else if(this->dvbt_settings->modulation == 4)
		{
			if(i&1)
				this->lookup[i].y = 1.0f / sqrtf( 10.0f );
			else
				this->lookup[i].y = 3.0f / sqrtf( 10.0f );
			if(i&2)
				this->lookup[i].x = 1.0f / sqrtf( 10.0f );
			else
				this->lookup[i].x = 3.0f / sqrtf( 10.0f );
			if(i&8)
				this->lookup[i].x *= -1.0f;
			if(i&4)
				this->lookup[i].y *= -1.0f;
		}
		else
		{
			if(i&0x08)
				this->lookup[i].x = 2.0f;
			else
				this->lookup[i].x = 6.0f;
			if(i&0x04)
				this->lookup[i].y = 2.0f;
			else
				this->lookup[i].y = 6.0f;
				
			if((i^(i>>2))&0x02)
				this->lookup[i].x -= 1.0f;
			else
				this->lookup[i].x += 1.0f;
			if((i^(i>>2))&0x01)
				this->lookup[i].y -= 1.0f;
			else
				this->lookup[i].y += 1.0f;
				
			if(i&0x20)
				this->lookup[i].x *= -1.0f;
			if(i&0x10)
				this->lookup[i].y *= -1.0f;
			this->lookup[i].x /= sqrtf( 42.0f );
			this->lookup[i].y /= sqrtf( 42.0f );
		}
		
	}
}

DVBT_sm::~DVBT_sm()
{
	delete[] this->lookup;
}

bool DVBT_sm::encode()
{
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	for(unsigned int i=0;i<in->size;i++)
	{
		((dvbt_complex_t*)(out->ptr))[i] = this->lookup[in->ptr[i]];
	}

	delete in;

	if(!this->pout->write(out))
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	return true;
}
