ETSI EN 300 744 compliant DVBT encoder written in c++
==============

Application:
---------------
dvbtenc

required libraries:

* libfftw3f

Arguments:
---------------

| argument | description | type | range  |
|-----|:--|------|:--|
| o   | ofdmmode       | int  | {2048, 8192} |
| b   | bandwidth      | int  | {6,7,8} |
| c   |  coderate      | int  | {2,3,4,5,7} |
| m   |  modulation    | int  | {2,4,6} |
| g   |  guardinterval | int  | {4,8,16,32} |
| a   |  alpha         | int  | 1 |
| i   |  cellid        | int  | 0-65535 |
| v   |  oversampling  | int  | {1,2,4,8,16} |
| s   |  outputformat  | int  | {0 CHAR,1 UCHAR,2 SHORT,3 USHORT,4 FLOAT} |
| f   |  gain          | float  |  |
| h   |  print help message  |   |  |
| p   |  print mpegts bitrate to stdout  |   |  |
| z   |  print samplerate to stdout  |   |  |
| l   |  number of bits for integer output | int  | {1-32} |
| j   |  disable automatic input CBR mode  |  |

> dvbtenc reads from stdin and writes to stdout !
>

Default Arguments:
---------------
If an argument isn't supplied this value is used instead:

> ofdmmode = 2048
>
> bandwidth = 8
>
> coderate = 3
>
> guardinterval = 4
>
> modulation = 4
>
> oversampling = 1
>
> alpha = 1
>
> cellid = 0
>
> gain = 1.0f
>
> outputformat = 4
>
> number of bits = full integer


Example:
---------------
cat input.mpegts | ./dvbtenc -o 2048 -b 8 -c 3 -m 2 -v 2 -g 4 -f 0.03 -s 2 -l 12 > ./output.complex

> This set OFDM mode to 2k,

> 8Mhz Channel bandwidth,

> puncturing coderate 2/3,

> QPSK carrier modulation,

> 2x oversampling,

> guardinterval set to 1/4,

> baseband gain set to 0.03,

> outputformat signed short integer (16bit),

> set outputformat bits to (+-11bit)

> Note:

> the output values are in range +-2048.

> the output values are	clamped	at -2048 and +2047.

> The output samplerate is 2x 8Mx 8/7 = 18.28MSP/s.

> By default input CBR mode is active, thus it is inserting NULL packets

> into the input MPEGTS if neccessary. It can be disabled by using -j.
