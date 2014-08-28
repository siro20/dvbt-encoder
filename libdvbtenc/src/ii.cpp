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

DVBT_ii::DVBT_ii(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->dvbt_settings = dvbt_settings;

	this->mReadSize = this->dvbt_settings->modulation * this->dvbt_settings->DVBT_II_DEPTH;
	this->mWriteSize = this->dvbt_settings->DVBT_II_DEPTH;
	this->pin = pin;
	this->pout = pout;

	pin->initReadEnd( this->mReadSize );

	this->lookup = new int[this->dvbt_settings->modulation * this->dvbt_settings->DVBT_II_DEPTH];

	static const uint8_t shiftreg_indx[6] = {0,63,105,42,21,84};
	
	if(this->dvbt_settings->modulation==2)
	{
		int idx=0;
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			lookup[idx++] = ((i+shiftreg_indx[0])%126)*2+0;
			lookup[idx++] = ((i+shiftreg_indx[1])%126)*2+1;
		}
	}
	else if(this->dvbt_settings->modulation==4)
	{
		int idx=0;
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
		int idx=0;
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

bool DVBT_ii::encode()
{
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	if(this->dvbt_settings->modulation==2)
	{
		int idx=0;
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			out->ptr[i] = (in->ptr[this->lookup[idx+0]] ? 0x02 : 0)
				| (in->ptr[this->lookup[idx+1]] ? 0x01 : 0);
			idx+=2;
		}
	}
	else if(this->dvbt_settings->modulation==4)
	{
		int idx=0;
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			out->ptr[i] = (in->ptr[this->lookup[idx+0]] ? 0x08 : 0)
				| (in->ptr[this->lookup[idx+1]] ? 0x04 : 0)
				| (in->ptr[this->lookup[idx+2]] ? 0x02 : 0)
				| (in->ptr[this->lookup[idx+3]] ? 0x01 : 0);
			idx+=4;
		}
	}
	else
	{
		int idx=0;
		for(int i=0;i<this->dvbt_settings->DVBT_II_DEPTH;i++)
		{
			out->ptr[i] = (in->ptr[this->lookup[idx+0]] ? 0x20 : 0)
				| (in->ptr[this->lookup[idx+1]] ? 0x10 : 0)
				| (in->ptr[this->lookup[idx+2]] ? 0x08 : 0)
				| (in->ptr[this->lookup[idx+3]] ? 0x04 : 0)
				| (in->ptr[this->lookup[idx+4]] ? 0x02 : 0)
				| (in->ptr[this->lookup[idx+5]] ? 0x01 : 0);
			idx+=6;
		}
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
