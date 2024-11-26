#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

enum CLPointType {LEFT, RIGHT, BEYOND, BEHIND, BETWEEN, ORIGIN, DESTINATION};
enum IntersectType {SAME, PARALLEL, SKEW, SKEW_CROSS, SKEW_NO_CROSS};

CLPointType classify(Point p1, Point p2, Point p) {
    double ax = p2.x - p1.x;
    double ay = p2.y - p1.y;
    double bx = p.x - p1.x;
    double by = p.y - p1.y;
    double s = ax * by - bx * ay;
    
    if (s > 0) return LEFT;
    if (s < 0) return RIGHT;
    if ((ax * bx < 0) || (ay * by < 0)) return BEHIND;
    if ((ax * ax + ay * ay) < (bx * bx + by * by)) return BEYOND;
    if (p1.x == p.x && p1.y == p.y) return ORIGIN;
    if (p2.x == p.x && p2.y == p.y) return DESTINATION;
    return BETWEEN;
}

IntersectType intersect(Point a, Point b, Point c, Point d, double* t) {
    double nx = d.y - c.y;
    double ny = c.x - d.x;
    CLPointType type;
    double denom = nx * (b.x - a.x) + ny * (b.y - a.y);
    if (denom == 0) {
        type = classify(c, d, a);
        if (type == LEFT || type == RIGHT) {
            return PARALLEL;
        }
        else {
            return SAME;
        }
    }
    double num = nx * (a.x - c.x) + ny * (a.y - c.y);
    *t = -num/denom;
    return SKEW;
}

IntersectType cross(Point a, Point b, Point c, Point d, double* tab, double* tcd) {
    IntersectType type = intersect(a, b, c, d, tab);
    if (type == SAME || type == PARALLEL) {
        return type;
    }
    if ((*tab < 0) || (*tab > 1)) {
        return SKEW_NO_CROSS;
    }
    intersect(c, d, a, b, tcd);
    if ((*tcd < 0) || (*tcd > 1)) {
        return SKEW_NO_CROSS;
    }
     return SKEW_CROSS;
}

bool isComplex(const std::vector<Point>& points) {
    int n = points.size();
    if (n < 3) return true;
    for (int i = 0; i < n; ++i) {
        Point a = points[i];
        Point b = points[(i + 1) % n];

        for (int j = i + 2; j < n; ++j) {
            if (j == (i + n - 1) % n) {
                continue;
            }

            Point c = points[j];
            Point d = points[(j + 1) % n];


            double tab, tcd;
            IntersectType type = cross(a, b, c, d, &tab, &tcd);
            if (type == SKEW_CROSS) {
                return true;
            }
        }
    }
    return false;
}

std::vector<Point> findContour(std::vector<Point> points) {
    if (!isComplex(points)) {
        return points;
    }
    int n = points.size();
    std::vector<Point> result(points);
    CLPointType initialSide = RIGHT;
    bool flag = false;
    for (int i = 0; i < n; ++i) {
        Point p1 = result[i];
        Point p2 = result[(i + 1) % n];
        for (int j = 0; j < n; ++j) {
            if (j == i || j == (i + 1) % n) continue;
            CLPointType type = classify(p1, p2, result[j]);
            if (type != initialSide) {
                flag = true;
                break;
            }
        }
        if (flag) {
            for (int j = i + 2; j < n; ++j) {
                Point p2 = result[j];
                bool new_flag = true;
                for (int k = 0; k < n; ++k) {
                    if (k == i || k == j) continue;
                    Point p = result[k];
                    CLPointType check = classify(p1, p2, p);
                    if (check != initialSide) {
                        new_flag = false;
                    }
                }
                if (new_flag) {
                    std::iter_swap(result.begin() + (i + 1) % n, result.begin() + j);
                    break;
                }
            }        
        }
    }
    return result;
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

void drawPolygon(Mat& img, std::vector<Point>& points) {
    for (int i = 0; i < points.size(); ++i) {
        drawLine(img, points[i], points[(i + 1) % points.size()]);
    }
}

int main() {
    Mat img(1000, 1000, CV_8UC3, Scalar(255, 255, 255));
    std::vector<Point> points;
    int n;
    std::cin >> n;
    for (int i = 0; i < n; ++i) {
        int x, y;
        std::cin >> x >> y;
        points.push_back(Point(x, y));
    }
    namedWindow("img1");
    namedWindow("img2");
    drawPolygon(img, points);
    imshow("img1", img);
    std::vector<Point> contour = findContour(points);
    drawPolygon(img, contour);
    imshow("img2", img);
    waitKey(0);
}