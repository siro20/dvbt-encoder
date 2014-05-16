ETSI EN 300 744 compliant DVBT encoder library written in c++
==============

required libraries:

* libfftwf

Supported Modes:
---------------
* Only hierarchical mode is supported
* IFFT size 8K and 2K
* coderate 1/2, 2/3, 3/4, 5/6, 7/8
* modulation QPSK, 16QAM, 64QAM
* guard interval 1/4, 1/8, 1/16, 1/32
* oversampling 1x,2x,4x,8x,16x
* output format
 * complex float
 * complex short (int16_t)
 * complex ushort (uint16_t)
 * complex char (int8_t)
 * complex uchar (uint8_t)
* set output gain [float]

Tested Modes:
------------------------
* IFFT size 2K
* coderate 1/2, 3/4, 4/5
* modulation 16QAM
* guard interval 1/4, 1/8, 1/16
* oversampling 2x
* output format float

TODO:
---------------
fix thread signals
fix 64QAM mapping

Encoding speed:
---------------
Realtime on Intel(R) Core(TM) i5-3450

features:
--------------
multiple processes for efficient multicore usage 
