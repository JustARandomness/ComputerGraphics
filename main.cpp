#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

uchar quantizePixel(uchar pixelValue, int n) {
    int levels = 1 << n;
    int step = 256 / levels;
    return (pixelValue / step) * step;
}

int main(int argc, char** argv) {
    Mat image = imread("../tosyapocalypsis.jpg", IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cout << "Could not open or find the image." << std::endl;
        return -1;
    }

    int num_colors = std::stoi(argv[1]);
    if (num_colors < 1 || num_colors >= 8) {
        std::cout << "num_colors must be between 1 and 7." << std::endl;
        return -1;
    }

    int n = image.rows;
    int m = image.cols;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            uchar oldPixel = image.at<uchar>(i, j);
            uchar newPixel = quantizePixel(oldPixel, num_colors);
            image.at<uchar>(i, j) = newPixel;

            int quantError = oldPixel - newPixel;

            if (j + 1 < m) {
                image.at<uchar>(i, j + 1) += quantError * 7 / 16;
            }
            if (i + 1 < n) {
                if (j > 0) {
                    image.at<uchar>(i + 1, j - 1) += quantError * 3 / 16;
                }
                image.at<uchar>(i + 1, j) += quantError * 5 / 16;
                if (j + 1 < m) {
                    image.at<uchar>(i + 1, j + 1) += quantError * 1 / 16;
                }
            }
        }
    }

    imwrite("../outputImage.jpg", image);

    return 0;
}
