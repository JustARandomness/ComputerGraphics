#include <iostream>
#include "opencv4/opencv2/opencv.hpp"

using namespace cv;

int main(int argc, char** argv) {
    Mat image = imread("../tosyapocalypsis.jpg", IMREAD_COLOR);

    if (image.empty()){
        std::cout << "Could not open or find the image for first part of the lab" << std::endl;
        return -1;
    }

    Mat grayImage(image.rows, image.cols, CV_8U);

    for (int i = 0; i < image.rows; i++) {
        for (int j= 0; j < image.cols; j++) {
            Vec3b pixel = image.at<Vec3b>(i, j);
            grayImage.at<uchar>(i, j) = static_cast<uchar>(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
        }
    }
    
    Mat mask = Mat::zeros(grayImage.size(), CV_8U);
    int radius = std::min(grayImage.rows, grayImage.cols) / 2;
    Point center(grayImage.cols / 2, grayImage.rows / 2);
    for (int i = 0; i < grayImage.rows; i++) {
        for (int j = 0; j < grayImage.cols; j++) {
            if (sqrt(pow(j - center.x, 2) + pow(i - center.y, 2)) <= radius) {
                mask.at<uchar>(i, j) = 255;
            }
        }
    }

    Mat maskedImage = Mat::zeros(grayImage.size(), grayImage.type());
    for (int i = 0; i < grayImage.rows; i++) {
        for (int j = 0; j < grayImage.cols; j++) {
            maskedImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) * mask.at<uchar>(i, j) / 255;
        }
    }

    // Вторая часть

    Mat image1 = imread("../tosyapocalypsis.jpg", IMREAD_GRAYSCALE);
    Mat image2 = imread("../tosyapocalypsis2.jpg", IMREAD_GRAYSCALE);
    Mat alphaImage = imread("../tosyapocalypsis3.jpg", IMREAD_GRAYSCALE);

    if (image1.empty() || image2.empty() || alphaImage.empty()) {
        std::cout << "Could not open or find the image for second part of the lab" << std::endl;
        return -1;
    }

    Mat blendedImage = Mat::zeros(image1.size(), CV_8U);
    for (int i = 0; i < image1.rows; i++) {
        for (int j = 0; j < image1.cols; j++) {
            float alpha = alphaImage.at<uchar>(i, j) / 255.0f;
            blendedImage.at<uchar>(i, j) = static_cast<uchar>(image1.at<uchar>(i, j) * alpha + image2.at<uchar>(i, j) * (1.0 - alpha));
        }
    }

    namedWindow("Image1", WINDOW_GUI_EXPANDED);
    imshow("Image1", maskedImage);
    imwrite("../maskedImage.jpg", maskedImage);

    namedWindow("Blended Image", WINDOW_GUI_EXPANDED);
    imshow("Blended Image", blendedImage);
    imwrite("../blendedImage.jpg", blendedImage);
    
    waitKey(0);
 
    return 0;
}