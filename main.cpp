#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

Point transform(Point3d p, double phi, double theta) {
    return Point(int(round(p.x * cos(phi) + p.z * sin(phi))), int(round(p.x * sin(phi) * sin(theta) + p.y * cos(theta) - p.z * sin(theta) * cos(phi))));
}

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

void getProjection(Mat& img, std::vector<Point3d> points, std::vector<std::vector<int>> hasEdge) {
    std::vector<Point> new_points;
    for (int i = 0; i < points.size(); ++i) {
        new_points.push_back(transform(points[i], M_PI / 2, M_PI / 4));
    }
    int n = new_points.size();
    for (int i = 0; i < int(points.size() / 2); ++i) {
        drawLine(img, new_points[i], new_points[(i + 1) % n]);
        drawLine(img, new_points[i], new_points[i + 4]);
        drawLine(img, new_points[i + 4], new_points[(i + 5) % 4 + 1]);
    }
}

int main() {
    Mat img(500, 500, CV_8UC3, Scalar(255, 255, 255));
    int n;
    std::cin >> n;
    std::vector<std::vector<int>> hasEdge(n, std::vector<int>(n));
    std::vector<Point3d> points;
    for (int i = 0; i < n; ++i) {
        int x, y, z;
        std::cin >> x >> y >> z;
        points.push_back(Point3d(x, y, z));
    }
    getProjection(img, points, hasEdge);
    namedWindow("img");
    imshow("img", img);
    waitKey(0);
    return 0;
}