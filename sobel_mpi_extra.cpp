#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cmath>
#include <time.h>
#include <vector>
#include <algorithm>
#include <mpi.h>
#include <cstring> // For memcpy

#define ROOT 0
#define RATIO 4
// #define MASTERS 4

int *matToIntArray(const cv::Mat &mat, int &arraySize)
{
    arraySize = mat.total();

    int *data = new int[arraySize];

    int index = 0;
    for (int i = 0; i < mat.rows; ++i)
    {
        for (int j = 0; j < mat.cols; ++j)
        {
            data[index++] = mat.at<uchar>(i, j);
        }
    }

    return data;
}

cv::Mat intArrayToMat(const int *data, int arraySize, int rows, int cols, int type)
{
    cv::Mat mat(rows, cols, type);

    int index = 0;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            mat.at<uchar>(i, j) = data[index++];
        }
    }

    return mat;
}

cv::Mat applyGrayscale(cv::Mat sourceImage)
{
    cv::Mat grayscaleImage = cv::Mat::zeros(sourceImage.size(), CV_8UC1);

    int rows = sourceImage.rows;
    int cols = sourceImage.cols;

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            cv::Vec3b pixel = sourceImage.at<cv::Vec3b>(i, j);
            int red = pixel[0];
            int green = pixel[1];
            int blue = pixel[2];
            grayscaleImage.at<uchar>(i, j) = 0.11 * pixel[0] + 0.59 * pixel[1] + 0.3 * pixel[2];
        }
    }

    return grayscaleImage;
}

cv::Mat blurImage(cv::Mat sourceImage)
{
    cv::Mat result = cv::Mat::zeros(sourceImage.size(), CV_8UC1);

    int rows = sourceImage.rows;
    int cols = sourceImage.cols;

    int B[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
    };

    for (int i = 1; i < rows - 1; ++i)
    {
        for (int j = 1; j < cols - 1; ++j)
        {
            int c = 0;

            for (int k1 = -1; k1 <= 1; ++k1)
            {
                for (int k2 = -1; k2 <= 1; ++k2)
                {
                    int pixelVal = sourceImage.at<uchar>(i + k1, j + k2);
                    c += B[k1 + 1][k2 + 1] * pixelVal;
                }
            }

            result.at<uchar>(i, j) = c / 9;
        }
    }

    return result;
}

int getAdaptiveThreshold(int r[3][3])
{
    for (int l = 0; l < 3; ++l)
    {
        if (r[l][0] < r[l][1])
            std::swap(r[l][0], r[l][1]);
        if (r[l][0] < r[l][2])
            std::swap(r[l][0], r[l][2]);
        if (r[l][1] < r[l][2])
            std::swap(r[l][1], r[l][2]);
    }

    for (int l = 0; l < 3; ++l)
    {
        if (r[0][l] < r[1][l])
            std::swap(r[0][l], r[1][l]);
        if (r[0][l] < r[2][l])
            std::swap(r[0][l], r[2][l]);
        if (r[1][l] < r[2][l])
            std::swap(r[1][l], r[2][l]);
    }

    int m[3];
    m[0] = r[0][2];
    m[1] = r[1][1];
    m[2] = r[2][0];

    if (m[0] < m[1])
        std::swap(m[0], m[1]);
    if (m[0] < m[2])
        std::swap(m[0], m[2]);
    if (m[1] < m[2])
        std::swap(m[1], m[2]);

    return m[1];
}

cv::Mat applySobelOperator(cv::Mat sourceImage)
{
    cv::Mat result = cv::Mat::zeros(sourceImage.size(), CV_8UC1);

    int Gx[3][3] = {
        {1, 0, -1},
        {2, 0, -2},
        {1, 0, -1}};

    int Gy[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}};

    int r[3][3];
    int rows = sourceImage.rows;
    int cols = sourceImage.cols;

    for (int i = 1; i < rows - 1; ++i)
    {
        for (int j = 1; j < cols - 1; ++j)
        {
            int cx = 0;
            int cy = 0;

            for (int k1 = -1; k1 <= 1; ++k1)
            {
                for (int k2 = -1; k2 <= 1; ++k2)
                {
                    int pixelVal = sourceImage.at<uchar>(i + k1, j + k2);
                    r[k1 + 1][k2 + 1] = sourceImage.at<uchar>(i + k1, j + k2);

                    cx += Gx[k1 + 1][k2 + 1] * pixelVal;
                    cy += Gy[k1 + 1][k2 + 1] * pixelVal;
                }
            }

            int threshold = getAdaptiveThreshold(r);

            float magnitude = std::sqrt(cx * cx + cy * cy);

            result.at<uchar>(i, j) = (magnitude > threshold) ? 255 : 0;
        }
    }
    return result;
}

