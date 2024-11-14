#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <time.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: make run IMAGE=<image_path> THRESHOLD=<threshold> (default = 100)" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    std::filesystem::path fs_path(imagePath);
    std::string imageName = fs_path.filename().string();
    std::string dirPath = fs_path.parent_path().parent_path().string();

    int THRESHOLD = atoi(argv[2]);

    cv::Mat image = cv::imread(imagePath);

    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return 1;
    }
    clock_t start = clock();

    int rows = image.rows;
    int cols = image.cols;

    cv::Mat grayscaleImage = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::Mat result = cv::Mat::zeros(image.size(), CV_8UC1);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
            int red = pixel[0];
            int green = pixel[1];
            int blue = pixel[2];
            grayscaleImage.at<uchar>(i,j) = (pixel[0] + pixel[1] + pixel[2]) / 3;
        }
    }

    int Gx[3][3] = {
        {1, 0, -1},
        {2, 0, -2},
        {1, 0, -1}
    };

    int Gy[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            int cx = 0;
            int cy = 0;

            for (int k1 = -1; k1 <= 1; ++k1) {
                for (int k2 = -1; k2 <= 1; ++k2) {
                    int pixelVal = grayscaleImage.at<uchar>(i + k1, j + k2);
                    cx += Gx[k1 + 1][k2 + 1] * pixelVal;
                    cy += Gy[k1 + 1][k2 + 1] * pixelVal;
                }
            }

            float magnitude = std::sqrt(cx * cx + cy * cy);

            result.at<uchar>(i, j) = (magnitude > THRESHOLD) ? 255 : 0;
        }
    }
    clock_t end = clock();


    double duration = (double)(end - start) / CLOCKS_PER_SEC;
    std::string outputImagePath = dirPath + "/edges/edges_" + imageName;
    cv::imwrite(outputImagePath, result);

    std::cout << "Edges image saved as " << outputImagePath << std::endl;
    std::cout << "Algorithm took: " << duration << std::endl;

    return 0;
}
