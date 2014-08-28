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

DVBT_oi::DVBT_oi(DVBT_pipe *pin, DVBT_pipe *pout)
{
	this->mReadSize = OI_SIZE;
	this->mWriteSize = OI_SIZE;
	this->pin = pin;
	this->pout = pout;

	this->pin->initReadEnd( this->mReadSize );

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
	unsigned int k = 0;
	
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	for( unsigned int i=0; i < in->size; i++ )
	{
		/* insert new byte into buffer */
		this->oi_queues[k].push(in->ptr[i]);
		
		/* copy last byte from buffer */
		out->ptr[i] = this->oi_queues[k].front(); 
		this->oi_queues[k].pop();
		k++;
		if(k == OI_SIZE)
			k = 0;
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
