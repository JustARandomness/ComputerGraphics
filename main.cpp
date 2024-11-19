#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

void drawLine(Mat& img, Point p1, Point p2) {
    int x2 = p2.x, y2 = p2.y;
    int x = p1.x, y = p1.y;
    int dx = (x < x2) ? x2 - x : x - x2;
    int dy = (y < y2) ? y2 - y : y - y2;
    int ix = (x < x2) ? 1 : -1;
    int iy = (y < y2) ? 1 : -1;
    int error;
    if (dx >= dy) {
        error = 2 * dy - dx;
        if (iy >= 0) {
            for (int i = 0; i < dx; ++i) {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error >= 0) {
                    y += iy;
                    error -= 2 * dx;
                }
                x += ix;
                error += 2 * dy;
            }
        }
        else {
            for (int i = 0; i < dx; ++i) {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error > 0) {
                    y += iy;
                    error -= 2 * dx;
                }
                x += ix;
                error += 2 * dy;
            }
        }
    }
    else {
        error = 2 * dx - dy;
        if (iy >= 0) {
            for (int i = 0; i < dy; ++i) {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error >= 0) {
                    x += ix;
                    error -= 2 * dy;
                }
                y += iy;
                error += 2 * dx;
            }
        }
        else {
            for (int i = 0; i < dy; ++i) {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error > 0) {
                    x += ix;
                    error -= 2 * dy;
                }
                y += iy;
                error += 2 * dx;
            }
        }
    }
}

Point getCubicBezierCurvePoint(Point p0, Point p1, Point p2, Point p3, double t) {
    double B0 = (1 - t) * (1 - t) * (1 - t);
    double B1 = 3 * t * (1 - t) * (1 - t);
    double B2 = 3 * t * t * (1 - t);
    double B3 = t * t * t;
    int x = int(round(B0 * p0.x + B1 * p1.x + B2 * p2.x + B3 * p3.x));
    int y = int(round(B0 * p0.y + B1 * p1.y + B2 * p2.y + B3 * p3.y));
    return Point(x, y);
}

void drawCubicBezierCurve(Mat& img, Point p0, Point p1, Point p2, Point p3, int N) {
    double step = 1.0 / (N - 1);
    for (int i = 0; i < N; ++i) {
        Point p = getCubicBezierCurvePoint(p0, p1, p2, p3, i * step);
        Point q = getCubicBezierCurvePoint(p0, p1, p2, p3, (i + 1) * step);
        drawLine(img, p, q);
    }
}

int main() {
    Mat img(500, 500, CV_8UC3, Scalar(255, 255, 255));
    drawCubicBezierCurve(img, Point(0, 0), Point(200, 20), Point(200, 50), Point(0, 100), 1000);
    namedWindow("img");
    imshow("img", img);
    waitKey(0);
    return 0;
}