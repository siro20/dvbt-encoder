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

#ifndef _DVBTENC_ED_RS_OI_H__
#define _DVBTENC_ED_RS_OI_H__

#include <iostream>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <thread>
#include "memory.hpp"
#include "settings.hpp"

#define OI_SIZE 12
#define OI_DEPTH 17

using namespace std;
using std::chrono::microseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

class DVBT_ed_rs_oi
{
public:
	DVBT_ed_rs_oi(DVBT_pipe *pin, DVBT_pipe *pout, DVBT_settings* dvbt_settings);
	~DVBT_ed_rs_oi();
	bool encode();
protected:
	uint32_t insert_oi(uint32_t val);
	void galois_mult( uint32_t wreg[5] );
	DVBT_memory *remuxer();
	uint8_t * ed_pbrs_seq;
	unsigned int edcnt;
	unsigned int oicnt;
	std::queue<uint8_t> oi_queues[OI_SIZE];
	DVBT_pipe *pin;
	DVBT_pipe *pout;
	unsigned int mReadSize;
	unsigned int mWriteSize;
	DVBT_settings* dvbt_settings;
	high_resolution_clock::time_point t0,t1;
	int64_t muxrate_cnt;
	bool mFirstTime;
	std::chrono::high_resolution_clock mClock;
	std::chrono::microseconds sleep_us_mpegts_pkt;
	unsigned char mPidCnt;
	int seccnt;
	int bitscnt;
};

#endif
