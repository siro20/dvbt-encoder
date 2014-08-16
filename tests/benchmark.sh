#!bin/bash
make all
LD_LIBRARY_PATH=../libdvbtenc/lib
export LD_LIBRARY_PATH
echo "generating random data file..."
dd if=/dev/urandom of=/tmp/inputXYZACD bs=1048576 count=100

arguments="-o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 4 -x"
bitrate=`./dvbtenc $arguments -p`

echo "****************"
echo "testing 2K mode, bandwidth 8Mhz, coderate 1/2, guardinterval 1/32, modulation qpsk, alpha 1, outputformat float"

cat /tmp/inputXYZACD | ./dvbtenc $arguments > /dev/null
echo "realtime is $((bitrate / ( 188 * 8 )))"

arguments="-o 8192 -b 7 -c 3 -g 8 -m 4 -a 1 -s 4 -x"
bitrate=`./dvbtenc $arguments -p`

echo "****************"
echo "testing 8K mode, bandwidth 7Mhz, coderate 2/3, guardinterval 1/8, modulation 16QAM, alpha 1, outputformat float"

cat /tmp/inputXYZACD | ./dvbtenc $arguments > /dev/null
echo "realtime is $((bitrate / ( 188 * 8 )))"

arguments="-o 2048 -b 6 -c 4 -g 4 -m 6 -a 1 -s 4 -x"
bitrate=`./dvbtenc $arguments -p`

echo "****************"
echo "testing 2K mode, bandwidth 6Mhz, coderate 3/4, guardinterval 1/4, modulation 64QAM, alpha 1, outputformat float"

cat /tmp/inputXYZACD | ./dvbtenc $arguments > /dev/null
echo "realtime is $((bitrate / ( 188 * 8 )))"

rm -f /tmp/inputXYZACD
