# cmdplay-pp
# What is this?
This is the next, improved version of CmdPlay (https://github.com/mariiaan/CmdPlay).
It's an improved version of the original project.

# Features
Plays videos back in command line in ascii-art.
- Colour support: play videos in colour!
- Dithering support: dithers grayscale and colours!
- Dynamic resizing: you can resize your console window while playing to obtain a higher/lower resolution!

# Screenshots
Demonstration of standard resolution; grayscale; dithering:
![Screenshot](https://marian.aomoridev.com/projects/cmdplaypp/screenshot1.png)
(Video: Bad Apple!)

Demonstration of high resolution; colours; dithering:
![Screenshot](https://marian.aomoridev.com/projects/cmdplaypp/screenshot2.png)
(Video: [Alexandros] - Dracula La)

Demonstration of true-colour; full-pixel:
![Screenshot](https://wmf.aomoridev.com/haruka1.png)
(Video: haruka-chan e.1)

# What got better (in comparison to the original project)
- now uses multi-threading for decoding videos!
- way faster
- doesn't need to precompute all frames before playing them back, does it while playing!
- colours as mentioned earlier

# Usage
Run cmdplay, enter the file path (most terminal emulators also support drag and drop) and press enter!
You can also pass the file name of the video as the first parameter, it will play the video automatically.

# Performance tips
- You should use windows terminal instead of the original windows conhost (known as cmd.exe). This will improve performance significantely due to optimizations in windows terminal.
- Colours enabled will drop performance significantely due to the terminal not being optimized for that use, so use low resolutions while colours being enabled
- Dithering in any way (colours and text dithering) will also reduce performance because it circumvents windows terminal optimizations, so again, use low resolutions

Using windows terminal with colours disabled should allow you to use pretty high resolutions!

# Controls
SPACE - play/pause
C - toggle colours
D - toggle colour dithering
T - toggle text / grayscale dithering
Q - quit
A - accurate colours (true-colour)
B - full pixel

# Libraries, this project depends on
- FFmpeg (https://www.ffmpeg.org/)
- bass.dll (https://www.un4seen.com/)

# License
This project uses the MIT license. TLDR: You can do whatever you want with the project as long as you include the license and the original copyright message. (LICENSE file)
Important: The BASS audio library (a dependency of this project) is a commercial product. While it is free for non-commercial use, please ensure to obtain a valid licence if you plan on distributing any application using it commercially.
You can obtain a commercial license at https://www.un4seen.com/license.php
The project also uses FFmpeg (https://ffmpeg.org/) which is licensed under LGPL 2.1+.

Pull-requests will be reviewed and accepted, please keep your code simple, short and understandable!
thank you for all your support
