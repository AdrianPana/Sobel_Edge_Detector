#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <time.h>

#define P 8

typedef struct param
{
    cv::Mat mat;
    int id;
} param_t;

cv::Mat grayscaleImage = cv::Mat::zeros(sourceImage.size(), CV_8UC1);

void *applyGrayscale(void *args)
{
    cv::Mat mat = ((param_t *)args)->mat;
    int id = ((param_t *)args)->id;
    int start = id * (double)mat.rows / P;
    int end = std::min((id + 1) * (double)mat.rows / P, mat.rows);

    for (int i = start; i < end; ++i)
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

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: make run IMAGE=<image_path> BLUR=true" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    std::filesystem::path fs_path(imagePath);
    std::string imageName = fs_path.filename().string();
    std::string dirPath = fs_path.parent_path().parent_path().string();

    std::string blur = argv[2];
    bool BLUR = blur == "true";

    cv::Mat image = cv::imread(imagePath);

    if (image.empty())
    {
        std::cerr << "Could not open or find the image" << std::endl;
        return 1;
    }

    cv::Mat sobelImage = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::Mat result = cv::Mat::zeros(image.size(), CV_8UC1);

    clock_t start = clock();
    applyGrayscale(image);

    if (BLUR)
    {
        sobelImage = blurImage(grayscaleImage);
    }
    else
    {
        sobelImage = grayscaleImage;
    }

    result = applySobelOperator(sobelImage);

    clock_t end = clock();
    double durationSeconds = (double)(end - start) / CLOCKS_PER_SEC;

    std::string outputImagePath = dirPath + "/edges/edges_" + imageName;
    cv::imwrite(outputImagePath, result);

    std::cout << "Edges image saved as " << outputImagePath << std::endl;
    std::cout << "Algorithm took: " << durationSeconds * 1000 << "ms";
    return 0;
}