void processFrame(int rank, int proc, cv::Mat frame, bool BLUR, cv::VideoWriter writer)
{
    cv::Mat grayscaleImage = cv::Mat::zeros(frame.size(), CV_8UC1);
    cv::Mat sobelImage = cv::Mat::zeros(frame.size(), CV_8UC1);
    cv::Mat result = cv::Mat::zeros(frame.size(), CV_8UC1);
    int slaves_per_master = RATIO - 1;
    int masters = proc / RATIO;

    grayscaleImage = applyGrayscale(frame);

    if (BLUR)
    {
        sobelImage = blurImage(grayscaleImage);
    }
    else
    {
        sobelImage = grayscaleImage;
    }

    MPI_Status status;

    int rows_per_slave = sobelImage.rows / RATIO;
    cv::Mat r1 = sobelImage(cv::Range(0, rows_per_slave), cv::Range::all());
    cv::Mat r2;

    r1 = applySobelOperator(r1);

    int idx = 1;
    for (int slave = masters + slaves_per_master * rank; slave < masters + slaves_per_master * (rank + 1); ++slave)
    {
        int end = rows_per_slave * (idx + 1);
        if (slave == masters + slaves_per_master * (rank + 1) - 1)
        {
            end = sobelImage.rows;
        }
        cv::Mat r2 = sobelImage(cv::Range(rows_per_slave * idx, end), cv::Range::all());

        idx++;
        int r2size;
        int *r2send = matToIntArray(r2, r2size);
        MPI_Send(&r2size, 1, MPI_INT, slave, 1, MPI_COMM_WORLD);
        MPI_Send(r2send, r2size, MPI_INT, slave, 1, MPI_COMM_WORLD);
        delete[] r2send;
    }

    for (int slave = masters + slaves_per_master * rank; slave < masters + slaves_per_master * (rank + 1); ++slave)
    {

        int rows = rows_per_slave;
        if (slave == masters + slaves_per_master * (rank + 1) - 1)
        {
            rows = sobelImage.rows - (RATIO - 1) * rows_per_slave;
        }

        // std::cout << rank << " " << slave << std::endl;
        int r2size;
        MPI_Recv(&r2size, 1, MPI_INT, slave, 1, MPI_COMM_WORLD, &status);
        int *r2send = new int[r2size];
        MPI_Recv(r2send, r2size, MPI_INT, slave, 1, MPI_COMM_WORLD, &status);
        r2 = intArrayToMat(r2send, r2size, rows, sobelImage.cols, CV_8UC1);
        delete[] r2send;
        cv::vconcat(r1, r2, r1);
    }
    writer.write(r1);
}

