# specify the compiler
CC=g++
# You need to install the following packages to build this: ncurses libavformat-dev libavcodec-dev libswresample-dev libswscale-dev libavutil-dev
# specify options for the compiler
CFLAGS=-c -Wall -Ilib/include/ -std=c++17 -O3 -Dlinux

all: program

program: FfmpegDecoder.o FfmpegException.o DecodedFrame.o Asciifier.o VideoPlayer.o ConsoleUtils.o ColorConverter.o ConsoleColor.o Main.o FfmpegAudio.o MiniAudioException.o Stopwatch.o
	$(CC) FfmpegDecoder.o FfmpegException.o DecodedFrame.o FfmpegAudio.o MiniAudioException.o Asciifier.o VideoPlayer.o ConsoleUtils.o ColorConverter.o ConsoleColor.o Main.o Stopwatch.o -o cmdpla -lavcodec -lavformat -lavutil -lswscale -lswresample -lncurses

# you may have to patch frame_num to frame_number if you got a newer ffmpeg version
FfmpegDecoder.o: cmdplaypp/src/video/FfmpegDecoder.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/video/FfmpegDecoder.cpp

FfmpegAudio.o: cmdplaypp/src/audio/FfmpegAudio.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/audio/FfmpegAudio.cpp

MiniAudioException.o: cmdplaypp/src/MiniAudioException.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/MiniAudioException.cpp

FfmpegException.o: cmdplaypp/src/FfmpegException.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/FfmpegException.cpp

DecodedFrame.o: cmdplaypp/src/video/DecodedFrame.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/video/DecodedFrame.cpp

Asciifier.o: cmdplaypp/src/Asciifier.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Asciifier.cpp

VideoPlayer.o: cmdplaypp/src/VideoPlayer.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/VideoPlayer.cpp

ConsoleUtils.o: cmdplaypp/src/ConsoleUtils.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/ConsoleUtils.cpp

ColorConverter.o: cmdplaypp/src/ColorConverter.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/ColorConverter.cpp

ConsoleColor.o: cmdplaypp/src/ConsoleColor.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/ConsoleColor.cpp

Main.o: cmdplaypp/src/Main.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Main.cpp

Stopwatch.o: cmdplaypp/src/Stopwatch.cpp
	$(CC) $(CFLAGS) cmdplaypp/src/Stopwatch.cpp

clean:
	rm -rf *o program
