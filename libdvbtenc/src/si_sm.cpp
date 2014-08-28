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

#include <math.h>
#include "si_sm.hpp"

using namespace std;

DVBT_si_sm::DVBT_si_sm(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->dvbt_settings = dvbt_settings;
	this->pin = pin;
	this->pout = pout;
	
	this->mReadSize = this->dvbt_settings->ofdmuseablecarriers;
	this->mWriteSize = this->dvbt_settings->ofdmuseablecarriers * sizeof(dvbt_complex_t);
	
	this->pin->initReadEnd( this->mReadSize );

	//todo: ofdmuseablecarriers ?
	this->Hq = new int[this->dvbt_settings->ofdmmode];
	memset(this->Hq, 0, this->dvbt_settings->ofdmmode * sizeof(int));
    
	if(this->dvbt_settings->ofdmmode == 2048)
	{
		int rs,rs_xor,r;
		int *hq_ptr = this->Hq;
		for(int i=0; i < 2048; i++)
		{
			if( i < 2 )
			    rs = 0;
			else if( i == 2 )
			    rs = 1;
			else
			{
				rs_xor = (rs & 1) ^ ((rs>>3) & 1);
				rs >>= 1;
				rs |= rs_xor<<9;
			}
			
			r = (rs & 0x00000001)<<4;
			r |= (rs & 0x00000002)<<2;
			r |= (rs & 0x00000004)<<7;
			r |= (rs & 0x00000008)<<3;
			r |= (rs & 0x00000010)>>2;
			r |= (rs & 0x00000020)<<3;
			r |= (rs & 0x00000040)>>5;
			r |= (rs & 0x00000080)>>2;
			r |= (rs & 0x00000100)>>1;
			r |= (rs & 0x00000200)>>9;
			r |= (i & 0x00000001)<<10;
			
			if( r < this->dvbt_settings->ofdmuseablecarriers )
			{
				*hq_ptr = r;
				 hq_ptr++;
			}
		}
	}
	else
	{
		int rs,rs_xor,r;
		int *hq_ptr = this->Hq;
		for(int i=0; i< 8192; i++)
		{
			if( i < 2 )
			    rs = 0;
			else if( i == 2 )
			    rs = 1;
			else
			{
				rs_xor = (rs & 1) ^ ((rs>>1) & 1) ^ ((rs>>4) & 1) ^ ((rs>>6) & 1);
				rs >>= 1;
				rs |= rs_xor<<11;
			}
			
			r = (rs & 0x00000001)<<7;
			r |= (rs & 0x00000002)<<0;
			r |= (rs & 0x00000004)<<2;
			r |= (rs & 0x00000008)>>1;
			r |= (rs & 0x00000010)<<5;
			r |= (rs & 0x00000020)<<1;
			r |= (rs & 0x00000040)<<2;
			r |= (rs & 0x00000080)<<3;
			r |= (rs & 0x00000100)>>8;
			r |= (rs & 0x00000200)>>6;
			r |= (rs & 0x00000400)<<1;
			r |= (rs & 0x00000800)>>6;
			r |= (i & 0x00000001)<<12;
			
			if( r < this->dvbt_settings->ofdmuseablecarriers )
			{
				*hq_ptr = r;
				hq_ptr++;
			}
		}
	}
	
	this->lookup = new dvbt_complex_t[(1<<this->dvbt_settings->modulation)];
    for(int i=0;i<(1<<this->dvbt_settings->modulation);i++)
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
	
	this->symbol = 0;
}


DVBT_si_sm::~DVBT_si_sm()
{
	delete[] this->Hq;
	delete[] this->lookup;
}

bool DVBT_si_sm::encode()
{
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	if(this->symbol & 1)
	{
		for(unsigned int i=0;i<this->dvbt_settings->ofdmuseablecarriers;i++)
		{
			((dvbt_complex_t*)out->ptr)[i] = this->lookup[in->ptr[this->Hq[i]]];
		}
	}
	else
	{
		for(unsigned int i=0;i<this->dvbt_settings->ofdmuseablecarriers;i++)
		{
			((dvbt_complex_t*)out->ptr)[this->Hq[i]] = this->lookup[in->ptr[i]];
		}
	}

	this->symbol ^=1;

	delete in;

	if(!this->pout->write(out))
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	return true;
}
