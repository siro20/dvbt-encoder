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

#include "ed_rs_oi.hpp"

using namespace std;

DVBT_ed_rs_oi::DVBT_ed_rs_oi(DVBT_pipe *pin, DVBT_pipe *pout)
{
	int pbrs = 0xa9;
	unsigned int out=0;
	this->mReadSize = 188*8; // multiple of 8
	this->mWriteSize = 204*8; // multiple of 204
	this->ed_pbrs_seq = new uint8_t[188*8];
	this->pin = pin;
	this->pout = pout;
	
	this->pin->initReadEnd( this->mReadSize );
	/* gen pbrs sequence */
	for(int i=1;i<188*8;i++)
	{
		for(int j=0;j<8;j++){
			pbrs <<= 1;
			out <<=1;
			if( ((pbrs>>15)&0x01) ^ ((pbrs>>14)&0x01) )
			{
				pbrs |= 1;
				out |= 1;
			}
		}
        this->ed_pbrs_seq[i] = (uint8_t)(out&0xff);

        out = 0;
	}

	/* keep the first sync byte, xor with 0xff */
	this->ed_pbrs_seq[0] = 0xff;

	/* toggle the sync bytes, xor with 0x00 */
	for(int i=188;i<188*8;i+=188)
	{
		this->ed_pbrs_seq[i] = 0x00;
	}

	// init outer interleaver
	for(int i=0;i<OI_SIZE;i++)
	{
		for(int j=0;j<i*OI_DEPTH;j++)
		{
			this->oi_queues[i].push(0);
		}
	}
	
	this->edcnt = 0;
	this->oicnt = 0;
}

DVBT_ed_rs_oi::~DVBT_ed_rs_oi()
{
	delete[] this->ed_pbrs_seq;
}


// define a helper function doing the galois multiplication on 16 bytes at once
// wreg points to a 16 byte array, shadow is the byte shifted out last round
static inline void galois_mult( uint32_t *wreg, uint8_t shadow )
{
	// using precalculated polynom, this encodes 16 bytes at once
	// regular approach is using two lookup tables to encode a single byte
	static const uint8_t xor_array[8][16] =
	{
		{59,13,104,189,68,209,30,8,163,65,41,229,98,50,36,59},	// generator polynom
		{118,26,208,103,136,191,60,16,91,130,82,215,196,100,72,118},
		{236,52,189,206,13,99,120,32,182,25,164,179,149,200,144,236},
		{197,104,103,129,26,198,240,64,113,50,85,123,55,141,61,197},
		{151,208,206,31,52,145,253,128,226,100,170,246,110,7,122,151},
		{51,189,129,62,104,63,231,29,217,200,73,241,220,14,244,51},
		{102,103,31,124,208,126,211,58,175,141,146,255,165,28,245,102},
		{204,206,62,248,189,252,187,116,67,7,57,227,87,56,247,204}
	};
	/* galois multiplication */
	for(int j=0;j<8;j++)
	{
		/* test every bit in shadow */
			if(shadow & 1)
			{
				uint64_t *xor_ptr = (uint64_t*)&xor_array[j][0];
				((uint64_t*)wreg)[0] ^= xor_ptr[0];
				((uint64_t*)wreg)[1] ^= xor_ptr[1];
			}
			shadow >>=1;
	}
}

//encodes length packets, assuming that the MPEG TS sync byte is at offset 0
bool DVBT_ed_rs_oi::encode()
{
	int n;
	uint8_t *dataout;
	uint8_t shadow;
	uint32_t wreg[4];
	uint64_t edtmp;
	uint32_t *datain;
	uint8_t edout;
	
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
	dataout = out->ptr;
	edtmp = 0;
    do{
		// clear register on each loop
		memset( wreg, 0 , sizeof(wreg) );
		shadow = 0;
        for(int i=0; i<188; i++)
        {
			/* energy dispersal */
			if((i & 0x3) == 0){
				edtmp = *datain ^ ((uint32_t*)this->ed_pbrs_seq)[this->edcnt];
				this->edcnt ++;
				datain ++;
				if( this->edcnt >= 188*8/sizeof(uint32_t) )
					this->edcnt = 0;
			}
			/* fetch one byte */
			edout = ((edtmp>>((i&0x3)<<3))&0xff);

			/* shift in new byte */
			wreg[3] |= edout<<24;

			/* do a galois multiplication on wreg */
			galois_mult( wreg, shadow );

			/* make a backup of the last byte */
			shadow = wreg[0]&0xff;

			/* shift the register */
			wreg[0] >>= 8;
			wreg[0] |= (wreg[1]&0xff)<<24;
			wreg[1] >>= 8;
			wreg[1] |= (wreg[2]&0xff)<<24;
			wreg[2] >>= 8;
			wreg[2] |= (wreg[3]&0xff)<<24;
			wreg[3] >>= 8;
			
			/* outer interleaver */
			/* insert new byte into buffer */
			this->oi_queues[this->oicnt].push(edout);
			
			/* copy last byte from buffer */
			*dataout = this->oi_queues[this->oicnt].front(); 
			dataout++;
			this->oi_queues[this->oicnt].pop();
			this->oicnt++;
			if(this->oicnt == OI_SIZE)
				this->oicnt = 0;
		}
		for(int i=0; i<15; i++)
		{
			/* shift in zeros: wreg[3] |= (0&0xff)<<24; , is already zero ! */

			/* do a galois multiplication on wreg */
			galois_mult( wreg, shadow );
			/* make a backup of the last byte */
			shadow = wreg[0]&0xff;

			/* shift the register */
			wreg[0] >>= 8;
			wreg[0] |= (wreg[1]&0xff)<<24;
			wreg[1] >>= 8;
			wreg[1] |= (wreg[2]&0xff)<<24;
			wreg[2] >>= 8;
			wreg[2] |= (wreg[3]&0xff)<<24;
			wreg[3] >>= 8;
		}

		/* do a galois multiplication on wreg */
		galois_mult( wreg, shadow );

		/* outer interleaver */
		for(int i=0; i<16; i++){
			/* insert new byte into buffer */
			this->oi_queues[this->oicnt].push(((uint8_t*)wreg)[i]);
			
			/* copy last byte from buffer */
			*dataout = this->oi_queues[this->oicnt].front(); 
			dataout++;
			this->oi_queues[this->oicnt].pop();
			this->oicnt++;
			if(this->oicnt == OI_SIZE)
				this->oicnt = 0;
		}
		n-=188;
	}while(n > 0);
	
	delete in;
	
	if(!this->pout->write(out))
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}
	return true;
}
