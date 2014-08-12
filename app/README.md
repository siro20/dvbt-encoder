ETSI EN 300 744 compliant DVBT encoder written in c++
==============

Application:
---------------
dvbtenc

required libraries:

* libfftw3f

Arguments:
---------------
* o		ofdmmode (int) {2048, 8192} 
* b		bandwidth (int) {6,7,8}
* c		coderate (int) {2,3,4,5,7}
* m		modulation (int) {2,4,6}
* g		guard interval (int) {4,8,16,32}
* a		alpha (int) 1
* i		cellid (int) (0-65535)
* v		oversampling (int) {1,2,4,8,16}
* s		outputformat (int) {0 CHAR,1 UCHAR,2 SHORT,3 USHORT,4 FLOAT}
* f		gain (float)
* h		print help message
* p		print mpegts bitrate to stdout
* z		print samplerate to stdout

dvbtenc reads from stdin and writes to stdout !


Default Arguments:
---------------
If an argument isn't supplied these value is used instead:

ofdmmode = 2048
bandwidth = 8
coderate = 3
guardinterval = 4
modulation = 4
oversampling = 1
alpha = 1
cellid = 0
gain = 1.0f
outputformat = 4


Example:
---------------
cat input.mpegts | ./dvbtenc -o 2048 -b 8 -c 3 -m 2 -o 2 -g 1 -s 4 > ./output.complex
