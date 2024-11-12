THRESHOLD = 100

build:
	g++ -o sobel_sequential sobel_sequential_adaptive.cpp `pkg-config --cflags --libs opencv4` -lm

run:
	./sobel_sequential $(IMAGE) $(THRESHOLD) 

clean:
	rm sobel_sequential