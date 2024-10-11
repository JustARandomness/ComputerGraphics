#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int getClosestLevel(int pixel, int levels) {
    int step = 256 / levels;
    return step * (pixel / step);
};

int checkIntOverflow(int value) {
    if (value < 0) {
        return 0;
    }
    if (value > 255) {
        return 255;
    }
    return value;
}

cv::Mat floydSteinberg(cv::Mat &image, int levels) {
    cv::Mat outputImage = image.clone();

    for (int i = 0; i < image.rows; ++i) {
         for (int j = 0; j < image.cols; ++j) {
            int oldPixel = image.at<uchar>(i, j);
            int newPixel = getClosestLevel(oldPixel, levels);
            outputImage.at<uchar>(i, j) = newPixel;
            int difference = oldPixel - newPixel;
            if (j + 1 < image.cols) {
                int pixel = outputImage.at<uchar>(i, j + 1);
                outputImage.at<uchar>(i, j + 1) = checkIntOverflow(pixel + 7 * difference / 16);
                if (i + 1 < image.rows) {
                    int pixel = outputImage.at<uchar>(i + 1, j + 1);
                    outputImage.at<uchar>(i + 1, j + 1) = checkIntOverflow(pixel + difference / 16);
                }
            }
            if (i + 1 < image.rows) {
                int pixel = outputImage.at<uchar>(i + 1, j);
                outputImage.at<uchar>(i + 1, j) = checkIntOverflow(pixel + 5 * difference / 16);
            }
            if (j - 1 >= 0 && i + 1 < image.rows) {
                 int pixel = outputImage.at<uchar>(i + 1, j - 1);
                outputImage.at<uchar>(i + 1, j - 1) = checkIntOverflow(pixel + 3 * difference / 16);
            }
        }
    }

    return outputImage;
}

int main(int argc, char** argv) {
    cv::Mat image = cv::imread("../tosyapocalypsis.jpg", cv::IMREAD_GRAYSCALE);

    int n;
    std::cin >> n;

    if (n < 1 || n > 8) {
        std::cout << "Invalid value" << std::endl;
        return -1;
    }

    if (n == 8) {
        imwrite("../outputImage.png", image);
        return 0;
    }

    cv::Mat outputImage = floydSteinberg(image, n);

    imwrite("../outputImage.png", outputImage);
    
    return 0;
}