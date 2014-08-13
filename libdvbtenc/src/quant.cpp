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

DVBT_quant::DVBT_quant(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings)
{
    this->fd_in = fd_in;
    this->fd_out = fd_out;
    this->dvbt_settings = dvbt_settings;
    // encode one ofdm symbol at time
    this->in_multiple_of = sizeof(dvbt_complex_t) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
    switch(this->dvbt_settings->outputformat)
	{
		case CHAR:
		case UCHAR:
			this->out_multiple_of = 2*sizeof(char) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
			break;
		case SHORT:
		case USHORT:
			this->out_multiple_of = 2*sizeof(short int) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);

			break;
		case FLOAT:
			this->out_multiple_of = sizeof(dvbt_complex_t) * (this->dvbt_settings->ofdmmode+this->dvbt_settings->guardcarriers);
			break;
	}

    this->mem = new DVBT_memory(fd_in,fd_out,this->in_multiple_of,this->out_multiple_of, true);
}

DVBT_quant::~DVBT_quant()
{
}

bool DVBT_quant::encode()
{
	unsigned int i;
	dvbt_complex_t* in;
	uint8_t *out;
	
	in = (dvbt_complex_t*)this->mem->get_in();
	if(!in)
		return false;
	out = this->mem->get_out();
	if(!out)
		return false;
		
	
	switch(this->dvbt_settings->outputformat)
	{
		case 0:
			for(i=0;i<this->mem->in_size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_char_t*)out)[i].x = (char)in[i].x*this->dvbt_settings->normalisation;
				((dvbt_complex_char_t*)out)[i].y = (char)in[i].y*this->dvbt_settings->normalisation;
			}
			break;
		case 1:
			for(i=0;i<this->mem->in_size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_uchar_t*)out)[i].x = ((unsigned char)in[i].x*this->dvbt_settings->normalisation)+0x80;
				((dvbt_complex_uchar_t*)out)[i].y = ((unsigned char)in[i].y*this->dvbt_settings->normalisation)+0x80;
			}
			break;
		case 2:
			for(i=0;i<this->mem->in_size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_short_t*)out)[i].x = (short int)in[i].x*this->dvbt_settings->normalisation;
				((dvbt_complex_short_t*)out)[i].y = (short int)in[i].y*this->dvbt_settings->normalisation;
			}
			break;
		case 3:
			for(i=0;i<this->mem->in_size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_ushort_t*)out)[i].x = ((unsigned short int)in[i].x*this->dvbt_settings->normalisation)+0x8000;
				((dvbt_complex_ushort_t*)out)[i].y = ((unsigned short int)in[i].y*this->dvbt_settings->normalisation)+0x8000;
			}
			break;
		case 4:
			for(i=0;i<this->mem->in_size/sizeof(dvbt_complex_t);i++)
			{
				((dvbt_complex_t*)out)[i].x = in[i].x*this->dvbt_settings->normalisation;
				((dvbt_complex_t*)out)[i].y = in[i].y*this->dvbt_settings->normalisation;
			}
			break;
	}
	this->mem->free_out(out);
	this->mem->free_in((uint8_t*)in);

	return true;
}
