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

DVBT_ifft::DVBT_ifft(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings)
{
	this->fd_in = fd_in;
	this->fd_out = fd_out;
	this->dvbt_settings = dvbt_settings;
	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
		
	this->in_multiple_of = this->dvbt_settings->ofdmcarriers * sizeof(dvbt_complex_t);
	this->out_multiple_of = (this->dvbt_settings->ofdmmode + this->dvbt_settings->guardcarriers) * sizeof(dvbt_complex_t) * this->dvbt_settings->oversampling;

	this->mem = new DVBT_memory(fd_in,fd_out,this->in_multiple_of,this->out_multiple_of,false);
	this->tmp = new dvbt_complex_t[this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling];
	this->buf = new dvbt_complex_t[this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling];
	
	memset(this->tmp, 0, this->dvbt_settings->ofdmmode * sizeof(dvbt_complex_t)*this->dvbt_settings->oversampling);
	
	this->fftshift_offset = (this->dvbt_settings->ofdmmode * this->dvbt_settings->oversampling - this->dvbt_settings->ofdmcarriers + 1)/2;
}


DVBT_ifft::~DVBT_ifft()
{
	delete[] this->buf;
	delete[] this->tmp;
}

bool DVBT_ifft::encode()
{
	dvbt_complex_t *in;
	fftwf_complex *out;
	
	in = (dvbt_complex_t*)this->mem->get_in();
	if(!in)
		return false;
	out = (fftwf_complex*)this->mem->get_out();
	if(!out)
		return false;
	
	this->p = fftwf_plan_dft_1d(this->dvbt_settings->ofdmmode * this->dvbt_settings->oversampling, (fftwf_complex*)this->buf, 
		out+(this->dvbt_settings->guardcarriers*this->dvbt_settings->oversampling), FFTW_BACKWARD, FFTW_ESTIMATE);
	
	this->fftshift(in,this->buf);
	fftwf_execute(this->p);
	
	//insert <guardcarriers> to the beginning of the buffer
	memcpy(out,out+this->dvbt_settings->ofdmmode*this->dvbt_settings->oversampling,
		this->dvbt_settings->guardcarriers*sizeof(dvbt_complex_t)*this->dvbt_settings->oversampling);
	
	fftwf_destroy_plan(this->p);

	this->mem->free_out((uint8_t*)out);
	this->mem->free_in((uint8_t*)in);
	
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
