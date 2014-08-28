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

#include "ifft.hpp"

using namespace std;

DVBT_ifft::DVBT_ifft(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	this->dvbt_settings = dvbt_settings;

	this->mReadSize = this->dvbt_settings->ofdmcarriers * sizeof(dvbt_complex_t);
	this->mWriteSize = (this->dvbt_settings->ofdmmode + this->dvbt_settings->guardcarriers) * sizeof(dvbt_complex_t) * this->dvbt_settings->oversampling;
	this->pin = pin;
	this->pout = pout;
	
	this->pin->initReadEnd( this->mReadSize );

	this->tmp = new dvbt_complex_t[this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling];
	this->bufA = new dvbt_complex_t[this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling];
	this->bufB = new dvbt_complex_t[this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling];
	this->p = fftwf_plan_dft_1d(this->dvbt_settings->ofdmmode * this->dvbt_settings->oversampling, (fftwf_complex*)this->bufA, 
		(fftwf_complex*)this->bufB, FFTW_BACKWARD, FFTW_ESTIMATE);

	memset(this->tmp, 0, this->dvbt_settings->ofdmmode * sizeof(dvbt_complex_t)*this->dvbt_settings->oversampling);
	
	this->fftshift_offset = (this->dvbt_settings->ofdmmode * this->dvbt_settings->oversampling - this->dvbt_settings->ofdmcarriers + 1)/2;
}


DVBT_ifft::~DVBT_ifft()
{
	fftwf_destroy_plan(this->p);
	delete[] this->bufA;
	delete[] this->bufB;
	delete[] this->tmp;
}

bool DVBT_ifft::encode()
{
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	this->fftshift((dvbt_complex_t*)(in->ptr),this->bufA);
	fftwf_execute(this->p);

	memcpy(((dvbt_complex_t*)out->ptr)+this->dvbt_settings->guardcarriers*this->dvbt_settings->oversampling, bufB, 
		this->dvbt_settings->ofdmmode*sizeof(dvbt_complex_t)*this->dvbt_settings->oversampling);

	//insert <guardcarriers> to the beginning of the buffer
	memcpy(((dvbt_complex_t*)out->ptr),((dvbt_complex_t*)out->ptr)+this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling,
		this->dvbt_settings->guardcarriers*sizeof(dvbt_complex_t)*this->dvbt_settings->oversampling);

	delete in;

	if(!this->pout->write(out))
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	return true;
}

void DVBT_ifft::fftshift( dvbt_complex_t *in, dvbt_complex_t *out )
{
	/* shift */
	memcpy(this->tmp + this->fftshift_offset , in , this->dvbt_settings->ofdmcarriers * sizeof(dvbt_complex_t) );

	/* swap */
	memcpy(out, this->tmp + this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling/2, this->dvbt_settings->ofdmmode / 2 * sizeof(dvbt_complex_t) * this->dvbt_settings->oversampling);
	memcpy(out + this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling/2 , this->tmp, this->dvbt_settings->ofdmmode / 2 * sizeof(dvbt_complex_t) * this->dvbt_settings->oversampling);
}
