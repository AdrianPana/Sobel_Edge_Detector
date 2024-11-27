BLUR=true

build: build_sequential build_pthreads

build_sequential:
	g++ -o sobel_sequential sobel_sequential_video.cpp `pkg-config --cflags --libs opencv4` -lm -g

build_pthreads:
	g++ -o sobel_pthreads sobel_pthreads.cpp `pkg-config --cflags --libs opencv4` -lm -lpthread -g

run:
	./sobel_sequential $(IMAGE) $(BLUR) 

run_pthreads:
	./sobel_pthreads $(IMAGE) $(BLUR) 

profile:
	valgrind --tool=callgrind ./sobel_sequential $(IMAGE) $(BLUR) 

clean:
	rm sobel_sequential