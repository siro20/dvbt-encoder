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

DVBT_ed_rs_oi::DVBT_ed_rs_oi(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings)
{
	int pbrs = 0xa9;
	unsigned int out=0;
	const int multiple = 16;
	this->mReadSize = 188*multiple; // multiple of 8
	this->mWriteSize = 204*multiple; // multiple of 204
	this->ed_pbrs_seq = new uint8_t[188*8];
	this->pin = pin;
	this->pout = pout;
	
	this->dvbt_settings = dvbt_settings;
	
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
	seccnt = 0;
	bitscnt = 0;
	this->edcnt = 0;
	this->oicnt = 0;
	this->mPidCnt = 0;
	this->mFirstTime = true;
	this->muxrate_cnt = 0;
	this->sleep_us_mpegts_pkt = std::chrono::microseconds((((uint64_t)this->dvbt_settings->DVBT_MPEG_BYTES_TS_PACKET * 8ULL)<<32 / this->dvbt_settings->muxrate) -1);
}

DVBT_ed_rs_oi::~DVBT_ed_rs_oi()
{
	delete[] this->ed_pbrs_seq;
}

/*
 * this optimized version allows to do 4 galois multiplication runs in a single function
 * On every run 16 bytes are XORed at once.
 * Instead of shifting single bytes the "last" byte is read from wreg[0].
 */
void DVBT_ed_rs_oi::galois_mult( uint32_t wreg[5] )
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

DVBT_memory *DVBT_ed_rs_oi::remuxer()
{
	if(mFirstTime){
		t0 = mClock.now();
		mFirstTime = false;
	}
	
	t1 = mClock.now();
	//time since last call
	microseconds us = duration_cast<microseconds>(t1-t0);
	t0 = t1;
	this->muxrate_cnt += this->dvbt_settings->muxrate * (uint64_t)us.count();
	//std::cerr << "this->muxrate_cnt " << (this->muxrate_cnt >> 32) << endl;
	seccnt += us.count();
	if(seccnt > 1000000)
	{
		
		std::cerr << "bits/s " << (1000000ULL * (uint64_t)bitscnt / (uint64_t)seccnt) << std::endl;
		bitscnt = 0;
		seccnt -= 1000000;
	}
	
	if( this->pin->read_size() == 0 )
	{		int i;
		for(i=this->mReadSize; i > 188; i-=188)
		{
			if( (this->muxrate_cnt>>32) > i )
				break;
		}
		if( i == 188 )
		{
			// muxer is to fast, slow it down
			std::this_thread::sleep_for( this->sleep_us_mpegts_pkt );
			std::cerr << "sleeping" << endl; 
		}
		DVBT_memory *nullpkt = new DVBT_memory(i);
		for(size_t j=0; j < i;j+=188) {
			nullpkt->ptr[0+j] = 0x47; // sync byte
			nullpkt->ptr[1+j] = 0x1f; // pid NULL packet
			nullpkt->ptr[2+j] = 0xff; // pid NULL packet
			nullpkt->ptr[3+j] = 0x10; //|mPidCnt; // std. payload
			mPidCnt++;
			mPidCnt&=0x0f;
		}
		std::cerr << "*" << std::endl;
		this->muxrate_cnt -= ((int64_t)(i*8)<<32);
		bitscnt += i*8;
		return nullpkt;
	}
	else
	{
		if( (this->muxrate_cnt>>32) < (int64_t)(this->mReadSize*8) )
		{
			// muxer is to fast, slow it down
			std::this_thread::sleep_for( this->sleep_us_mpegts_pkt * 8 );
			std::cerr << "sleeping" << endl; 
		}
		// reset pid
		mPidCnt = 0;
		std::cerr << "_" << std::endl;
		// adjust muxrate counter
		this->muxrate_cnt -= ((int64_t)(this->mReadSize*8)<<32);
		bitscnt += this->mReadSize*8;
		return this->pin->read();
	}
}


uint32_t DVBT_ed_rs_oi::insert_oi(uint32_t val) {
	uint32_t ret = 0;
	/* insert new byte into buffer */
	this->oi_queues[this->oicnt].push((val>>0)&0xff);

	/* copy last byte from buffer */
	ret |= this->oi_queues[this->oicnt].front();
	this->oi_queues[this->oicnt].pop();

	if(this->oicnt == OI_SIZE - 1) {
		this->oicnt = 0;
	} else {
		this->oicnt++;
	}

	/* insert new byte into buffer */
	this->oi_queues[this->oicnt].push((val>>8)&0xff);

	/* copy last byte from buffer */
	ret |= this->oi_queues[this->oicnt].front() << 8;

	this->oi_queues[this->oicnt].pop();

	if(this->oicnt == OI_SIZE - 1) {
		this->oicnt = 0;
	} else {
		this->oicnt++;
	}


	/* insert new byte into buffer */
	this->oi_queues[this->oicnt].push((val>>16)&0xff);

	/* copy last byte from buffer */
	ret |= this->oi_queues[this->oicnt].front() << 16;

	this->oi_queues[this->oicnt].pop();

	if(this->oicnt == OI_SIZE - 1) {
		this->oicnt = 0;
	} else {
		this->oicnt++;
	}


	/* insert new byte into buffer */
	this->oi_queues[this->oicnt].push((val>>24)&0xff);

	/* copy last byte from buffer */
	ret |= this->oi_queues[this->oicnt].front() << 24;

	this->oi_queues[this->oicnt].pop();

	if(this->oicnt == OI_SIZE - 1) {
		this->oicnt = 0;
	} else {
		this->oicnt++;
	}
	return ret;
}

//encodes length packets, assuming that the MPEG TS sync byte is at offset 0
bool DVBT_ed_rs_oi::encode()
{
	uint32_t *dataout;
	uint32_t wreg[5];
	uint32_t *datain;
	uint32_t data;
	DVBT_memory *in;
	in = NULL;
	if( this->dvbt_settings->remuxer )
	{
		in = remuxer();
	}
	else
	{
		in = this->pin->read();
	}
	
	DVBT_memory *out = new DVBT_memory( this->mWriteSize );
	if( !in || !in->size || !out || !out->ptr )
	{
		this->pout->CloseWriteEnd();
		this->pin->CloseReadEnd();
		return false;
	}
	
	datain = (uint32_t*)in->ptr;
	dataout = (uint32_t*)out->ptr;

	for(size_t j=0; j < in->size; j+= 188) {
		// clear register on each loop
		memset(wreg, 0, sizeof(wreg));

		//reset pbrs counter
		if( this->edcnt >= 188*8/sizeof(uint32_t) )
			this->edcnt = 0;
		for(size_t i = 0; i < 47; i++) {
			/*  energy dispersal */
			data = datain[i] ^ ((uint32_t*)this->ed_pbrs_seq)[this->edcnt];
			this->edcnt ++;

			/* shift in new values */
			wreg[4] = data;

			/* do a galois multiplication on wreg */
			galois_mult( wreg );

			/* shift the register */
			wreg[0] = wreg[1];
			wreg[1] = wreg[2];
			wreg[2] = wreg[3];
			wreg[3] = wreg[4];
			
			/* outer interleaver */
			dataout[i] = insert_oi(data);
		}
		for(size_t i = 0; i < 3; i++ ) {
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

		/* outer interleaver */
		dataout[47] = insert_oi(wreg[1]);
		dataout[48] = insert_oi(wreg[2]);
		dataout[49] = insert_oi(wreg[3]);
		dataout[50] = insert_oi(wreg[4]);
		dataout += 51;
		datain += 47;
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
