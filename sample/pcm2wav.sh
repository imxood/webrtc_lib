#!/bin/sh
find $1 -name "*.pcm" >list

for file in `cat list`; do
    echo $file
    wav_name=`echo $file|cut -d'.' -f1`;
    echo "$wav_name"

    ffmpeg  -f s16le -v 8 -y -ar 16000 -ac 1 -i $file  "$wav_name.wav"
done

rm -f list