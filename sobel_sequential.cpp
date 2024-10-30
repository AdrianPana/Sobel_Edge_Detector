#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

#define THRESHOLD 150

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    cv::Mat image = cv::imread(imagePath);

    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return 1;
    }

    cv::Mat grayscaleImage;
    cv::cvtColor(image, grayscaleImage, cv::COLOR_BGR2GRAY);

    cv::Mat result = cv::Mat::zeros(grayscaleImage.size(), CV_8UC1);

    int rows = grayscaleImage.rows;
    int cols = grayscaleImage.cols;

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

    std::string outputImagePath = "edges_" + imagePath;
    cv::imwrite(outputImagePath, result);

    std::cout << "Edges image saved as " << outputImagePath << std::endl;

    return 0;
}
