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

#include <cstdlib>
#include "dvbtenc.hpp"
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void _proc_ed(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_ed dvbted(fd_in,fd_out);

	while(!dvbted.encode())
	{};

	exit(0);
}

void _proc_rs(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_rs dvbtrs(fd_in,fd_out);

	while(!dvbtrs.encode())
	{};

	exit(0);
}

void _proc_oi(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_oi dvbtoi(fd_in,fd_out);

	while(!dvbtoi.encode())
	{};

	exit(0);
}

void _proc_ce(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_ce dvbtoi(fd_in,fd_out,dvbtsettings);

	while(!dvbtoi.encode())
	{};

	exit(0);
}

void _proc_ii(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_ii dvbtii(fd_in,fd_out,dvbtsettings);

	while(!dvbtii.encode())
	{};

	exit(0);
}

void _proc_si(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	int symbol;
	DVBT_si dvbtsi(fd_in,fd_out,dvbtsettings);
	symbol = 0;
	while(!dvbtsi.encode(symbol))
	{
		symbol ++;
		if(symbol == 68)
			symbol = 0;
	};

	exit(0);
}

void _proc_sm(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_sm dvbtsm(fd_in,fd_out,dvbtsettings);
	while(!dvbtsm.encode())
	{
	};

	exit(0);
}

void _proc_chan(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	unsigned int symbol;
	unsigned int frame;
	symbol = 0;
	frame = 0;
	DVBT_chan dvbtchan(fd_in,fd_out,dvbtsettings);
	while(!dvbtchan.encode(frame, symbol))
	{
		symbol++;
		if(symbol == 68)
		{
			symbol = 0;
			frame++;
			frame %= 4;
		}
	};

	exit(0);
}

void _proc_ifft(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_ifft dvbtifft(fd_in,fd_out,dvbtsettings);
	while(!dvbtifft.encode())
	{
	};

	exit(0);
}

void _proc_quant(FILE* fd_in, FILE* fd_out,DVBT_settings *dvbtsettings)
{
	DVBT_quant dvbtquant(fd_in,fd_out,dvbtsettings);
	while(!dvbtquant.encode())
	{
	};

	exit(0);
}

typedef void(*funcs)(FILE*,FILE*,DVBT_settings*);
funcs FunctionPointers[] = {_proc_ed,_proc_rs,_proc_oi,_proc_ce,_proc_ii,_proc_si,_proc_sm,_proc_chan,_proc_ifft,_proc_quant};

/* test function */
DVBT_enc::DVBT_enc(FILE* in, FILE* out, DVBT_settings *dvbtsettings)
{
	int pipe_fd[2];
	int i;
	pid_t pid;
	int fd;
	
	this->out = out;
	this->dvbt_settings = dvbtsettings;

	pipe(pipe_fd);
	dup2(fileno(stdin), pipe_fd[1]);
	close(fileno(stdin));
	fd = pipe_fd[0];
	
	for(i=0;i<sizeof(FunctionPointers)/sizeof(funcs)-1;i++)
	{
		pipe(pipe_fd);
		
		pid = fork();
		if(!pid) // child
		{
			FILE *c_out,*c_in;
			close(pipe_fd[0]);//close the r end

			c_out = fdopen(pipe_fd[1], "w"); //always write to pipe_fd[1]
			//fprintf(stderr,"errno %d\n",errno);
			if(!c_out)
				throw std::runtime_error(__FILE__" C failed to open write end for child!\n");
			c_in = fdopen(fd, "r");
			if(!c_in)
				throw std::runtime_error(__FILE__" C failed to open read end for child!\n");

			FunctionPointers[i](c_in, c_out,this->dvbt_settings);
			return;
		}
		else if(pid > 0) //parent
		{
			close(pipe_fd[1]);
			fd = pipe_fd[0];
		}
		else
			break;
	}
	this->next_fd = fd;
}

DVBT_enc::~DVBT_enc()
{
	
}

void DVBT_enc::encode()
{
	FunctionPointers[sizeof(FunctionPointers)/sizeof(funcs)-1](fdopen(this->next_fd, "r"), fdopen(fileno(stdout), "w"),this->dvbt_settings);
}
