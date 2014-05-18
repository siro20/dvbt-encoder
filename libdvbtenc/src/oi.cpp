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

#include "oi.hpp"

DVBT_oi::DVBT_oi(FILE *fd_in, FILE *fd_out)
{
	this->in_multiple_of = OI_SIZE;
	this->out_multiple_of = OI_SIZE;
	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
	this->mem = new DVBT_memory(fd_in,fd_out,this->in_multiple_of,this->out_multiple_of,false);

	this->fd_in = fd_in;
	this->fd_out = fd_out;
	
	for(int i=0;i<OI_SIZE;i++)
	{
		for(int j=0;j<i*OI_DEPTH;j++)
		{
			this->oi_queues[i].push(0);
		}
	}
}

DVBT_oi::~DVBT_oi()
{
}

bool DVBT_oi::encode()
/* interleave data byteswise, oi_buf has size 17*(1+2+3+4+5+6+7+8+9+10+11) + 12, reset on startup, n has to be multiple of 12 */
/* data written to data_out is same as data_cnt */
{
	int i,k;
	unsigned char *p_oi_buf;
	uint8_t *in;
	uint8_t *out;
	
	in = this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;
    k = 0;
	for(i=0;i<this->mem->in_size;i++)
	{
		/* insert new byte into buffer */
		this->oi_queues[k].push(in[i]);
		
		/* copy last byte from buffer */
		out[i] = this->oi_queues[k].front(); 
		this->oi_queues[k].pop();
		k++;
		if(k == OI_SIZE)
			k = 0;
	}
	this->mem->free_out(out);
	this->mem->free_in(in);

	return true;
}
