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

/* this->program is based uppon the work of
 Simon Rockliff, University of Adelaide   21/9/89
   26/6/91 Slight modifications to remove a compiler dependent bug which hadn't
           previously surfaced. A few extra comments added for clarity.
           Appears to all work fine, ready for posting to net!

                  Notice
                 --------
   this->program may be freely modified and/or given to whoever wants it.
   A condition of such distribution is that the author's contribution be
   acknowledged by his name being left in the comments heading the program,
   however no responsibility is accepted for any financial or other loss which
   may result from some unforseen errors or malfunctioning of the program
   during use.
                                 Simon Rockliff, 26th June 1991

*/

#include "rs.hpp"

DVBT_rs::DVBT_rs(DVBT_pipe *pin, DVBT_pipe *pout)
{
	// primitive polynomial p(x) = x8 + x4 + x3 + x2 + 1
	this->mReadSize = 188;
	this->mWriteSize = 204;
	this->pin = pin;
	this->pout = pout;
	
	this->pin->initReadEnd( this->mReadSize );
}

DVBT_rs::~DVBT_rs()
{
}

/*
 * this optimized version allows to do 4 galois multiplication runs in a single function
 * On every run 16 bytes are XORed at once.
 * Instead of shifting single bytes the "last" byte is read from wreg[0].
 */
void DVBT_rs::galois_mult( uint32_t wreg[5] )
{
	int i,j;
	uint32_t tmp;
	// using precalculated polynom, this encodes 16 bytes at once
	// regular approach is using two lookup tables to encode a single byte
	static const uint8_t xor_array[32][20] =
	{
		{0,59,13,104,189,68,209,30,8,163,65,41,229,98,50,36,59,0,0,0},	// generator polynom
		{0,118,26,208,103,136,191,60,16,91,130,82,215,196,100,72,118,0,0,0},
		{0,236,52,189,206,13,99,120,32,182,25,164,179,149,200,144,236,0,0,0},
		{0,197,104,103,129,26,198,240,64,113,50,85,123,55,141,61,197,0,0,0},
		{0,151,208,206,31,52,145,253,128,226,100,170,246,110,7,122,151,0,0,0},
		{0,51,189,129,62,104,63,231,29,217,200,73,241,220,14,244,51,0,0,0},
		{0,102,103,31,124,208,126,211,58,175,141,146,255,165,28,245,102,0,0,0},
		{0,204,206,62,248,189,252,187,116,67,7,57,227,87,56,247,204,0,0,0},
		{0,0,59,13,104,189,68,209,30,8,163,65,41,229,98,50,36,59,0,0},	// generator polynom
		{0,0,118,26,208,103,136,191,60,16,91,130,82,215,196,100,72,118,0,0},
		{0,0,236,52,189,206,13,99,120,32,182,25,164,179,149,200,144,236,0,0},
		{0,0,197,104,103,129,26,198,240,64,113,50,85,123,55,141,61,197,0,0},
		{0,0,151,208,206,31,52,145,253,128,226,100,170,246,110,7,122,151,0,0},
		{0,0,51,189,129,62,104,63,231,29,217,200,73,241,220,14,244,51,0,0},
		{0,0,102,103,31,124,208,126,211,58,175,141,146,255,165,28,245,102,0,0},
		{0,0,204,206,62,248,189,252,187,116,67,7,57,227,87,56,247,204,0,0},
		{0,0,0,59,13,104,189,68,209,30,8,163,65,41,229,98,50,36,59,0},	// generator polynom
		{0,0,0,118,26,208,103,136,191,60,16,91,130,82,215,196,100,72,118,0},
		{0,0,0,236,52,189,206,13,99,120,32,182,25,164,179,149,200,144,236,0},
		{0,0,0,197,104,103,129,26,198,240,64,113,50,85,123,55,141,61,197,0},
		{0,0,0,151,208,206,31,52,145,253,128,226,100,170,246,110,7,122,151,0},
		{0,0,0,51,189,129,62,104,63,231,29,217,200,73,241,220,14,244,51,0},
		{0,0,0,102,103,31,124,208,126,211,58,175,141,146,255,165,28,245,102,0},
		{0,0,0,204,206,62,248,189,252,187,116,67,7,57,227,87,56,247,204,0},
		{0,0,0,0,59,13,104,189,68,209,30,8,163,65,41,229,98,50,36,59},	// generator polynom
		{0,0,0,0,118,26,208,103,136,191,60,16,91,130,82,215,196,100,72,118},
		{0,0,0,0,236,52,189,206,13,99,120,32,182,25,164,179,149,200,144,236},
		{0,0,0,0,197,104,103,129,26,198,240,64,113,50,85,123,55,141,61,197},
		{0,0,0,0,151,208,206,31,52,145,253,128,226,100,170,246,110,7,122,151},
		{0,0,0,0,51,189,129,62,104,63,231,29,217,200,73,241,220,14,244,51},
		{0,0,0,0,102,103,31,124,208,126,211,58,175,141,146,255,165,28,245,102},
		{0,0,0,0,204,206,62,248,189,252,187,116,67,7,57,227,87,56,247,204}
	};
	for(i = 0; i < 32; i+= 8) {
		tmp = (wreg[0]>>i) & 0xff;
		/* galois multiplication */
		for(j = i; tmp; j++) {
			/* test every bit in shadow */
			if(tmp & 1) {
				uint32_t *xor_ptr = (uint32_t*)&xor_array[j][0];
				wreg[0] ^= xor_ptr[0];
				wreg[1] ^= xor_ptr[1];
				wreg[2] ^= xor_ptr[2];
				wreg[3] ^= xor_ptr[3];
				wreg[4] ^= xor_ptr[4];
			}
			tmp >>=1;
		}
	}
}

/* software implementation of rs encoder, optimized for 32bit */
bool DVBT_rs::encode( )
{
	int i,n;
	uint32_t wreg[5];

	uint32_t *datain;
	uint32_t *dataout;
	DVBT_memory *in = this->pin->read();
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}
	n = in->size;
	datain = (uint32_t*)in->ptr;
	dataout = (uint32_t*)out->ptr;
	do {
		// clear register on each loop
		memset(wreg, 0, sizeof(wreg));
		for( i=0; i<47; i++ ) {
			/* shift in new byte */
			wreg[4] = datain[i];
			dataout[i] = datain[i];

			/* do a galois multiplication on wreg */
			galois_mult( wreg );
			/* shift the register */
			wreg[0] = wreg[1];
			wreg[1] = wreg[2];
			wreg[2] = wreg[3];
			wreg[3] = wreg[4];
		}
		for( i=0; i<3; i++ ) {
			/* shift in zeros */
			wreg[4] = 0;

			/* do a galois multiplication on wreg */
			galois_mult( wreg );

			/* shift the register */
			wreg[0] = wreg[1];
			wreg[1] = wreg[2];
			wreg[2] = wreg[3];
			wreg[3] = wreg[4];
		}
		/* shift in zeros */
		wreg[4] = 0;

		/* do a galois multiplication on wreg */
		galois_mult( wreg );

		/* copy the result */
		memcpy(&dataout[47], &wreg[1], sizeof(wreg[0]) * 4);

		datain+=47;
		dataout+=51;
		n-=188;
	} while(n > 0);

	delete in;

	if(!this->pout->write(out)) {
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}

	return true;
}
