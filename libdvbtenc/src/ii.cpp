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

#include "ii.hpp"

using namespace std;

DVBT_ii::DVBT_ii(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings)
{
	int idx;
	this->fd_in = fd_in;
	this->fd_out = fd_out;
	this->dvbt_settings = dvbt_settings;

	this->in_multiple_of = this->dvbt_settings->modulation * this->dvbt_settings->DVBT_II_DEPTH;
	this->out_multiple_of = this->dvbt_settings->DVBT_II_DEPTH;

	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
		
	this->mem = new DVBT_memory(fd_in,fd_out,this->in_multiple_of,this->out_multiple_of,false);
	this->lookup = new int[this->dvbt_settings->modulation * this->dvbt_settings->DVBT_II_DEPTH];
	
	static const uint8_t shiftreg_indx[6] = {0,63,105,42,21,84};
	
	if(this->dvbt_settings->modulation==2)
	{
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			lookup[idx++] = ((i+shiftreg_indx[0])%126)*2+0;
			lookup[idx++] = ((i+shiftreg_indx[1])%126)*2+1;
		}
	}
	else if(this->dvbt_settings->modulation==4)
	{
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			lookup[idx++] = ((i+shiftreg_indx[0])%126)*4+0;
			lookup[idx++] = ((i+shiftreg_indx[1])%126)*4+2;
			lookup[idx++] = ((i+shiftreg_indx[2])%126)*4+1;
			lookup[idx++] = ((i+shiftreg_indx[3])%126)*4+3;
		}
	}
	else
	{
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			lookup[idx++] = ((i+shiftreg_indx[0])%126)*6+0;
			lookup[idx++] = ((i+shiftreg_indx[1])%126)*6+3;
			lookup[idx++] = ((i+shiftreg_indx[2])%126)*6+1;
			lookup[idx++] = ((i+shiftreg_indx[3])%126)*6+4;
			lookup[idx++] = ((i+shiftreg_indx[4])%126)*6+2;
			lookup[idx++] = ((i+shiftreg_indx[5])%126)*6+5;
		}
	}
}

DVBT_ii::~DVBT_ii()
{
	delete[] this->lookup;
}

//simple modulo 126 for numbers smaller than 2*126
#define MOD126(x,y) if((x) >= 126) y=(x)-126; else y=x;

bool DVBT_ii::encode()
{
	int i,n;
	uint8_t *out;
	uint8_t *in;
	uint8_t *outptr;
	uint8_t *inptr;
	
	in = this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;
	outptr = out;
	inptr = in;
	for(n=0;n<this->mem->in_size;n+=this->dvbt_settings->DVBT_II_DEPTH*this->dvbt_settings->modulation)
	{
		memset(outptr,0,this->dvbt_settings->DVBT_II_DEPTH);
		if(this->dvbt_settings->modulation==2)
		{
			int idx=0;
			for(i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
			{
				outptr[i] |= inptr[this->lookup[idx]] ? 0x02 : 0;
				outptr[i] |= inptr[this->lookup[idx]] ? 0x01 : 0;
			}
		}
		else if(this->dvbt_settings->modulation==4)
		{
			int idx=0;
			for(i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
			{
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x08 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x04 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x02 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x01 : 0;
			}
		}
		else
		{
			int idx=0;
			for(i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
			{
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x20 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x10 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x08 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x04 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x02 : 0;
				outptr[i] |= inptr[this->lookup[idx++]] ? 0x01 : 0;
			}
		}
		outptr += this->dvbt_settings->DVBT_II_DEPTH;
		inptr += this->dvbt_settings->DVBT_II_DEPTH*this->dvbt_settings->modulation;
	};
	
	this->mem->free_out(out);
	this->mem->free_in(in);

	return true;
}
