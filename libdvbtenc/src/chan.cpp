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

#include "chan.hpp"

using namespace std;

DVBT_chan::DVBT_chan(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	DVBT_tps dvbt_tps(dvbt_settings);

	this->dvbt_settings = dvbt_settings;
	this->mReadSize = this->dvbt_settings->ofdmuseablecarriers * sizeof(dvbt_complex_t);
	this->mWriteSize = this->dvbt_settings->ofdmcarriers * sizeof(dvbt_complex_t);
	this->pin = pin;
	this->pout = pout;
	this->pin->initReadEnd( this->mReadSize );

	for(unsigned int frame=0; frame < this->dvbt_settings->DVBT_FRAMES_SUPERFRAME; frame++)
	{
		for(unsigned int i=0; i < this->dvbt_settings->DVBT_SYMBOLS_FRAME; i++)
		{
			this->dvbt_pilots[frame][i] = new DVBT_pilots(frame,i,&dvbt_tps,dvbt_settings,false);
		}
	}
}


DVBT_chan::~DVBT_chan()
{
}

bool DVBT_chan::encode(int frame, int symbol)
{
	if(symbol >= this->dvbt_settings->DVBT_SYMBOLS_FRAME)
		return false;
	if(frame >= this->dvbt_settings->DVBT_FRAMES_SUPERFRAME)
		return false;

	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	this->dvbt_pilots[frame][symbol]->encode((dvbt_complex_t*)(in->ptr),(dvbt_complex_t*)(out->ptr));

	delete in;

	if(!this->pout->write(out))
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	return true;
}
