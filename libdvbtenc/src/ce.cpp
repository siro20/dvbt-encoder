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

#include "ce.hpp"

using namespace std;

DVBT_ce::DVBT_ce(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->pin = pin;
	this->pout = pout;
	this->dvbt_settings = dvbt_settings;
	
	this->mWriteSize = this->dvbt_settings->ofdmuseablecarriers * this->dvbt_settings->modulation;
	this->mReadSize = ((this->dvbt_settings->coderate -1) * this->dvbt_settings->ofdmuseablecarriers * this->dvbt_settings->modulation);
	//encode multiple symbols for coderate > 2/3
	while(this->mReadSize % (this->dvbt_settings->coderate * 8))
	{
		this->mReadSize *= 2;
		this->mWriteSize *= 2;
	}
	this->mReadSize /= (this->dvbt_settings->coderate * 8);
	
	pin->initReadEnd( this->mReadSize );
	
	this->shiftreg = 0x00;
}

DVBT_ce::~DVBT_ce()
{
}

inline uint8_t _conv_x( uint8_t x )
{
	return ((x>>6) ^ (x>>5) ^ (x>>4) ^ (x>>3) ^ x)&1;
}

inline uint8_t _conv_y( uint8_t x )
{
	return ((x>>6) ^ (x>>4) ^ (x>>3) ^ (x>>1) ^ x)&1;
}

void DVBT_ce::conv_encoder_12(DVBT_memory *in, DVBT_memory *out)
{
	int i,j;
	uint8_t *outptr = out->ptr;

	for(i=0;i<in->size;i++)
	{
		for(j=0x80;j!=0;j>>=1)
		{
			this->shiftreg >>= 1;
			if(in->ptr[i] & j){
				this->shiftreg |= 0x40;
			}

			outptr[0] = _conv_x( this->shiftreg );
			outptr[1] = _conv_y( this->shiftreg );
			outptr += 2;
		}
	}
}

void DVBT_ce::conv_encoder_23(DVBT_memory *in, DVBT_memory *out)
{
	int i,j,z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(i=0;i<in->size;i++)
	{
		for(j=0x80;j!=0;j>>=1)
		{
			this->shiftreg >>= 1;
			if(in->ptr[i] & j)
				this->shiftreg |= 0x40;
			if( z != 2 ) // remove X2 from stream
			{
				*outptr = _conv_x( this->shiftreg );
				outptr++;
			}
			z++;
			if( z != 2 ) // remove X2 from stream
			{
				*outptr = _conv_y( this->shiftreg );
				outptr++;
			}
			z++;
			z &= 0x3; // z modulo 4
		}
	}
}

void DVBT_ce::conv_encoder_34(DVBT_memory *in, DVBT_memory *out)
{
	int i,j,z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(i=0;i<in->size;i++)
	{
		for(j=0x80;j!=0;j>>=1)
		{
			this->shiftreg >>= 1;
			if(in->ptr[i] & j)
				this->shiftreg |= 0x40;
			if( z != 2 && z != 5) // remove X2, Y3 from stream
			{
				*outptr = _conv_x( this->shiftreg );
				outptr++;
			}
			z++;
			if( z != 2 && z != 5) // remove X2, Y3 from stream
			{
				*outptr = _conv_y( this->shiftreg );
				outptr++;
			}
			z++;
			z = z % 6;
		}
	}
}


void DVBT_ce::conv_encoder_56(DVBT_memory *in, DVBT_memory *out)
{
	int i,j,z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(i=0;i<in->size;i++)
	{
		for(j=0x80;j!=0;j>>=1)
		{
			this->shiftreg >>= 1;
			if(in->ptr[i] & j)
				this->shiftreg |= 0x40;
			if( z != 2 && z != 5 && z != 6 && z != 9 ) // remove X2 and Y3 and X4 and Y5 from stream
			{
				*outptr = _conv_x( this->shiftreg );
				outptr++;
			}
			z++;
			if( z != 2 && z != 5 && z != 6 && z != 9 ) // remove X2 and Y3 and X4 and Y5 from stream
			{
				*outptr = _conv_y( this->shiftreg );
				outptr++;
			}
			z++;
			z = z % 10;
		}
	}
}


void DVBT_ce::conv_encoder_78(DVBT_memory *in, DVBT_memory *out)
{
	int i,j,z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(i=0;i<in->size;i++)
	{
		for(j=0x80;j!=0;j>>=1)
		{
			this->shiftreg >>= 1;
			if(in->ptr[i] & j)
				this->shiftreg |= 0x40;
			if( z != 2 && z != 5 && z != 6 && z != 9 && z != 10 && z != 13 ) // remove X2 and X3 and X4 and Y5 and X6 and Y7 from stream
			{
				*outptr = _conv_x( this->shiftreg );
				outptr++;
			}
			z++;
			if( z != 2 && z != 5 && z != 6 && z != 9 && z != 10 && z != 13 ) // remove X2 and X3 and X4 and Y5 and X6 and Y7 from stream
			{
				*outptr = _conv_y( this->shiftreg );
				outptr++;
			}
			z++;
			z = z % 14;
		}
	}
}

bool DVBT_ce::encode()
{
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	switch(this->dvbt_settings->coderate)
	{
		case 2:
			conv_encoder_12 (in, out);
			break;
		case 3:
			conv_encoder_23 (in, out);
			break;
		case 4:
			conv_encoder_34 (in, out);
			break;
		case 6:
			conv_encoder_56 (in, out);
			break;
		case 8:
			conv_encoder_78 (in, out);
			break;
		default:
			conv_encoder_12 (in, out);
			break;
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
