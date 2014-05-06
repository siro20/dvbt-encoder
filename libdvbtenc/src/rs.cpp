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
 
void DVBT_rs::generate_gf()
/* generate GF(2**mm) from the irreducible polynomial p(X) in pp[0]..pp[mm]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**mm)
*/
 {
   int i, mask ;

  mask = 1 ;
  alpha_to[mm] = 0 ;
  for (i=0; i<mm; i++)
   { alpha_to[i] = mask ;
     index_of[alpha_to[i]] = i ;
     if (pp[i]!=0)
       alpha_to[mm] ^= mask ;
     mask <<= 1 ;
   }
  index_of[alpha_to[mm]] = mm ;
  mask >>= 1 ;
  for (i=mm+1; i<nn; i++)
   { if (alpha_to[i-1] >= mask)
        alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i-1]^mask)<<1) ;
     else alpha_to[i] = alpha_to[i-1]<<1 ;
     index_of[alpha_to[i]] = i ;
   }
  index_of[0] = -1 ;
 }


void DVBT_rs::gen_poly()
/* Obtain the generator polynomial of the tt-error correcting, length
  nn=(2**mm -1) Reed Solomon code  from the product of (X+alpha**i), i=1..2*tt
*/
 {
   int i,j ;

   gg[0] = 2 ;    /* primitive element alpha = 2  for GF(2**mm)  */
   gg[1] = 1 ;    /* g(x) = (X+alpha) initially */
   for (i=2; i<=nn-kk; i++)
    { gg[i] = 1 ;
      for (j=i-1; j>0; j--)
        if (gg[j] != 0)  gg[j] = gg[j-1]^ alpha_to[(index_of[gg[j]]+i)%nn] ;
        else gg[j] = gg[j-1] ;
      gg[0] = alpha_to[(index_of[gg[0]]+i)%nn] ;     /* gg[0] can never be zero */
    }
   /* convert gg[] to index form for quicker encoding */
   //cout << "gg:" << endl;
   for (i=0; i<=nn-kk; i++)
   {
        //cout << gg[i] << ", ";
        gg[i] = index_of[gg[i]];
   }
   //cout << endl;
 }

DVBT_rs::DVBT_rs(FILE *fd_in, FILE *fd_out)
{
	// primitive polynomial p(x) = x8 + x4 + x3 + x2 + 1
	this->pp[0] = 1;
	this->pp[1] = 0;
	this->pp[2] = 1;
	this->pp[3] = 1;
	this->pp[4] = 1;
	this->pp[5] = 0;
	this->pp[6] = 0;
	this->pp[7] = 0;
	this->pp[8] = 1;

	generate_gf();
	gen_poly();
	this->in_multiple_of = 188;
	this->out_multiple_of = 204;
	if(!fd_in)
		throw std::runtime_error(__FILE__" invalid in file descriptor!\n");
	if(!fd_out)
		throw std::runtime_error(__FILE__" invalid out file descriptor!\n");
	this->mem = new DVBT_memory(this->in_multiple_of,this->out_multiple_of);

	this->fd_in = fd_in;
	this->fd_out = fd_out;
}

DVBT_rs::~DVBT_rs()
{
}

// define a helper function doing the galois multiplication on 16 bytes at once
// wreg points to a 16 byte array, shadow is the byte shifted out last round
void DVBT_rs::galois_mult( unsigned int *wreg, unsigned char shadow )
{
	int j;
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
    for(j=0;j<8;j++)
    {
        /* test every bit in shadow */
            if(shadow & 1)
            {
                /* do 32 bit arithmetics */
                uint32_t *xor_ptr = (uint32_t*)&xor_array[j][0];
                wreg[0] ^= xor_ptr[0];
                wreg[1] ^= xor_ptr[1];
                wreg[2] ^= xor_ptr[2];
                wreg[3] ^= xor_ptr[3];
            }
            shadow >>=1;
    }
}

// software implementation of rs encoder
int DVBT_rs::encode( )
{
	int i,n,rret,wret;
	unsigned int wreg[4];
	unsigned char shadow;
	uint8_t *datain = this->mem->in;
	uint8_t *dataout = this->mem->out;
	n = this->mem->in_size;
	
	rret = this->mem->read(this->fd_in);

    do{
		// clear register on each loop
		memset( wreg, 0 , sizeof(int) * 4 );
		shadow = 0;
        for( i=0; i<188; i++ )
        {
            /* shift in new byte */
            wreg[3] |= (datain[i]&0xff)<<24;
            dataout[i] = datain[i];

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
        for( i=0; i<15; i++ )
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

        /* copy the result */
        memcpy(&dataout[188], wreg, sizeof(int) * 4);
        
        datain+=188;
        dataout+=204;
        n-=188;
    }while(n > 0);
    
	wret = this->mem->write(this->fd_out);

	//wret = child_write();

	if(rret || wret)
		return 1;
	return 0;
}
