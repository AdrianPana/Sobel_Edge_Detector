# Sobel Edge Detector

## APP & SM Project

**Codrut Ciulacu 344C1**

**Pana Adrian 344C1**

# Requirements

- #### C++ 17

- #### OpenCV

- #### Valgrind, Kcachegrind
```
sudo apt-get install libopencv-dev
sudo apt-get install valgrind
sudo apt-get install kcachegrind
```
# Usage
Add images to the [images](./images/) folder
```
make build
make run IMAGE=./images/image.jpg BLUR=true/false
make profile IMAGE=./images/image.jpg BLUR=true/false
```
Find the result in the [edges](./edges/) folder

For profiling, use 
```
kcachegrind callgrind.out
```
# Description

This is a project originated from a university assignment meant to show the optimization of 
an advanced Sobel Edge Detection sequential algorithm through the use of different parallelization
techniques.

The [images](./images/) folder contains a few examples to play with, the results of the edge detection
being stored in the [edges](./edges/) folder.

The [BIG](./images/big.jpg) image is used for stress-testing, as it has very large dimensions.

# Roadmap

## Week 0
Implemented the [sequential version](./sobel_sequential.cpp) of the algorithm as a starting point.
We used OpenCV for the image extraction and (at first) for the grayscale
conversion. 

## Week 0.5
Modified the [sequential version](./sobel_sequential.cpp) to an explicit grayscale conversion.
Modified [Makefile](./Makefile) and source file, they now take the image path and threshold as
parameters.
Also added [input](./images/) and [output](./edges/) folders, with a few examples.

## Week 1

### Final sequential version updates
Fixed grayscale conversion to weighted average with weights 0.11, 0.59, 0.3 for R,G,B, as stated in [5].

We added a simple strategy to select an adaptive threshold for each 3x3 pixel window in the image inspired by [4].

We also added the option to blur[6] the image, by applying a blur kernel to the grayscale version, to get rid of anomalies.

## Week 2
We enhanced the algorithm to take as an input a video and output the video where we detected the edges for each frame 
and streamed it to the result.

## Week 3
We started with the intuitive pthread implementation and profiling. Our approach
was to run in parallel on as many threads as the machine has CPUs the main work
of processing one frame. Basically we tried to process a part of the video in 
parallel and then bring all the parts toghether.

This way we manage to have around 2.41 speedup.

# Profiling
## Sequential Version

![graphic](./docs/profiling_sequential_graphic.png)

The initial result took over 24s for a 6s long video. As you can see from the 
screenshot above only 16% of the CPUs were used by this version(1.325 logical CPUs 
out of 8 cores available). Under the hood the OpenCV already does some work on
multiple threads(reading and writing the video file in general).

We identified some hotspots for our application as we described in the table below.

| No.| Function name         | Usage |
|----|-----------------------|-------|
| 1  | processFrame          | 73.6% |
| 2  | applySobelOperator    | 45%   |
| 4  | getAdaptiveThresshold | 17%   |
| 3  | blurImage             | 15.6% |
| 4  | Mat.at                | 12.3% |
| 5  | applyGrayscale        | 11.2% |       

We will be focusing our attention on the main processing function and the function
that applies the Sobel operator as the are the ones that take the most out of
our computation workload.

As we can further identify any speedup we will try to run in parallel the other functions too.

## Pthread Version
### Initial approach
![graphic1](./docs/profiling_pthreads_graphic1.png)

# References

1. https://medium.com/@erhan_arslan/exploring-edge-detection-in-python-2-sobel-edge-detector-a-closer-look-de051a7b56df
2. https://homepages.inf.ed.ac.uk/rbf/HIPR2/sobel.htm
3. https://automaticaddison.com/how-the-sobel-operator-works/
4. https://iopscience.iop.org/article/10.1088/1742-6596/1678/1/012105/pdf
5. https://gist.github.com/SubhiH/b34e74ffe4fd1aab046bcf62b7f12408
6. https://www.youtube.com/watch?v=VL8PuOPjVjY&t=1s
