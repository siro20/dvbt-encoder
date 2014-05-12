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

#define OI_SIZE 12
#define OI_DEPTH 17
#define OI_BUFSIZE (OI_DEPTH * (1+2+3+4+5+6+7+8+9+10+11)+12)

DVBT_oi::DVBT_oi(FILE *fd_in, FILE *fd_out)
{
	this->in_multiple_of = 12;
	this->out_multiple_of = 12;
	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
	this->mem = new DVBT_memory(fd_in,fd_out,this->in_multiple_of,this->out_multiple_of,false);

	this->fd_in = fd_in;
	this->fd_out = fd_out;
	this->oi_buf = new uint8_t[OI_BUFSIZE];
	memset(this->oi_buf, 0, OI_BUFSIZE);
}

DVBT_oi::~DVBT_oi()
{
	delete[] this->oi_buf;
}

bool DVBT_oi::encode()
/* interleave data byteswise, oi_buf has size 17*(1+2+3+4+5+6+7+8+9+10+11) + 12, reset on startup, n has to be multiple of 12 */
/* data written to data_out is same as data_cnt */
{
	int i,j,k;
	unsigned char *p_oi_buf;
	uint8_t *in;
	uint8_t *out;
	
	in = this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;

	for(i=0;i<this->mem->in_size;i++)
	{
		/* TODO: use counter for better performance ? */
		j = ( i % OI_SIZE );
		if( j == 0 )
			p_oi_buf = this->oi_buf;
		
		/* insert new byte into buffer */
		p_oi_buf[0] = in[i];
		
		/* copy last byte from buffer */
		out[i] = p_oi_buf[j * OI_DEPTH];
		
		/* shift depth * j bytes */
		for(k = j * OI_DEPTH; k > 0; k--)
		{
			p_oi_buf[k] = p_oi_buf[k-1];
		}
		
		/* increment pointer to next buffer */
		p_oi_buf += j * OI_DEPTH + 1;
	}

	this->mem->free_out(out);
	this->mem->free_in(in);

	return true;
}
