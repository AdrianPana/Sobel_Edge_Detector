BLUR=true
IMAGE=./videos/adi_scurtu.mp4

build: build_sequential build_pthreads build_mpi build_pthreads_better build_openmp

build_sequential:
	g++ -o sobel_sequential sobel_sequential_video.cpp `pkg-config --cflags --libs opencv4` -lm -g

build_pthreads:
	g++ -o sobel_pthreads sobel_pthreads.cpp `pkg-config --cflags --libs opencv4` -lm -lpthread -g

build_mpi:
	mpic++ -o sobel_mpi sobel_mpi.cpp `pkg-config --cflags --libs opencv4` -lm -lpthread -g
	# mpic++ -o sobel_mpi sobel_mpi_extra.cpp `pkg-config --cflags --libs opencv4` -lm -lpthread -g
  
build_pthreads_better:
	g++ -o pthreads_better sobel_pthreads_better.cpp `pkg-config --cflags --libs opencv4` -lm -lpthread -g

build_openmp:
	g++ -o sobel_openmp sobel_openmp.cpp `pkg-config --cflags --libs opencv4` -lm -g -fopenmp

run:
	./sobel_sequential $(IMAGE) $(BLUR) 

run_pthreads:pthreads_better
	./sobel_pthreads $(IMAGE) $(BLUR) 

run_mpi:
	mpirun -np 16 ./sobel_mpi $(IMAGE) $(BLUR) 

profile:
	valgrind ./sobel_mpi $(IMAGE) $(BLUR) 

clean:
	rm sobel_sequential