int main(int argc, char **argv)
{
    int rank, proc;
    long start, end;

    if (argc != 3)
    {
        std::cerr << "Usage: make run_mpi IMAGE=<video_path> BLUR=<true/false>" << std::endl;
        return 1;
    }

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    int masters = proc / RATIO;
    int slaves_per_master = RATIO - 1;

    std::string videoPath = argv[1];
    std::filesystem::path fs_path(videoPath);
    std::string videoName = fs_path.filename().string();
    std::string dirPath = fs_path.parent_path().parent_path().string();
    std::string outputVideoPath = dirPath + "/edges/edges_" + std::to_string(rank) + "_" + videoName;
    std::ofstream demux;

    std::string blur = argv[2];
    bool BLUR = blur == "true";

    cv::VideoCapture vidCapture(videoPath);
    cv::VideoWriter writer;
    int frameWidth, frameHeight, frameRate, ex;
    std::vector<cv::Mat> frames;

    MPI_Status status;

    if (!vidCapture.isOpened())
    {
        std::cerr << "Error opening video stream or file" << std::endl;
        return 1;
    }

    clock_t start_t;
    if (rank == ROOT)
    {
        start_t = clock();
        demux.open(dirPath + "/edges/videos.txt");
        for (int i = 0; i < masters; i++)
        {
            std::string temp = dirPath + "/edges/edges_" + std::to_string(i) + "_" + videoName;
            int slashIndex = temp.find_last_of("/");
            demux << "file " << temp.substr(slashIndex + 1) << std::endl;
        }
    }

    if (rank < masters)
    {
        frameWidth = vidCapture.get(cv::CAP_PROP_FRAME_WIDTH);
        frameHeight = vidCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
        frameRate = vidCapture.get(cv::CAP_PROP_FPS);
        ex = static_cast<int>(vidCapture.get(cv::CAP_PROP_FOURCC));

        writer = cv::VideoWriter(outputVideoPath, ex, frameRate, cv::Size(frameWidth, frameHeight), false);

        while (true)
        {
            cv::Mat frame;

            bool frameResult = vidCapture.read(frame);
            if (!frameResult)
            {
                break;
            }

            if (frame.empty())
            {
                std::cerr << "Could not open or find the image" << std::endl;
                return 1;
            }

            frames.push_back(frame);
        }

        if (rank == ROOT)
        {
            int len_frames = frames.size();

            for (int i = 0; i < masters; ++i)
            {
                start = i * (double)len_frames / masters;
                end = std::min((i + 1) * (double)len_frames / masters, (double)len_frames);

                MPI_Send(&start, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD);
                MPI_Send(&end, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD);
            }
        }

        MPI_Recv(&start, 1, MPI_LONG_LONG, ROOT, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_LONG_LONG, ROOT, 0, MPI_COMM_WORLD, &status);

        int my_frames = end - start;
        int cols = frames[0].cols;
        int send_rows = frames[0].rows / RATIO;

        for (int slave = masters + slaves_per_master * rank; slave < masters + slaves_per_master * (rank + 1); ++slave)
        {
            MPI_Send(&my_frames, 1, MPI_INT, slave, 1, MPI_COMM_WORLD);
            MPI_Send(&cols, 1, MPI_INT, slave, 1, MPI_COMM_WORLD);
            if (slave == masters + slaves_per_master * (rank + 1) - 1)
            {
                send_rows = frames[0].rows - (RATIO - 1) * send_rows;
            }
            MPI_Send(&send_rows, 1, MPI_INT, slave, 1, MPI_COMM_WORLD);
        }

        for (long long i = start; i < end; ++i)
        {
            processFrame(rank, proc, frames[i], BLUR, writer);
        }
    }
    else
    {
        int my_frames, rows, cols;
        MPI_Recv(&my_frames, 1, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&cols, 1, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD, &status);
        for (int i = 0; i < my_frames; ++i)
        {
            int r2size;
            MPI_Status status;

            MPI_Recv(&r2size, 1, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD, &status);
            int *r2send = new int[r2size];
            MPI_Recv(r2send, r2size, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD, &status);

            cv::Mat r2 = intArrayToMat(r2send, r2size, rows, cols, CV_8UC1);
            r2 = applySobelOperator(r2);

            r2send = matToIntArray(r2, r2size);
            MPI_Send(&r2size, 1, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD);
            MPI_Send(r2send, r2size, MPI_INT, (rank - masters) / slaves_per_master, 1, MPI_COMM_WORLD);
            delete[] r2send;
        }
    }

    if (rank == ROOT)
    {
        clock_t end_t = clock();
        double durationSeconds = (double)(end_t - start_t) / CLOCKS_PER_SEC;

        std::cout << "Edges image saved as " << outputVideoPath << std::endl;
        std::cout << "Algorithm took: " << durationSeconds * 1000 << "ms";
    }

    vidCapture.release();
    writer.release();
    if (rank == ROOT)
    {
        demux.close();

        system("ffmpeg -f concat -safe 0 -i edges/videos.txt -fflags +genpts edges/merged.mp4");
    }

    MPI_Finalize();
}

// 0 1 2 3 4 5 6 7 8 9 10 11
// 0 1 2 - masters
// 0 -> 3 4 5
// 1 -> 6 7 8
// 2 -> 9 10 11
// RATIO = 4
// no_slaves * (rank + 1)
//

// 0 1 2 3 4 5 6 7
// RATIO 2
// 0 -> 4
// 1 -> 5
// 2 -> 6
// 3 -> 7