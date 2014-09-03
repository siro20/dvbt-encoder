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

#include "quant.hpp"

using namespace std;

DVBT_quant::DVBT_quant(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->dvbt_settings = dvbt_settings;

	this->mReadSize = sizeof(dvbt_complex_t) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
	this->mWriteSize = 0;
	this->pin = pin;
	this->pout = pout;

	switch(this->dvbt_settings->outputformat)
	{
		case CHAR:
		case UCHAR:
			this->mWriteSize = 2 * sizeof(char) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
			break;
		case SHORT:
		case USHORT:
			this->mWriteSize = 2 * sizeof(short int) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
			break;
		case FLOAT:
			this->mWriteSize = sizeof(dvbt_complex_t) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
			break;
	}

	pin->initReadEnd( this->mReadSize );
}

DVBT_quant::~DVBT_quant()
{
}

bool DVBT_quant::encode()
{
	unsigned int i;
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	dvbt_complex_t *inptr = (dvbt_complex_t*)(in->ptr);

	switch(this->dvbt_settings->outputformat)
	{
		case 0:
			for(i=0;i<in->size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_char_t*)(out->ptr))[i].x = (char)inptr[i].x*this->dvbt_settings->normalisation;
				((dvbt_complex_char_t*)(out->ptr))[i].y = (char)inptr[i].y*this->dvbt_settings->normalisation;
			}
			break;
		case 1:
			for(i=0;i<in->size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_uchar_t*)(out->ptr))[i].x = ((unsigned char)inptr[i].x*this->dvbt_settings->normalisation)+0x80;
				((dvbt_complex_uchar_t*)(out->ptr))[i].y = ((unsigned char)inptr[i].y*this->dvbt_settings->normalisation)+0x80;
			}
			break;
		case 2:
			for(i=0;i<in->size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_short_t*)(out->ptr))[i].x = (short int)inptr[i].x*this->dvbt_settings->normalisation;
				((dvbt_complex_short_t*)(out->ptr))[i].y = (short int)inptr[i].y*this->dvbt_settings->normalisation;
			}
			break;
		case 3:
			for(i=0;i<in->size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_ushort_t*)(out->ptr))[i].x = ((unsigned short int)inptr[i].x*this->dvbt_settings->normalisation)+0x8000;
				((dvbt_complex_ushort_t*)(out->ptr))[i].y = ((unsigned short int)inptr[i].y*this->dvbt_settings->normalisation)+0x8000;
			}
			break;
		case 4:
			for(i=0;i<in->size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_t*)(out->ptr))[i].x = inptr[i].x*this->dvbt_settings->normalisation;
				((dvbt_complex_t*)(out->ptr))[i].y = inptr[i].y*this->dvbt_settings->normalisation;
			}
			break;
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
