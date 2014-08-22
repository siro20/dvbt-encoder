#!bin/bash
# script for bladeRF DVBT transmitter
# parameters

FREQUENCY=437000000

BANDWIDTH=6
GAIN=0.024
FFTSIZE=2048
CODERATE=3
MODULATION=2
OVERSAMPLING=2
GUARDINTERVAL=16
INPUTDIMENSIONX=1680
INPUTDIMENSIONY=1050
SCALEX=720
SCALEY=576
SERVICENAME="Stream#1"
SERVICEPROVIDER="HomeTV"
FONTFILE="/usr/share/fonts/dejavu/DejaVuSans.ttf"

#static content
#LD_LIBRARY_PATH=../libdvbtenc/lib
#export LD_LIBRARY_PATH
if [ ! -e "/tmp/fifo" ]; then
	mkfifo /tmp/fifo
fi

if [ ! `which bladeRF-cli 2>/dev/null` ]; then
echo "could not find bladeRF-cli"
exit
fi

dvbtencargs="-o $FFTSIZE -b $BANDWIDTH -c $CODERATE -m $MODULATION -g $GUARDINTERVAL -a 1 -i 0 -v $OVERSAMPLING -s 2"
muxrate=`./dvbtenc $dvbtencargs -p`
samplerate=`./dvbtenc $dvbtencargs -z`

if [ ! `which xrandr 2>/dev/null` ]; then
echo "could not find xrandr, please modify this script and hardcode screen resolution using INPUTDIMENSION"
exit
else
resolution=($(xrandr|grep \*|cut -d ' ' -f4))
INPUTDIMENSIONX=($(echo $resolution|cut -d 'x' -f1))
INPUTDIMENSIONY=($(echo $resolution|cut -d 'x' -f2))
fi

scalelogox=$(($INPUTDIMENSIONX/10))
scalelogoy=$(($INPUTDIMENSIONY/8))
videobitrate=$(($muxrate - 1000000 ))
echo "MPEGTS muxrate: $muxrate"
echo "DAC samplerate: $samplerate"
echo "screen resolution detected: $resolution"
echo "MPEG video cbr: $videobitrate"
rm -f /tmp/bladerfcli.tmp
echo "open" >> /tmp/bladerfcli.tmp 

# Configure readio settings
echo "set frequency $FREQUENCY" >> /tmp/bladerfcli.tmp 
echo "set samplerate $samplerate" >> /tmp/bladerfcli.tmp 
echo "set bandwidth $(($BANDWIDTH * 1000000 ))" >> /tmp/bladerfcli.tmp 

# Configure gains
echo "set txvga1 -6" >> /tmp/bladerfcli.tmp 
echo "set txvga2 25" >> /tmp/bladerfcli.tmp 
echo "set rxvga1 20" >> /tmp/bladerfcli.tmp 
echo "set rxvga2 20" >> /tmp/bladerfcli.tmp 
#txvga1 Gain setting of TXVGA1, in dB. Range: [-35, -4]
#txvga2 Gain setting of TXVGA2, in dB. Range: [0, 25]          

# Configure tx
echo "tx config file=/tmp/fifo format=bin repeat=0 delay=0 buffers=64" >> /tmp/bladerfcli.tmp
echo "tx start" >> /tmp/bladerfcli.tmp 
echo "tx wait" >> /tmp/bladerfcli.tmp 

bladeRF-cli -s /tmp/bladerfcli.tmp &

if [ `which ffmpeg 2>/dev/null` ]; then
#ffmpeg -i "$1"
ffmpeg \
-f x11grab -r 25 -s ${resolution[0]} -i :0.0 \
-filter:v "movie=logo.png,scale=$scalelogox:$scalelogoy[watermark];[in][watermark] overlay=main_w-overlay_w*1.1:overlay_h*0.1:,scale=$SCALEX:$SCALEY,drawtext=fontfile='"$FONTFILE"':text='"$SERVICEPROVIDER"':x=(main_w-text_w):y=$scalelogoy-text_h:fontsize=20:fontcolor=white[out]" \
-codec:a mp2 \
-b:a 192k \
-muxrate $muxrate \
-threads 4 \
-codec:v mpeg2video \
-mpegts_original_network_id 0x1122 \
-mpegts_transport_stream_id 0x3344 \
-mpegts_service_id 0x5566 \
-mpegts_pmt_start_pid 0x1500 \
-mpegts_start_pid 0x150 \
-metadata service_provider=$SERVICEPROVIDER \
-metadata service_name=$SERVICENAME \
-metadata language=ger \
-metadata title="dvbtenc by Patrick Rudolph 2014" \
-metadata comment="comment" \
-aspect 16:9 \
-mbd rd \
-trellis 2 \
-cmp 2 \
-subcmp 2 \
-qscale 1 \
-y \
-r 25 \
-vsync 1 \
-async 25 \
-f mpegts - 2>ffmpeg.log | \
./dvbtenc -o $FFTSIZE \
-b $BANDWIDTH \
-c $CODERATE \
-m $MODULATION \
-g $GUARDINTERVAL \
-a 1 \
-i 0 \
-v $OVERSAMPLING \
-s 2 \
-f $GAIN > /tmp/fifo
elif [ `which avconv 2>/dev/null` ]; then
avconv -r 25 -s ${resolution[0]} -i :0.0 \
-filter:v "movie=logo.png,scale=$scalelogox:$scalelogoy[watermark];[in][watermark] overlay=main_w-overlay_w:1,scale=$SCALEX:$SCALEY,drawtext=fontfile='$FONTFILE':text='hackerTV  ':x=(main_w-text_w):y=$scalelogoy+text_h*2:fontsize=20:fontcolor=white[out]" \
-codec:a mp2 \
-b:a 192k \
-muxrate $muxrate \
-threads 4 \
-codec:v mpeg2video \
-mpegts_original_network_id 0x1122 \
-mpegts_transport_stream_id 0x3344 \
-mpegts_service_id 0x5566 \
-mpegts_pmt_start_pid 0x1500 \
-mpegts_start_pid 0x150 \
-metadata service_provider="$SERVICEPROVIDER" \
-metadata service_name="$SERVICENAME" \
-aspect 16:9 \
-mbd rd \
-trellis 2 \
-cmp 2 \
-subcmp 2 \
-qscale 1 \
-y \
-r 25 \
-vsync 1 \
-async 25 \
-f mpegts - 2>avconv.log | \
./dvbtenc -o $FFTSIZE \
-b $BANDWIDTH \
-c $CODERATE \
-m $MODULATION \
-g $GUARDINTERVAL \
-a 1 \
-i 0 \
-v $OVERSAMPLING \
-s 2 \
-f $GAIN > /tmp/fifo
fi
rm -f /tmp/bladerfcli.tmp
rm -f /tmp/fifo
