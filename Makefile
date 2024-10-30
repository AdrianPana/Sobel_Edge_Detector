build:
	g++ -o sobel_sequential sobel_sequential.cpp `pkg-config --cflags --libs opencv4` -lm

clean:
	rm sobel_sequential