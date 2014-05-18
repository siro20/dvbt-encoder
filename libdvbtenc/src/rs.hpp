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

/* this program is based uppon the work of
 Simon Rockliff, University of Adelaide   21/9/89
   26/6/91 Slight modifications to remove a compiler dependent bug which hadn't
           previously surfaced. A few extra comments added for clarity.
           Appears to all work fine, ready for posting to net!

                  Notice
                 --------
   This program may be freely modified and/or given to whoever wants it.
   A condition of such distribution is that the author's contribution be
   acknowledged by his name being left in the comments heading the program,
   however no responsibility is accepted for any financial or other loss which
   may result from some unforseen errors or malfunctioning of the program
   during use.
                                 Simon Rockliff, 26th June 1991

*/

#ifndef _DVBTENC_RS_H__
#define _DVBTENC_RS_H__

#include <iostream>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include "memory.hpp"

using namespace std;

#define mm  8           /* RS code over GF(2**m) - change to suit */
#define nn  255         /* nn=2**mm -1   length of codeword */
#define tt  8           /* number of errors that can be corrected */
#define kk  (nn-2*tt)   /* kk = nn-2*tt  */

class DVBT_rs
{
public:
    DVBT_rs(FILE *fd_in, FILE *fd_out);
    ~DVBT_rs();
    bool encode();
private:
    void galois_mult( uint32_t *wreg, uint8_t shadow );
    void gen_poly();
	void generate_gf();
	
protected:
	int in_multiple_of;
	int out_multiple_of;
	DVBT_memory *mem;
	int pp [mm+1]; // primitive polynomial p(x) = x8 + x4 + x3 + x2 + 1
	int alpha_to [nn+1], index_of [nn+1], gg [nn-kk+1] ;
	int recd [nn], data [kk], bb [nn-kk] ;
	FILE *fd_in;
	FILE *fd_out;
};

#endif
