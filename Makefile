BLUR=true

build:
	g++ -o sobel_sequential sobel_sequential.cpp `pkg-config --cflags --libs opencv4` -lm

run:
	./sobel_sequential $(IMAGE) $(BLUR) 

profile:
	valgrind --tool=callgrind ./sobel_sequential $(IMAGE) $(BLUR) 

clean:
	rm sobel_sequential