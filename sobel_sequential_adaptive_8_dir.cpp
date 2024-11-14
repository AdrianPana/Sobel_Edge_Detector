#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <time.h>

int max(int conv[8]) {
    int result = -1;
    for(int i = 0; i < 8; i++) {
        if(conv[i] > result) {
            result = conv[i];
        }
    }

    return result;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: make run IMAGE=<image_path> THRESHOLD=<threshold> (default = 100)" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    std::filesystem::path fs_path(imagePath);
    std::string imageName = fs_path.filename().string();
    std::string dirPath = fs_path.parent_path().parent_path().string();

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

    int G0[5][5] = {
        {0, 0, 0, 0, 0},
        {-1, -2, -4, -2, -2},
        {0, 0, 0, 0, 0},
        {1, 2, 4, 2, 1},
        {0, 0, 0, 0, 0}
    };

    int G225[5][5] = {
        {0, 0, 0, 0, 0},
        {0, -2, -4, -2, 0},
        {-1, -4, 0, 4, 4},
        {0, 2, 4, 2, 0},
        {0, 0, 0, 0, 0}
    };

    int G45[5][5] = {
        {0, 0, 0, -1, 0},
        {0, -2, -4, 0, 1},
        {0, -4, 0, 4, 0},
        {-1, 0, 4, 2, 0},
        {0, 1, 0, 0, 0}
    };

    int G675[5][5] = {
        {0, 0, -1, 0, 0},
        {0, -2, -4, -2, 0},
        {0, -4, 0, 4, 0},
        {0, -2, 4, 2, 0},
        {0, 0, 1, 0, 0}
    };

    int G90[5][5] = {
        {0, -1, 0, 1, 0},
        {0, -2, 0, 2, 0},
        {0, -4, 0, 4, 0},
        {0, -2, 0, 2, 0},
        {0, -1, 0, 1, 0}
    };

    int G1125[5][5] = {
        {0, 0, 1, 0, 0},
        {0, -2, 4, 2, 0},
        {0, -4, 0, 4, 0},
        {0, -2, -4, 2, 0},
        {0, 0, -1, 0, 0}
    };

    int G135[5][5] = {
        {0, 1, 0, 0, 0},
        {-1, 0, 4, 2, 0},
        {0, -4, 0, 4, 0},
        {0, -2, -4, 0, 1},
        {0, 0, 0, -1, 0}
    };

    int G1575[5][5] = {
        {0, 0, 0, 0, 0},
        {0, 2, 4, 2, 0},
        {-1, -4, 0, 4, 1},
        {0, -2, -4, 2, 0},
        {0, 0, 0, 0, 0}
    };


    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            int c[8] = {0, 0, 0, 0, 0, 0, 0, 0};

            for (int k1 = -2; k1 <= 2; ++k1) {
                for (int k2 = -2; k2 <= 2; ++k2) {
                    int pixelVal = grayscaleImage.at<uchar>(i + k1, j + k2);
                    c[0] += G0[k1 + 2][k2 + 2] * pixelVal;
                    c[1] += G225[k1 + 2][k2 + 2] * pixelVal;
                    c[2] += G45[k1 + 2][k2 + 2] * pixelVal;
                    c[3] += G675[k1 + 2][k2 + 2] * pixelVal;
                    c[4] += G90[k1 + 2][k2 + 2] * pixelVal;
                    c[5] += G1125[k1 + 2][k2 + 2] * pixelVal;
                    c[6] += G135[k1 + 2][k2 + 2] * pixelVal;
                    c[7] += G1575[k1 + 2][k2 + 2] * pixelVal;
                }
            }

            result.at<uchar>(i, j) = max(c);
        }
    }
    clock_t end = clock();


    double duration = (double)(end - start) / CLOCKS_PER_SEC;
    std::string outputImagePath = dirPath + "/edges/edges_" + imageName;
    cv::imwrite(outputImagePath, result);

    std::cout << "Edges image saved as " << outputImagePath << std::endl;
    std::cout << "Algorithm took: " << duration * 1000 << std::endl;

    return 0;
}
