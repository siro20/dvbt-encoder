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

}

DVBT_ii::~DVBT_ii()
{
}

//simple modulo 126 for numbers smaller than 2*126
#define MOD126(x,y) if((x) >= 126) y=(x)-126; else y=x;

bool DVBT_ii::encode()
{
	int i,n;
	uint8_t *out;
	uint8_t *in;
	static const uint8_t shiftreg_indx[6] = {0,63,105,42,21,84};
	
	in = this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;
		
	for(n=0;n<this->mem->in_size;n+=this->dvbt_settings->DVBT_II_DEPTH*this->dvbt_settings->modulation)
	{
		memset(out,0,this->dvbt_settings->DVBT_II_DEPTH);
		if(this->dvbt_settings->modulation==2)
		{
			for(i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
			{
				int off;
				MOD126(i+shiftreg_indx[0], off)
				if(in[off*2+0])
					out[i] |= 0x02;
				MOD126(i+shiftreg_indx[1], off)
				if(in[off*2+1])
					out[i] |= 0x01;
			}
		}
		else if(this->dvbt_settings->modulation==4)
		{
			for(i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
			{
				int off;
				MOD126(i+shiftreg_indx[0], off)
				if(in[off*4+0])
					out[i] |= 0x08;
				MOD126(i+shiftreg_indx[1], off)
				if(in[off*4+2])
					out[i] |= 0x04;
				MOD126(i+shiftreg_indx[2], off)
				if(in[off*4+1])
					out[i] |= 0x02;
				MOD126(i+shiftreg_indx[3], off)
				if(in[off*4+3])
					out[i] |= 0x01;
			}
		}
		else
		{
			for(i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
			{
				int off;
				MOD126(i+shiftreg_indx[0], off)
				if(in[off*6+0])
					out[i] |= 0x20;
				MOD126(i+shiftreg_indx[1], off)
				if(in[off*6+2])
					out[i] |= 0x10;
				MOD126(i+shiftreg_indx[2], off)
				if(in[off*6+4])
					out[i] |= 0x08;
				MOD126(i+shiftreg_indx[3], off)
				if(in[off*6+1])
					out[i] |= 0x04;
				MOD126(i+shiftreg_indx[4], off)
				if(in[off*6+3])
					out[i] |= 0x02;
				MOD126(i+shiftreg_indx[5], off)
				if(in[off*6+5])
					out[i] |= 0x01;
					
			}
		}
		out += this->dvbt_settings->DVBT_II_DEPTH;
		in += this->dvbt_settings->DVBT_II_DEPTH*this->dvbt_settings->modulation;
	};
	
	this->mem->free_out(out);
	this->mem->free_in(in);

	return true;
}
