## Darkroom

This is for video processing and export (x, y) of center point.
This will feed the data correponding to each frame, serving for predict purpose.

## Build

For each video, there are corresponding source file in `src` folder.
Run the following command and system will auto-build with correponding source.

```
make build TARGET=carpet|marble|wood_noise|wood_multi
```

With:
  + `carpet.cpp`: videos 1 + 2

  + `marble.cpp`: video 3

  + `wood_noise.cpp`: video 4

  + `wood_multi.cpp`: video 5


## Run

Ouput of build process is an exectable `center` file, then:

```
./center <video_path>
```

### Demo

![Crop](demos/crop.png)

Demo came with a frame (cropped) from wood floor & multi lane.
As you can see, it draws contours, red point for center point.

## Clean

For a fresh start:

```
make clean
```

## Development and contributing

+ Put your source code into `src` folder, and naming convention follow background of the floor.

+ Remember to input video from command line argument, and output to file through `stdout` by redirecting:

```
./center vid1.avi > Contestant.txt
```

+ Syntax of output:

```
N
frame_id x_center y_center

with,
    N: denotes number of frame
    frame_id(int): id of current frame, 0-index
    x_center(int): midpoint's x
    y_center(int): midpoint's y
```
