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
	this->out_multiple_of = (this->dvbt_settings->ofdmmode + this->dvbt_settings->guardcarriers) * sizeof(dvbt_complex_t);


	this->mem = new DVBT_memory(this->in_multiple_of,this->out_multiple_of);

	this->buf = new dvbt_complex_t[dvbt_settings->ofdmmode];
	memset(this->buf, 0, this->dvbt_settings->ofdmmode * sizeof(dvbt_complex_t));
	this->p = fftwf_plan_dft_1d(this->dvbt_settings->ofdmmode, (fftwf_complex*)this->buf, 
			((fftwf_complex*)this->mem->out)+this->dvbt_settings->guardcarriers, FFTW_BACKWARD, FFTW_MEASURE);
}


DVBT_ifft::~DVBT_ifft()
{
	fftwf_destroy_plan(this->p);
	delete[] this->buf;
}

int DVBT_ifft::encode()
{
	int rret, wret;
	
	rret = this->mem->read(this->fd_in);
	this->fftshift((dvbt_complex_t*)this->mem->in,this->buf);
	fftwf_execute(this->p);
	memcpy(this->mem->out,this->mem->out+this->dvbt_settings->ofdmmode*sizeof(dvbt_complex_t),this->dvbt_settings->guardcarriers*sizeof(dvbt_complex_t));
	wret = this->mem->write(this->fd_out);

	if(rret || wret)
		return 1;
	return 0;
}

void DVBT_ifft::fftshift( dvbt_complex_t *in, dvbt_complex_t *out )
{
	int i;
	
	dvbt_complex_t *tmp = new dvbt_complex_t[this->dvbt_settings->ofdmmode];
	memset(tmp,0,this->dvbt_settings->ofdmmode * sizeof(dvbt_complex_t));
	
	/* calculate offset */
	i = (this->dvbt_settings->ofdmmode - this->dvbt_settings->ofdmcarriers + 1)/2;

	/* shift */
	memcpy(tmp + i , in , this->dvbt_settings->ofdmcarriers * sizeof(dvbt_complex_t) );

	/* swap */
	memcpy(out, tmp + this->dvbt_settings->ofdmmode/2, this->dvbt_settings->ofdmmode / 2 * sizeof(dvbt_complex_t));
	memcpy(out + this->dvbt_settings->ofdmmode/2 , tmp, this->dvbt_settings->ofdmmode / 2 * sizeof(dvbt_complex_t));

	delete[] tmp;
}
