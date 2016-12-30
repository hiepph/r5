
## Build
```
make
```

## Run
```
./center <video_path>
```

## Clean
```
make clean
```

## Demo
![Crop](demos/crop.png)


## Development and contributing

+ Root folder is for production, and `center.cpp` is the source file

+ `vid<i>` folder is for R&D for each video, `make/run/clean` action is the same.

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
