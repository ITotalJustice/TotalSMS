# TotalSMS

Simple Sega Master System emulator!

## Showcase

|                          |                          |
:-------------------------:|:-------------------------:
![Img](res/Screenshot_2021-06-22_20.44.24.png) | ![Img](res/Screenshot_2021-06-22_20.45.17.png)
![Img](res/Screenshot_2021-06-22_20.45.41.png) | ![Img](res/Screenshot_2021-06-22_20.46.31.png)
![Img](res/Screenshot_2021-06-22_20.47.21.png) | ![Img](res/Screenshot_2021-06-22_20.51.02.png)

## Building

to build a simple SDL2 based example, you will need

- SDL2 (any version)

- cmake (3.13.4 or higher)

```sh
git clone https://github.com/ITotalJustice/TotalSMS.git

cd TotalSMS

cmake -S . -B build -DSMS_EXAMPLE_SDL=ON -DCMAKE_BUILD_TYPE=Release

cmake --build build -j 2
```
this will build the binary in `build/examples/example_sdl`

so to load a rom, you can do `build/examples/example_sdl path/to/rom.sms`

## Reporting bugs

if you find any issues, please open an issue.

if it is a graphical bug, if possible, include a screenshot or video.
