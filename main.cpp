#include <iostream>
#include "opencv4/opencv2/opencv.hpp"

using namespace cv;

int main(int argc, char** argv) {
    Mat image = imread(argv[1], IMREAD_COLOR);

    if (image.empty()){
        std::cout << "Could not open or find the image for first part of the lab" << std::endl;
        return -1;
    }

    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);

    Mat mask = Mat::zeros(grayImage.size(), CV_8U);
    int radius = std::min(grayImage.rows, grayImage.cols) / 2;

    Point center(grayImage.cols / 2, grayImage.rows / 2);

    circle(mask, center, radius, Scalar(255), -1);

    Mat maskedImage;
    grayImage.copyTo(maskedImage, mask);

    namedWindow("Masked Image", WINDOW_GUI_EXPANDED);
    imshow("Masked Image", maskedImage);

    //Вторая часть лабораторной работы

    Mat image1 = imread("../tosyapocalypsis.jpg", IMREAD_GRAYSCALE);
    Mat image2 = imread("../tosyapocalypsis2.jpg", IMREAD_GRAYSCALE);
    Mat alphaImage = imread("../tosyapocalypsis3.jpg", IMREAD_GRAYSCALE);

    if (image1.empty() || image2.empty() || alphaImage.empty()){
        std::cout << "Could not open or find the image for second part of the lab" << std::endl;
        return -1;
    }

    if (image1.size()!= image2.size() || image1.size()!= alphaImage.size()){
        std::cout << "Images must have the same size" << std::endl;
        return -1;
    }

    Mat alphaImageNormalized;
    alphaImage.convertTo(alphaImageNormalized, CV_32FC1, 1.0 / 255);

    Mat image1Float, image2Float;
    image1.convertTo(image1Float, CV_32FC1);
    image2.convertTo(image2Float, CV_32FC1);

    Mat blendedImageFloat = alphaImageNormalized.mul(image1Float) + (1 - alphaImageNormalized).mul(image2Float);

    Mat blendedImage;
    blendedImageFloat.convertTo(blendedImage, CV_8UC1);

    namedWindow("Image1", WINDOW_GUI_EXPANDED);
    imshow("Image1", image1);

    namedWindow("Image2", WINDOW_GUI_EXPANDED);
    imshow("Image2", image2);

    namedWindow("Alpha Image", WINDOW_GUI_EXPANDED);
    imshow("Alpha Image", alphaImage);

    namedWindow("Blended Image", WINDOW_GUI_EXPANDED);
    imshow("Blended Image", blendedImage);
    
    waitKey(0);
 
    return 0;
}