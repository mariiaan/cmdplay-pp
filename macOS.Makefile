# specify the compiler
CC=g++

# specify options for the compiler
CFLAGS=-c -Wall -Ilib/include/ -std=c++17 -O3 -Dmac

# for compilation, you need to get the current ffmpeg libraries (for example from https://github.com/ColorsWind/FFmpeg-macOS), then update the include files and drag the dylib files to the static/mac directory. You need to do the same for the bass audio library which can be downloaded at https://www.un4seen.com/
# you may need to change the names of the dylib in this makefile at the end of line 13.

# in order to run it, see the readme.txt file included in the macos-release
all: program

program: FfmpegDecoder.o FfmpegException.o DecodedFrame.o Asciifier.o VideoPlayer.o ConsoleUtils.o AudioEngine.o AudioException.o AudioSource.o ColorConverter.o Instance.o ConsoleColor.o Main.o Stopwatch.o
	$(CC) FfmpegDecoder.o FfmpegException.o DecodedFrame.o Asciifier.o VideoPlayer.o ConsoleUtils.o AudioEngine.o AudioException.o AudioSource.o ColorConverter.o Instance.o ConsoleColor.o Main.o Stopwatch.o -o cmdplay -Llib/static/mac/ -lbass -lavcodec.59 -lavformat.59 -lavutil.57 -lswscale.6

# you may have to patch frame_num to frame_number if you got a newer ffmpeg version
FfmpegDecoder.o: cmdplaypp/src/video/FfmpegDecoder.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/video/FfmpegDecoder.cpp

FfmpegException.o: cmdplaypp/src/video/FfmpegException.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/video/FfmpegException.cpp

DecodedFrame.o: cmdplaypp/src/video/DecodedFrame.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/video/DecodedFrame.cpp

Asciifier.o: cmdplaypp/src/Asciifier.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Asciifier.cpp

VideoPlayer.o: cmdplaypp/src/VideoPlayer.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/VideoPlayer.cpp

ConsoleUtils.o: cmdplaypp/src/ConsoleUtils.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/ConsoleUtils.cpp

AudioEngine.o: cmdplaypp/src/audio/AudioEngine.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/audio/AudioEngine.cpp

AudioException.o: cmdplaypp/src/audio/AudioException.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/audio/AudioException.cpp

AudioSource.o: cmdplaypp/src/audio/AudioSource.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/audio/AudioSource.cpp

ColorConverter.o: cmdplaypp/src/ColorConverter.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/ColorConverter.cpp

Instance.o: cmdplaypp/src/Instance.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Instance.cpp

ConsoleColor.o: cmdplaypp/src/ConsoleColor.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/ConsoleColor.cpp

Main.o: cmdplaypp/src/Main.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Main.cpp

Stopwatch.o: cmdplaypp/src/Stopwatch.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Stopwatch.cpp

clean:
	rm -rf *o program
