# Sobel Edge Detector

## APP & SM Project

**Codrut Ciulacu 344C1**

**Pana Adrian 344C1**

# Requirements

- #### C++ 17

- #### OpenCV
```
sudo apt-get install libopencv-dev
```
# Usage
Add images to the [images](./images/) folder
```
make build
make run IMAGE=./images/image.jpg THRESHOLD=100
```

# Description

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

# References

1. https://medium.com/@erhan_arslan/exploring-edge-detection-in-python-2-sobel-edge-detector-a-closer-look-de051a7b56df
2. https://homepages.inf.ed.ac.uk/rbf/HIPR2/sobel.htm
3. https://automaticaddison.com/how-the-sobel-operator-works/