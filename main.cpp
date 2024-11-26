#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace cv;

double getDist(Vec3f a, Vec3f b) {
    return std::sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]));
}

int main() {
    Mat img = imread("../tosyapocalypsis.jpg", IMREAD_COLOR);
    // std::cout << img.channels();
    // std::cout << img.size() << " " << img.type();
    Mat data;
    img.convertTo(data, CV_32F);
    data = data.reshape(img.rows * img.cols);

    int k;
    std::cin >> k;
    std::cout << data.size();
    std::vector<Vec3f> centers(k, 0);
    std::vector<int> labels(data.cols);

    for (int i = 0; i < k; ++i) {
        centers[i] = data.at<Vec3f>(rand() % data.cols, 0);
    }

    bool hasChanged = true;
    int iterations = 0;

    srand(time(0));


    while (hasChanged && iterations < 100) {
        for (int i = 0; i < data.cols; ++i) {
            float minDist = FLT_MAX;
            int bestCluster = 0;
            for (int j = 0; j < k; ++j) {
                float dist = getDist(data.at<Vec3f>(i, 0), centers[j]);
                if (dist < minDist) {
                    minDist = dist;
                    bestCluster = j;
                }
            }
            if (labels[i] != bestCluster) {
                labels[i] = bestCluster;
                hasChanged = true;
            }
        }

        std::vector<Vec3f> newCenters(k, Vec3f(0, 0, 0));
        std::vector<int> counts(k, 0);

        for (int i = 0; i < data.cols; ++i) {
            int cluster = labels[i];
            newCenters[cluster] += data.at<Vec3f>(i, 0);
            counts[cluster]++;
        }

        for (int j = 0; j < k; ++j) {
            if (counts[j] > 0) {
                centers[j] = newCenters[j] / counts[j];
            }
        }

        iterations++;
    }

    Mat img2(img.size(), img.type());
    for (int i = 0; i < data.cols; ++i) {
        int cluster = labels[i];
        Vec3f color = centers[cluster];
        Vec3b finalColor(
            static_cast<uchar>(std::min(255.0f, std::max(0.0f, color[0]))),
            static_cast<uchar>(std::min(255.0f, std::max(0.0f, color[1]))),
            static_cast<uchar>(std::min(255.0f, std::max(0.0f, color[2])))
        );
        // std::cout << i / img.cols << " " <<  i % img.cols <<  " " << float(i) / img.cols <<std::endl;
        img2.at<Vec3b>(i / img.cols, i % img.cols) = finalColor;
    }

    namedWindow("initial image", WINDOW_GUI_EXPANDED);
    imshow("initial image", img);

    namedWindow("img", WINDOW_GUI_EXPANDED);
    imshow("img", img2);
    waitKey(0);
    return 0;
}