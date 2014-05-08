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

#include "settings.hpp"

using namespace std;

DVBT_settings::DVBT_settings(int ofdmmode, int bandwidth, int coderate, int guardinterval, int modulation, int alpha, int cellid, int oversampling, dvbt_data_formats outputformat, float gain )
{
	//constants 
	this->DVBT_MPEG_BYTES_RS_PACKET = 204;
	this->DVBT_MPEG_BYTES_TS_PACKET = 188;
	this->DVBT_SYMBOLS_FRAME = 68;
	this->DVBT_FRAMES_SUPERFRAME = 4;
	this->DVBT_II_DEPTH = 126;
	
	/* check input parameters */
	if( ofdmmode == 8192 )
	{
		this->ofdmcarriers = 6817;
		this->ofdmuseablecarriers = 6048;
		this->ofdmmode = 8192;
	}
	else if( ofdmmode == 2048 )
	{
		this->ofdmcarriers = 1705;
		this->ofdmuseablecarriers = 1512;
		this->ofdmmode = 2048;
	}
	else
	{
		fprintf( stderr, "Error: ofdmmode %d not supported!\n", ofdmmode );
		throw std::runtime_error("Error: ofdmmode not supported!\n");
	}

	if( bandwidth != 8 && bandwidth != 7 && bandwidth != 6 )
	{
		fprintf( stderr, "Error: bandwidth %d not supported!\n", bandwidth );
		throw std::runtime_error("einval");
	}
	this->bandwidth = bandwidth;

	if( coderate != 2 && coderate != 3 && coderate != 4 && coderate != 6)
	{
		fprintf( stderr, "Error: unknown coderate %d !\n", coderate );
		throw std::runtime_error("einval");
	}
	this->coderate = coderate;

	if( guardinterval != 4 &&  guardinterval != 8 && guardinterval != 16 && guardinterval != 32)
	{
		fprintf( stderr, "Error: guardinterval 1/%d not supported!\n", guardinterval );
		throw std::runtime_error("einval");
	}
	this->guardinterval = guardinterval;
	this->guardcarriers = ofdmmode / guardinterval;
	
	if( modulation != 2 && modulation != 4 && modulation != 6 )
	{
		fprintf( stderr, "Error: modulation %d not supported!\n", modulation );
		throw std::runtime_error("einval");
	}   
	this->modulation = modulation;
	
	if( alpha != 1 )
	{
		fprintf( stderr, "Error: alpha %d not supported!\n", alpha );
		throw std::runtime_error("einval");
	}
	this->alpha = alpha;

	if( !oversampling )
	{
		this->oversampling = 1;
	}
	else
	{
		if( !(oversampling == 1 ||
		oversampling == 2 ||
		oversampling == 4 ||
		oversampling == 8 ||
		oversampling == 16) )
		{
			fprintf( stderr, "Fixme: oversampling %d not supported!\n", oversampling );
			throw std::runtime_error("einval");
		}
		this->oversampling = oversampling;
	}
	this->cellid = cellid;
	
	if( !(outputformat == CHAR ||
	outputformat == UCHAR ||
	outputformat == SHORT ||
	outputformat == USHORT ||
	outputformat == FLOAT) )
	{
		fprintf( stderr, "Error: outputformat %d not supported!\n", outputformat );
		throw std::runtime_error("einval");
	}
	this->outputformat = outputformat;
	this->gain = gain;
	switch(outputformat)
	{
		case CHAR:
		case UCHAR:
			this->normalisation = 2.1f * this->gain;
			break;
		case SHORT:
		case USHORT:
			this->normalisation = 537.0f * this->gain;
			break;
		case FLOAT:
			this->normalisation = 1.0f * this->gain;
			break;
		break;
	}
	
	/* calc common settings */

	/* total bits ( including inner coder bits ) that are transmitted every superframe */
	this->bitspersuperframe = this->ofdmuseablecarriers * DVBT_SYMBOLS_FRAME * DVBT_FRAMES_SUPERFRAME * this->modulation;
	/* total bytes ( excluding inner coder bits ) */
	this->bytespersuperframe = ((this->coderate -1) * this->bitspersuperframe) / (this->coderate * 8) ;

	if( this->bytespersuperframe % DVBT_MPEG_BYTES_RS_PACKET > 0 )
	{
		fprintf( stderr, "Warn: bytespersuperframe is not multiple of %d!\n", DVBT_MPEG_BYTES_RS_PACKET );
	}

	/* mpegts packet per superframe */
	this->tspacketspersuperframe = this->bytespersuperframe / DVBT_MPEG_BYTES_RS_PACKET;
	/* total ofdm symbol per superframe */
	this->ofdmsymbolspersuperframe = (this->ofdmmode + this->ofdmmode / this->guardinterval) * DVBT_SYMBOLS_FRAME * DVBT_FRAMES_SUPERFRAME;
	/* ofdm symbolrate at I/Q transmitter */
	this->symbolrate = (float)this->bandwidth * 1000000.0f * 8.0f / 7.0f;

	/* mpegts bitrate */
	this->mpegtsbitrate = (float)this->tspacketspersuperframe * (float)DVBT_MPEG_BYTES_TS_PACKET * 8.0f * (this->symbolrate / (float)this->ofdmsymbolspersuperframe);
}
