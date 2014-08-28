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

DVBT_ed::DVBT_ed(DVBT_pipe *pin, DVBT_pipe *pout)
{
    int i,j;
	int pbrs = 0xa9;
	unsigned int out=0;

	this->pbrs_seq = new uint8_t[188*8];
	this->internal_buf = new uint8_t[188*8];
	this->pin = pin;
	this->pout = pout;
	
	pin->initReadEnd( 188 * 8 );
	
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
	unsigned int cnt = 0;
	DVBT_memory *in;
	
	in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( 8 * 188 );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	for(i = 0; i < in->size / sizeof(size_t); i++)
	{
		((size_t*)out->ptr)[i] = ((size_t*)in->ptr)[i] ^ ((size_t*)this->pbrs_seq)[cnt];
		cnt++;
		if( cnt >= 188*8 / sizeof(size_t) )
			cnt = 0;
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
