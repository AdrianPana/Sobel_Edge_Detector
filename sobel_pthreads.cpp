#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <time.h>
#include <vector>
#include <pthread.h>
#include <fstream>

#define P 8

std::vector<cv::Mat> frames;
cv::VideoWriter writers[P];
bool BLUR;
std::ofstream demux;
std::string outputPaths[P];

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

void *processFrame(void *args)
{
    int tid = *((int *)args);
    int start = tid * (double)frames.size() / P;
    int end = std::min((tid + 1) * (double)frames.size() / P, (double)frames.size());

    for (int i = start; i < end; i++)
    {
        cv::Mat frame = frames.at(i);
        cv::Mat grayscaleImage = cv::Mat::zeros(frame.size(), CV_8UC1);
        cv::Mat sobelImage = cv::Mat::zeros(frame.size(), CV_8UC1);
        cv::Mat result = cv::Mat::zeros(frame.size(), CV_8UC1);

        grayscaleImage = applyGrayscale(frame);

        if (BLUR)
        {
            sobelImage = blurImage(grayscaleImage);
        }
        else
        {
            sobelImage = grayscaleImage;
        }

        result = applySobelOperator(sobelImage);

        frames.at(i) = result;
        writers[tid].write(result);
    }

    return NULL;
}

#include <time.h>
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: make run IMAGE=<video_path> BLUR=<true/false>" << std::endl;
        return 1;
    }

    std::string videoPath = argv[1];
    std::filesystem::path fs_path(videoPath);
    std::string videoName = fs_path.filename().string();
    std::string dirPath = fs_path.parent_path().parent_path().string();
    std::string outputVideoPath = dirPath + "/edges/edges_" + videoName;
    demux.open("./edges/videos.txt");

    std::string blur = argv[2];
    BLUR = blur == "true";

    cv::VideoCapture vidCapture(videoPath);

    if (!vidCapture.isOpened())
    {
        std::cerr << "Error opening video stream or file" << std::endl;
        return 1;
    }

    int frameWidth = vidCapture.get(cv::CAP_PROP_FRAME_WIDTH);
    int frameHeight = vidCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
    int frameRate = vidCapture.get(cv::CAP_PROP_FPS);
    int ex = static_cast<int>(vidCapture.get(cv::CAP_PROP_FOURCC));
    for (int i = 0; i < P; i++)
    {
        int pointIndex = outputVideoPath.find_last_of(".");
        std::string outputVideoPathName = outputVideoPath.substr(0, pointIndex);
        std::string outputVideoPathExtension = outputVideoPath.substr(pointIndex);
        outputPaths[i] = outputVideoPathName + std::to_string(i) + outputVideoPathExtension;
        std::cout << outputPaths[i] << std::endl;

        writers[i] = cv::VideoWriter(outputVideoPathName + std::to_string(i) + outputVideoPathExtension, ex, frameRate, cv::Size(frameWidth, frameHeight), false);
        int slashIndex = outputPaths[i].find_last_of("/");
        demux << "file " << outputPaths[i].substr(slashIndex + 1) << std::endl;
    }

    clock_t start = clock();
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

    pthread_t threads[P];
    int args[P];
    for (int i = 0; i < P; i++)
    {
        args[i] = i;
        int res = pthread_create(&threads[i], NULL, processFrame, &args[i]);
        if (res)
        {
            std::cerr << "Error while creating thread with id: " << std::to_string(i) << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < P; i++)
    {
        int res = pthread_join(threads[i], NULL);
        if (res)
        {
            std::cerr << "Error while joining thread with id: " << std::to_string(i) << std::endl;
            return 1;
        }
    }
    clock_t end = clock();
    double durationSeconds = (double)(end - start) / CLOCKS_PER_SEC;

    std::cout << "Edges image saved as " << outputVideoPath << std::endl;
    std::cout << "Algorithm took: " << durationSeconds * 1000 << "ms";

    vidCapture.release();
    for (int i = 0; i < P; i++)
    {
        writers[i].release();
    }

    system("ffmpeg -f concat -safe 0 -i edges/videos.txt -fflags +genpts edges/merged.mp4");
    std::cout << "Edges image saved as " << outputVideoPath << std::endl;

    demux.close();
}