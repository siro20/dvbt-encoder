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

#include "ed.hpp"

using namespace std;

DVBT_ed::DVBT_ed(FILE *fd_in, FILE *fd_out)
{
    int i,j;
	int pbrs = 0xa9;
	unsigned int out=0;
	this->in_multiple_of = 188*8;
	this->out_multiple_of = 188*8;
	this->pbrs_seq = new uint8_t[188*8];
	this->internal_buf = new uint8_t[188*8];

	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
		
	/* gen pbrs sequence */
	for(i=1;i<188*8;i++)
	{
		for(j=0;j<8;j++){
			pbrs <<= 1;
			out <<=1;
			if( ((pbrs>>15)&0x01) ^ ((pbrs>>14)&0x01) )
			{
				pbrs |= 1;
				out |= 1;
			}
		}
        this->pbrs_seq[i] = (uint8_t)(out&0xff);

        out = 0;
	}

	/* keep the first sync byte, xor with 0xff */
	this->pbrs_seq[0] = 0xff;

	/* toggle the sync bytes, xor with 0x00 */
	for(i=188;i<188*8;i+=188)
	{
		this->pbrs_seq[i] = 0x00;
	}


	this->mem = new DVBT_memory(fd_in, fd_out, this->in_multiple_of,this->out_multiple_of,false);

	this->fd_in = fd_in;
	this->fd_out = fd_out;
	this->cnt = 0;
}

DVBT_ed::~DVBT_ed()
{
	delete[] this->pbrs_seq;
	delete[] this->internal_buf;
}

//encodes length packets, assuming that the MPEG TS sync byte is at offset 0
bool DVBT_ed::encode()
{
	unsigned int i;
	uint8_t *in;
	uint8_t *out;
	
	in = this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;
		
#if _WIN64 || __amd64__
	for(i=0;i<this->mem->in_size/sizeof(uint64_t);i++)
	{
		((uint64_t*)out)[i] = ((uint64_t*)in)[i] ^ ((uint64_t*)this->pbrs_seq)[this->cnt];
		this->cnt ++;
		if( this->cnt >= 188*8/sizeof(uint64_t) )
			this->cnt = 0;
	}
#else
	for(i=0;i<this->mem->in_size/sizeof(uint32_t);i++)
	{
		((uint32_t*)out)[i] = ((uint32_t*)in)[i] ^ ((uint32_t*)this->pbrs_seq)[this->cnt];
		this->cnt ++;
		if( this->cnt >= 188*8/sizeof(uint32_t) )
			this->cnt = 0;
	}
#endif
	this->mem->free_out(out);
	this->mem->free_in(in);
	
	return true;
}
