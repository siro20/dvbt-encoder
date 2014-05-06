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

#include "tps.hpp"

using namespace std;

DVBT_tps::DVBT_tps(DVBT_settings *dvbt_settings)
{
	int frame, i;
	
	for( frame = 0; frame < 4; frame++ )
	{
		/* tps first bit always 0 */
		this->tpsarray[frame][0] = 0;
		/* tps sync */
		if ( !(frame & 1) )
		{
			this->tpsarray[frame][1] = 0;
			this->tpsarray[frame][2] = 0;
			this->tpsarray[frame][3] = 1;
			this->tpsarray[frame][4] = 1;
			this->tpsarray[frame][5] = 0;
			this->tpsarray[frame][6] = 1;
			this->tpsarray[frame][7] = 0;
			this->tpsarray[frame][8] = 1;
			this->tpsarray[frame][9] = 1;
			this->tpsarray[frame][10] = 1;
			this->tpsarray[frame][11] = 1;
			this->tpsarray[frame][12] = 0;
			this->tpsarray[frame][13] = 1;
			this->tpsarray[frame][14] = 1;
			this->tpsarray[frame][15] = 1;
			this->tpsarray[frame][16] = 0;
		}
		else
		{
			this->tpsarray[frame][1] = 1;
			this->tpsarray[frame][2] = 1;
			this->tpsarray[frame][3] = 0;
			this->tpsarray[frame][4] = 0;
			this->tpsarray[frame][5] = 1;
			this->tpsarray[frame][6] = 0;
			this->tpsarray[frame][7] = 1;
			this->tpsarray[frame][8] = 0;
			this->tpsarray[frame][9] = 0;
			this->tpsarray[frame][10] = 0;
			this->tpsarray[frame][11] = 0;
			this->tpsarray[frame][12] = 1;
			this->tpsarray[frame][13] = 0;
			this->tpsarray[frame][14] = 0;
			this->tpsarray[frame][15] = 0;
			this->tpsarray[frame][16] = 1;
		}
		/* tps length indicator */
		this->tpsarray[frame][17] = 0;
		this->tpsarray[frame][18] = 1;
		this->tpsarray[frame][19] = 0; //set to 0 if no cellid is beeing transmitted
		this->tpsarray[frame][20] = 1;  
		this->tpsarray[frame][21] = 1;
		this->tpsarray[frame][22] = 1;
		  
		/* tps frame bits */
		this->tpsarray[frame][23] = ((frame & 0x02)>>1);
		this->tpsarray[frame][24] = (frame & 0x01);

		/* tps constelation bits */
		if( dvbt_settings->modulation == 2 )
		{
			this->tpsarray[frame][25] = 0;
			this->tpsarray[frame][26] = 0;
		}
		else if ( dvbt_settings->modulation == 4 )
		{
			this->tpsarray[frame][25] = 0;
			this->tpsarray[frame][26] = 1;
		}
		else
		{
			this->tpsarray[frame][25] = 1;
			this->tpsarray[frame][26] = 0;
		}

		/* tps hierarchical mode bits */
		if( dvbt_settings->alpha == 1 )
		{
			this->tpsarray[frame][27] = 0;
			this->tpsarray[frame][28] = 0;
			this->tpsarray[frame][29] = 0;
		}
		else
		{
			/* TODO */
		}
		/* tps code rate  bits */
		if( dvbt_settings->coderate == 2 )
		{
			this->tpsarray[frame][30] = 0;
			this->tpsarray[frame][31] = 0;
			this->tpsarray[frame][32] = 0;
		}
		else if( dvbt_settings->coderate == 3 )
		{
			this->tpsarray[frame][30] = 0;
			this->tpsarray[frame][31] = 0;
			this->tpsarray[frame][32] = 1;
		}
		else if( dvbt_settings->coderate == 4 )
		{
			this->tpsarray[frame][30] = 0;
			this->tpsarray[frame][31] = 1;
			this->tpsarray[frame][32] = 0;
		}
		else if( dvbt_settings->coderate == 5 )
		{
			this->tpsarray[frame][30] = 0;
			this->tpsarray[frame][31] = 1;
			this->tpsarray[frame][32] = 1;
		}
		else
		{
			this->tpsarray[frame][30] = 1;
			this->tpsarray[frame][31] = 0;
			this->tpsarray[frame][32] = 0;
		}

		/* tps code rate bits in hierarchical */
		/* TODO */
		this->tpsarray[frame][33] = 0;
		this->tpsarray[frame][34] = 0;
		this->tpsarray[frame][35] = 0;

		/* tps guard interval bits */
		if( dvbt_settings->guardinterval == 4 )
		{
			this->tpsarray[frame][36] = 1;
			this->tpsarray[frame][37] = 1;
		}
		else if( dvbt_settings->guardinterval == 8 )
		{
			this->tpsarray[frame][36] = 1;
			this->tpsarray[frame][37] = 0;
		}
		else if( dvbt_settings->guardinterval == 16 )
		{
			this->tpsarray[frame][36] = 0;
			this->tpsarray[frame][37] = 1;
		}
		else if( dvbt_settings->guardinterval == 32 )
		{
			this->tpsarray[frame][36] = 0;
			this->tpsarray[frame][37] = 0;
		}

		/* tps transmission mode bits */
		if( dvbt_settings->ofdmmode == 8192 )
		{
			this->tpsarray[frame][38] = 0;
			this->tpsarray[frame][39] = 1;
		}
		else
		{
			this->tpsarray[frame][38] = 0;
			this->tpsarray[frame][39] = 0;
		}

		/* tps cell id bits - optional */
		if( frame & 1 )
		{
			this->tpsarray[frame][40] = (dvbt_settings->cellid & 0x8000) >> 15;
			this->tpsarray[frame][41] = (dvbt_settings->cellid & 0x4000) >> 14;
			this->tpsarray[frame][42] = (dvbt_settings->cellid & 0x2000) >> 13;
			this->tpsarray[frame][43] = (dvbt_settings->cellid & 0x1000) >> 12;
			this->tpsarray[frame][44] = (dvbt_settings->cellid & 0x0800) >> 11;
			this->tpsarray[frame][45] = (dvbt_settings->cellid & 0x0400) >> 10;
			this->tpsarray[frame][46] = (dvbt_settings->cellid & 0x0200) >> 9;
			this->tpsarray[frame][47] = (dvbt_settings->cellid & 0x0100) >> 8;
		}
		else
		{
			this->tpsarray[frame][40] = (dvbt_settings->cellid & 0x0080) >> 7;
			this->tpsarray[frame][41] = (dvbt_settings->cellid & 0x0040) >> 6;
			this->tpsarray[frame][42] = (dvbt_settings->cellid & 0x0020) >> 5;
			this->tpsarray[frame][43] = (dvbt_settings->cellid & 0x0010) >> 4;
			this->tpsarray[frame][44] = (dvbt_settings->cellid & 0x0008) >> 3;
			this->tpsarray[frame][45] = (dvbt_settings->cellid & 0x0004) >> 2;
			this->tpsarray[frame][46] = (dvbt_settings->cellid & 0x0002) >> 1;
			this->tpsarray[frame][47] = (dvbt_settings->cellid & 0x0001) >> 0;
		}

		/* tps unused bits */
		this->tpsarray[frame][48] = 0;
		this->tpsarray[frame][49] = 0;
		this->tpsarray[frame][50] = 0;
		this->tpsarray[frame][51] = 0;
		this->tpsarray[frame][52] = 0;
		this->tpsarray[frame][53] = 0;

		//do a shortened BCH (127, 113, 14)
		this->generate_bch_code(&this->tpsarray[frame][0]);
		
		/* do dpsk modulation */
		this->dpsktpsarray[frame][0] = 1.0f;
		for(i = 1; i < 68; i++)
		{
			if( this->tpsarray[frame][i] == 0 )
			{
				this->dpsktpsarray[frame][i] = this->dpsktpsarray[frame][i-1];
			}
			else
			{
				this->dpsktpsarray[frame][i] = this->dpsktpsarray[frame][i-1] * -1.0f;
			}
		}
	}
}

