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

DVBT_ce::DVBT_ce(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings)
{
	this->fd_in = fd_in;
	this->fd_out = fd_out;
	this->dvbt_settings = dvbt_settings;
	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
	// encode one ofdm symbol at time
	this->in_multiple_of = this->dvbt_settings->bytespersuperframe / (this->dvbt_settings->DVBT_SYMBOLS_FRAME*this->dvbt_settings->DVBT_FRAMES_SUPERFRAME);
	this->out_multiple_of = this->dvbt_settings->ofdmuseablecarriers * this->dvbt_settings->modulation;
	this->shiftreg = 0x00;

	this->mem = new DVBT_memory(this->in_multiple_of,this->out_multiple_of, true);
}

DVBT_ce::~DVBT_ce()
{
}

inline uint8_t _conv_x( uint8_t x )
{
	uint8_t i=0;
	i += x & 0x40 ? 1 : 0;
	i += x & 0x20 ? 1 : 0;
	i += x & 0x10 ? 1 : 0;
	i += x & 0x08 ? 1 : 0;
	i += x & 0x01 ? 1 : 0;
	return i & 1;
}

inline uint8_t _conv_y( uint8_t x )
{
	uint8_t i=0;
	i += x & 0x40 ? 1 : 0;
	i += x & 0x10 ? 1 : 0;
	i += x & 0x08 ? 1 : 0;
	i += x & 0x02 ? 1 : 0;
	i += x & 0x01 ? 1 : 0;
	return i & 1;
}

int DVBT_ce::conv_encoder_12(uint8_t *in, uint8_t *out)
{
	int i,j;

	for(i=0;i<this->mem->in_size;i++)
	{
		for(j=0x80;j!=0;j>>=1)
		{
			this->shiftreg >>= 1;
			if(in[i] & j)
				this->shiftreg |= 0x40;
			out[0] = _conv_x( this->shiftreg );
			out[1] = _conv_y( this->shiftreg );
			out += 2;
		}
	}
	return this->mem->in_size*16;
}

int DVBT_ce::conv_encoder_23()
{
	int i;
	int cnt_ret;
	uint8_t *outptr = this->mem->out;
	uint8_t *tmp = new uint8_t[this->mem->in_size*16];

	conv_encoder_12( this->mem->in, tmp);
	cnt_ret=0;
	for(i=0;i<this->mem->in_size*16;i++)
	{
		if( (i % 4) != 2 ) // remove X2 from stream
		{
			*outptr = tmp[i];
			outptr++;
			cnt_ret++;
		}
	}
	delete[] tmp;
	return cnt_ret;
}

int DVBT_ce::conv_encoder_34()
{
	int i;
	int cnt_ret;
	uint8_t *outptr = this->mem->out;
	uint8_t *tmp = new uint8_t[this->mem->in_size*16];

	conv_encoder_12( this->mem->in, tmp);
	cnt_ret = 0;
	for(i=0;i<this->mem->in_size*16;i++)
	{
		if( (i % 6) != 2 && (i % 6) != 5) // remove X2 and Y3 from stream
		{
			*outptr = tmp[i];
			outptr++;
			cnt_ret++;
		}
	}
	delete[] tmp;
	return cnt_ret;
}

int DVBT_ce::conv_encoder_56()
{
	int i;
	int cnt_ret;
	uint8_t *outptr = this->mem->out;
	uint8_t *tmp = new uint8_t[this->mem->in_size*16];

	conv_encoder_12( this->mem->in, tmp);
	cnt_ret = 0;
	for(i=0;i<this->mem->in_size*16;i++)
	{
		if( (i % 10) != 2 && (i % 10) != 5 && (i % 10) != 6 && (i % 10) != 9 ) // remove X2 and Y3 and X4 and Y5 from stream
		{
			*outptr = tmp[i];
			outptr++;
			cnt_ret++;
		}
	}
	delete[] tmp;
	return cnt_ret;
}

int DVBT_ce::conv_encoder_78()
{
	int i;
	int cnt_ret;
	uint8_t *outptr = this->mem->out;
	uint8_t *tmp = new uint8_t[this->mem->in_size*16];

	conv_encoder_12( this->mem->in, tmp );
	cnt_ret = 0;
	for(i=0;i<this->mem->in_size*16;i++)
	{
		if( (i % 14) != 2 && (i % 14) != 4 && (i % 14) != 6 && (i % 14) != 9 && 
			(i % 14) != 10 && (i % 14) != 13) // remove X2 and X3 and X4 and Y5 and X6 and Y7 from stream
		{
			*outptr = tmp[i];
			outptr++;
			cnt_ret++;
		}
	}
	delete[] tmp;
	return cnt_ret;
}

int DVBT_ce::encode()
{
	int rret, wret;

	rret = this->mem->read(this->fd_in);
	
	switch(this->dvbt_settings->coderate)
	{
		case 2:
			conv_encoder_12 (this->mem->in, this->mem->out);
			break;
		case 3:
			conv_encoder_23 ();
			break;
		case 4:
			conv_encoder_34 ();
			break;
		case 6:
			conv_encoder_56 ();
			break;
		case 8:
			conv_encoder_78 ();
			break;
		default:
			conv_encoder_12 (this->mem->in, this->mem->out);
			break;
	}
	wret = this->mem->write(this->fd_out);

	if(rret || wret)
		return 1;
	return 0;
}
