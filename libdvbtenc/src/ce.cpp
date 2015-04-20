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

#include "ce.hpp"

using namespace std;

DVBT_ce::DVBT_ce(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->pin = pin;
	this->pout = pout;
	this->dvbt_settings = dvbt_settings;
	
	this->mWriteSize = this->dvbt_settings->ofdmuseablecarriers * this->dvbt_settings->modulation;
	this->mReadSize = ((this->dvbt_settings->coderate -1) * this->dvbt_settings->ofdmuseablecarriers * this->dvbt_settings->modulation);
	//encode multiple symbols for coderate > 2/3
	while(this->mReadSize % (this->dvbt_settings->coderate * 8))
	{
		this->mReadSize *= 2;
		this->mWriteSize *= 2;
	}
	this->mReadSize /= (this->dvbt_settings->coderate * 8);
	
	pin->initReadEnd( this->mReadSize );
	
	this->shiftreg = 0x00;
}

DVBT_ce::~DVBT_ce()
{
}

#define CALC_X_Y() { \
uint16_t tmp[5]; \
this->shiftreg |= in->ptr[i]; \
tmp[0] = this->shiftreg >> 6; \
tmp[1] = this->shiftreg >> 5; \
tmp[2] = this->shiftreg >> 3; \
tmp[3] = this->shiftreg >> 2; \
tmp[4] = this->shiftreg >> 1; \
x = tmp[0] ^ tmp[2] ^ tmp[3] ^ tmp[4] ^ this->shiftreg; \
y = tmp[0] ^ tmp[1] ^ tmp[2] ^ tmp[3] ^ this->shiftreg;

void DVBT_ce::conv_encoder_12(DVBT_memory *in, DVBT_memory *out)
{
	uint8_t *outptr = out->ptr;

	for(int i=0;i<in->size;i++) {
		uint16_t x;
		uint16_t y;

		CALC_X_Y

		for(int j=7;j>-1;j--) {
			*outptr = (x >> j) & 1; outptr++;
			*outptr = (y >> j) & 1; outptr++;
		}
		this->shiftreg <<= 8;
	}
}

void DVBT_ce::conv_encoder_23(DVBT_memory *in, DVBT_memory *out)
{
	int z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(int i=0;i<in->size;i++) {
		uint16_t x;
		uint16_t y;

		CALC_X_Y

		for(int j=7;j>-1;j--) {
			if( z != 2 ) { // remove X2
				*outptr = (x >> j) & 1; outptr++;
			}
			z++;
			*outptr = (y >> j) & 1; outptr++;
			z++;
			z &= 0x3; // z modulo 4
		}
		this->shiftreg <<= 8;
	}
}

void DVBT_ce::conv_encoder_34(DVBT_memory *in, DVBT_memory *out)
{
	int z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(int i=0;i<in->size;i++) {
		uint16_t x;
		uint16_t y;

		CALC_X_Y

		for(int j=7;j>-1;j--) {
			if( z != 2 ) { // remove X2 from stream
				*outptr = (x >> j) & 1; outptr++;
			}
			z++;
			if( z != 5 ) { // remove Y3 from stream
				*outptr = (y >> j) & 1; outptr++;
			}
			z++;
			z = z % 6; // z modulo 6
		}
		this->shiftreg <<= 8;
	}
}


void DVBT_ce::conv_encoder_56(DVBT_memory *in, DVBT_memory *out)
{
	int z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(int i=0;i<in->size;i++) {
		uint16_t x;
		uint16_t y;

		CALC_X_Y

		for(int j=7;j>-1;j--) {
			if( z != 2 && z != 6 ) { // remove X2 and X4 from stream
				*outptr = (x >> j) & 1; outptr++;
			}
			z++;
			if( z != 5 && z != 9 ) { // remove Y3 and Y5 from stream
				*outptr = (y >> j) & 1; outptr++;
			}
			z++;
			z = z % 10; // z modulo 10
		}
		this->shiftreg <<= 8;
	}
}


void DVBT_ce::conv_encoder_78(DVBT_memory *in, DVBT_memory *out)
{
	int z;
	uint8_t *outptr = out->ptr;
	z = 0;
	for(int i=0;i<in->size;i++) {
		uint16_t x;
		uint16_t y;

		CALC_X_Y

		for(int j=7;j>-1;j--) {
			if( z != 2 && z != 6 && z != 10 ) { // remove X2 and X3 and X4 from stream
				*outptr = (x >> j) & 1; outptr++;
			}
			z++;
			if( z != 5 && z != 9 &&  z != 13 ) { // remove Y5 and X6 and Y7 from stream
				*outptr = (y >> j) & 1; outptr++;
			}
			z++;
			z = z % 14; // z modulo 14
		}
		this->shiftreg <<= 8;
	}
}

bool DVBT_ce::encode()
{
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	switch(this->dvbt_settings->coderate)
	{
		case 2:
			conv_encoder_12 (in, out);
			break;
		case 3:
			conv_encoder_23 (in, out);
			break;
		case 4:
			conv_encoder_34 (in, out);
			break;
		case 6:
			conv_encoder_56 (in, out);
			break;
		case 8:
			conv_encoder_78 (in, out);
			break;
		default:
			conv_encoder_12 (in, out);
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
