#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

using namespace cv;

void drawLine(Mat &img, Point p1, Point p2) {
    int x = p1.x, y = p1.y;
    int dx = (x < p2.x) ? p2.x - x : x - p2.x;
    int dy = (y < p2.y) ? p2.y - y : y - p2.y;
    int ix = (x < p2.x) ? 1 : -1;
    int iy = (y < p2.y) ? 1 : -1;
    int error;
    if (dx >= dy)
    {
        error = 2 * dy - dx;
        if (iy >= 0)
        {
            for (int i = 0; i < dx; ++i)
            {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error >= 0)
                {
                    y += iy;
                    error -= 2 * dx;
                }
                x += ix;
                error += 2 * dy;
            }
        }
        else
        {
            for (int i = 0; i < dx; ++i)
            {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error > 0)
                {
                    y += iy;
                    error -= 2 * dx;
                }
                x += ix;
                error += 2 * dy;
            }
        }
    }
    else
    {
        error = 2 * dx - dy;
        if (iy >= 0)
        {
            for (int i = 0; i < dy; ++i)
            {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error >= 0)
                {
                    x += ix;
                    error -= 2 * dy;
                }
                y += iy;
                error += 2 * dx;
            }
        }
        else
        {
            for (int i = 0; i < dy; ++i)
            {
                img.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                if (error > 0)
                {
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

void drawCubicBezierCurve(Mat &img, Point p0, Point p1, Point p2, Point p3, int N) {
    double step = 1.0 / (N - 1);
    for (int i = 0; i < N; ++i) {
        Point p = getCubicBezierCurvePoint(p0, p1, p2, p3, i * step);
        Point q = getCubicBezierCurvePoint(p0, p1, p2, p3, (i + 1) * step);
        drawLine(img, p, q);
    }
}

Point findIntersection(Point O, double R, Point A, Point B) {
    double a1 = A.x - O.x;
    double b1 = A.y - O.y;
    double c1 = R * R;

    double a2 = B.x - O.x;
    double b2 = B.y - O.y;
    double c2 = R * R;

    double d1 = c1 + O.x * a1 + O.y * b1;
    double d2 = c2 + O.x * a2 + O.y * b2;

    double det = a1 * b2 - a2 * b1;
    if (std::abs(det) < 1e-9) {
        std::cout << "Касательные параллельны или совпадают!" << std::endl;
        return Point(0, 0);
    }

    double x = (d1 * b2 - d2 * b1) / det;
    double y = (a1 * d2 - a2 * d1) / det;

    std::cout << x << " " << y;

    return Point(int(x), int(y));
}

void drawCirclePart(Mat& img, Point center, double radius, double alpha, double beta) {
    double quarterLen = M_PI * radius / 2;
    std::cout << alpha << " " << beta << " ";
    if (alpha > 180) alpha = alpha - 360;
    if (beta > 180) beta = beta - 360;
    if (alpha > beta) {
        double tmp = alpha;
        alpha = beta;
        beta = tmp;
    }
    double len = 2 * M_PI * radius * (beta - alpha) / 360;
    alpha = alpha * M_PI / 180;
    beta = beta * M_PI / 180;
    int numParts = int(std::ceil(len / quarterLen));
    double angleStep = (beta - alpha) / numParts;
    if (alpha == beta) {
        angleStep = M_PI / 2;
        numParts = 4;
    }

    for (int i = 0; i < numParts; ++i) {
        Point p0 = Point(center.x + int(std::round(radius * std::cos(alpha  + i * angleStep))), 
                         center.y + int(std::round(radius * std::sin(alpha  + i * angleStep))));
        if (i == 0) {
            drawLine(img, center, p0);
        }
        Point p3 = Point(center.x + int(std::round(radius * std::cos(alpha  + (i + 1) * angleStep))), 
                         center.y + int(std::round(radius * std::sin(alpha + (i + 1) * angleStep))));
        if (i == numParts - 1) {
            drawLine(img, center, p3);
        }
        Point Pt = findIntersection(center, radius, p0, p3);
        double d = norm(Pt - p0);
        double F = 4 / (3 + 3 * std::sqrt(1 + (d / radius) * (d / radius)));
        Point p1 = p0 + F * (Pt - p0);
        Point p2 = p3 + F * (Pt - p3);
        drawCubicBezierCurve(img, p0, p1, p2, p3, 1000);
    }
}

int main() {
    Mat img(1000, 1000, CV_8UC3, Scalar(255, 255, 255));
    drawCirclePart(img, Point(500, 500), 200.0, 60.0, 200.0);
    imwrite("../circle.png", img);
    return 0;
}