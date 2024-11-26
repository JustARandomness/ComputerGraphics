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

Mat kMeans(Mat& img, int k) {
    Mat data;
    img.convertTo(data, CV_32FC3);
    data = data.reshape(1, img.rows * img.cols);
    
    std::vector<Vec3f> centers(k);
    std::vector<int> labels(data.rows);

    srand(time(0));
    for (int i = 0; i < k; ++i) {
        centers[i] = data.at<Vec3f>(rand() % data.rows, 0);
    }

    bool hasChanged = true;
    int iterations = 0;

    while (hasChanged && iterations < 100) {
        hasChanged = false;
        for (int i = 0; i < data.rows; ++i) {
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

        for (int i = 0; i < data.rows; ++i) {
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

    Mat result(img.size(), img.type());
    for (int i = 0; i < data.rows; ++i) {
        int cluster = labels[i];
        Vec3f color = centers[cluster];
        result.at<Vec3b>(i / img.cols, i % img.cols) = Vec3b(
            static_cast<uchar>(std::min(255.0f, std::max(0.0f, color[0]))),
            static_cast<uchar>(std::min(255.0f, std::max(0.0f, color[1]))),
            static_cast<uchar>(std::min(255.0f, std::max(0.0f, color[2])))
        );
    }
    return result;
}

int main() {
    Mat img = imread("../tosyapocalypsis.jpg", IMREAD_COLOR);

    int k;
    std::cin >> k;

    imwrite("../k-means.jpg", kMeans(img, k));
    return 0;
}