DVBT_tps::~DVBT_tps()
{
}

// NOTICE: this code was taken from 
// Copyright 2013 <Bogdan Diaconescu, yo3iiu@yo3iiu.ro>.
// https://github.com/BogdanDIA/gr-dvbt/blob/master/lib/reference_signals_impl.cc

/*
* Generate shortened BCH(67, 53) codes from TPS data
* Extend the code with 60 bits and use BCH(127, 113)
*/
void
DVBT_tps::generate_bch_code(unsigned char d_tps_data[68])
{

	//(n, k) = (127, 113) = (60+67, 60+53)
	unsigned int reg_bch = 0;
	unsigned char data_in[113];

	//fill in 60 zeros
	memset(&data_in[0], 0, 60);
	//fill in TPS data - start bit not included
	memcpy(&data_in[60], &d_tps_data[1], 53);

	//X^14+X^9+X^8+X^6+X^5+X^4+X^2+X+1
	for (int i = 0; i < 113; i++)
	{
		int feedback = 0x1 & (data_in[i] ^ reg_bch);
		reg_bch = reg_bch >> 1;
		reg_bch |= feedback << 13;
		reg_bch = reg_bch \
			^ (feedback << 12) ^ (feedback << 11) \
			^ (feedback << 9) ^ (feedback << 8) \
			^ (feedback << 7) ^ (feedback << 5) \
			^ (feedback << 4);
	}

	for (int i = 0; i < 14; i++)
		d_tps_data[i + 54] = 0x1 & (reg_bch >> i);
}
