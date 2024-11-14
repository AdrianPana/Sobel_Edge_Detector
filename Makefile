THRESHOLD = 100

build: build_sequential build_sequential_8_dir

build_sequential:
	g++ -o sobel_sequential sobel_sequential.cpp `pkg-config --cflags --libs opencv4` -lm
build_sequential_8_dir:
	g++ -o sobel_sequential_8_dir sobel_sequential_adaptive_8_dir.cpp `pkg-config --cflags --libs opencv4` -lm

run_sobel:
	./sobel_sequential $(IMAGE) $(THRESHOLD) 

run_8_dir:
	./sobel_sequential_8_dir $(IMAGE)


clean:
	rm sobel_sequential