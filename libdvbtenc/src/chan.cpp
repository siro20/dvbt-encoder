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

DVBT_chan::DVBT_chan(FILE *fd_in, FILE *fd_out, DVBT_settings* dvbt_settings)
{
	int frame,i;
	DVBT_tps dvbt_tps(dvbt_settings);
	
    this->fd_in = fd_in;
    this->fd_out = fd_out;
    this->dvbt_settings = dvbt_settings;
	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
    this->in_multiple_of = this->dvbt_settings->ofdmuseablecarriers * sizeof(dvbt_complex_t);
    this->out_multiple_of = this->dvbt_settings->ofdmcarriers * sizeof(dvbt_complex_t);


	this->mem = new DVBT_memory(this->in_multiple_of,this->out_multiple_of, true);
    
    for(frame=0;frame<this->dvbt_settings->DVBT_FRAMES_SUPERFRAME;frame++)
    {
		for(i=0;i<this->dvbt_settings->DVBT_SYMBOLS_FRAME;i++)
		{
			this->lookup[frame][i] = new DVBT_pilots(frame,i,&dvbt_tps,dvbt_settings);
		}
	}
}


DVBT_chan::~DVBT_chan()
{
}

int DVBT_chan::encode(unsigned int frame, unsigned int symbol)
{
	int rret, wret;
	dvbt_complex_t *out;
	dvbt_complex_t *in;
	
	if(symbol >= this->dvbt_settings->DVBT_SYMBOLS_FRAME)
		return 1;
	if(frame >= this->dvbt_settings->DVBT_FRAMES_SUPERFRAME)
		return 1;

	rret = this->mem->read(this->fd_in);
	out = (dvbt_complex_t*)this->mem->out;
	in = (dvbt_complex_t*)this->mem->in;
	
	this->lookup[frame][symbol]->encode(in,out);

	wret = this->mem->write(this->fd_out);

	if(rret || wret)
		return 1;
	return 0;
}
