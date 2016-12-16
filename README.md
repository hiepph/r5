# [WIP]

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

+ Remember to input video from command line argument, and output to file through `stdout` by Piping:
    
    ./center vid1.avi > Contestant.txt 
