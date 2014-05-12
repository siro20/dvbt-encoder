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

#include "si.hpp"

using namespace std;

DVBT_si::DVBT_si(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings)
{
	int i,rs,rs_xor,r;
	int *hq_ptr;
	this->fd_in = fd_in;
	this->fd_out = fd_out;
	this->dvbt_settings = dvbt_settings;

	this->in_multiple_of = this->dvbt_settings->ofdmuseablecarriers;
	this->out_multiple_of = this->dvbt_settings->ofdmuseablecarriers;

	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
		
	this->mem = new DVBT_memory(fd_in,fd_out,this->in_multiple_of,this->out_multiple_of, true);

	//todo: ofdmuseablecarriers ?
	hq_ptr = this->Hq = new int[this->dvbt_settings->ofdmmode];
	memset(this->Hq, 0, this->dvbt_settings->ofdmmode * sizeof(int));
    
	if(this->dvbt_settings->ofdmmode == 2048)
	{
		for(i=0; i < 2048; i++)
		{
			if( i < 2)
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
		for(i=0; i< 8192; i++)
		{
			if( i < 2)
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
}


DVBT_si::~DVBT_si()
{
	delete[] this->Hq;
}

bool DVBT_si::encode(int symbol)
{
	int i;
	int rret, wret;
	uint8_t *out;
	uint8_t *in;
	
	in = this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;

	if(symbol & 1)
	{
		for(i=0;i<this->dvbt_settings->ofdmuseablecarriers;i++)
		{
			out[i] = in[this->Hq[i]];
		}
	}
	else
	{
		for(i=0;i<this->dvbt_settings->ofdmuseablecarriers;i++)
		{
			out[this->Hq[i]] = in[i];
		}
	}
	
	this->mem->free_out((uint8_t*)out);
	this->mem->free_in(in);

	return true;
}
