#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

using namespace cv;

enum CLPointType {LEFT, RIGHT, BEYOND, BEHIND, BETWEEN, ORIGIN, DESTINATION};
enum IntersectType {SAME, PARALLEL, SKEW, SKEW_CROSS, SKEW_NO_CROSS};
enum EType {TOUCHING, CROSS_LEFT, CROSS_RIGHT, INESSENTIAL};
enum PType {INSIDE, OUTSIDE};
enum fillType {EO, NZW};

CLPointType classify(Point2d p1, Point2d p2, Point2d p) {
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

EType getEdgeType(Point2d o, Point2d d, Point2d a) {
    switch(classify(o, d, a)) {
        case LEFT:
            if (a.y > o.y && a.y <= d.y) {
                return CROSS_LEFT;
            }
            else {
                return INESSENTIAL;
            }
        case RIGHT:
            if (a.y > d.y && a.y <= o.y) {
                return CROSS_RIGHT;
            }
            else {
                return INESSENTIAL;
            }
        case BETWEEN:
        case ORIGIN:
        case DESTINATION:
            return TOUCHING;
        default:
            return INESSENTIAL;
    }
}

PType PInPolygonNZWMode(Point p, const std::vector<Point2d>& points) {
    int n = points.size();
    int param = 0;
    for (int i = 0; i < n; ++i) {
        switch(getEdgeType(points[i], points[(i + 1) % n], p)) {
            case TOUCHING:
                return INSIDE;
            case CROSS_LEFT:
                param++;
                break;
            case CROSS_RIGHT:
                param--;
                break;
        }
    }
    
    if (param == 0) {
        return OUTSIDE;
    } 
    else {
        return INSIDE;
    }
}

void fillPolygon(Mat& img, std::vector<Point2d>& points, Vec3b color) {
    int xMin = std::min_element(points.begin(), points.end(), [](Point2d a, Point2d b) { return a.x < b.x; })->x;
    int xMax = std::max_element(points.begin(), points.end(), [](Point2d a, Point2d b) { return a.x < b.x; })->x;
    int yMin = std::min_element(points.begin(), points.end(), [](Point2d a, Point2d b) { return a.y < b.y; })->y;
    int yMax = std::max_element(points.begin(), points.end(), [](Point2d a, Point2d b) { return a.y < b.y; })->y;
    // std::cout << xMin << " " << xMax << " " << yMin << " " << yMax << std::endl;
    for (int i = yMin; i < yMax; ++i) {
        for (int j = xMin; j < xMax; ++j) {
            if (PInPolygonNZWMode(Point(j, i), points) == INSIDE) {
                img.at<Vec3b>(i, j) = color;
            }
        }
    }
}

std::vector<Point3d> makeRotation(std::vector<Point3d> points, Point3d normal, int angle) {
    std::vector<Point3d> newPoints;
    for (int i = 0; i < points.size(); ++i) {
        double cosphi = std::cos(double(angle) * M_PI / 180);
        double sinphi = std::sin(double(angle) * M_PI / 180);
        Point3d newPoint;
        newPoint.x = points[i].x * (normal.x * normal.x * (1 - cosphi) + cosphi) + points[i].y * (normal.x * normal.y * (1 - cosphi) - normal.z * sinphi) + points[i].z * (normal.x * normal.z * (1 - cosphi) + normal.y * sinphi);
        newPoint.y = points[i].x * (normal.x * normal.y * (1 - cosphi) + normal.z * sinphi) + points[i].y * (normal.y * normal.y * (1 - cosphi) + cosphi) + points[i].z * (normal.y * normal.z * (1 - cosphi) - normal.x * sinphi);
        newPoint.z = points[i].x * (normal.x * normal.z * (1 - cosphi) - normal.y * sinphi) + points[i].y * (normal.y * normal.z * (1 - cosphi) + normal.x * sinphi) + points[i].z * (normal.z * normal.z * (1 - cosphi) + cosphi);
        newPoints.push_back(newPoint);
    }
    return newPoints;
}

void drawLine(Mat& img, Point2d p1, Point2d p2, Point2d figureCenter) {
    Point2d imgCenter(int(round(img.cols / 2)), int(round(img.rows / 2)));
    p1 = p1 + imgCenter - figureCenter;
    p2 = p2 + imgCenter - figureCenter;
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
                img.at<Vec3b>(img.rows - y, x) = Vec3b(0, 0, 0);
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
                img.at<Vec3b>(img.rows - y, x) = Vec3b(0, 0, 0);
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
                img.at<Vec3b>(img.rows - y, x) = Vec3b(0, 0, 0);
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
                img.at<Vec3b>(img.rows - y, x) = Vec3b(0, 0, 0);
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

Point3f transformPerspective(Point3d p, double r) {
    Point3f transformedPoint(
        double(p.x) / (r * p.z + 1.0),
        double(p.y) / (r * p.z + 1.0), 
        double(p.z) / (r * p.z + 1.0)
    );
    return transformedPoint;
}

Point2d transformParallel(Point3d p) {
    return Point2d(p.x, p.y);
}

Point3d getNormal(Point3d p1, Point3d p2, Point3d p3) {
    Point3d normal;
    normal.x = (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y);
    normal.y = (p2.z - p1.z) * (p3.x - p1.x) - (p2.x - p1.x) * (p3.z - p1.z);
    normal.z = (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    return normal;
}

bool isFrontFace(Point3d normal, Point3d viewDirection) {
    double dotProduct = normal.x * viewDirection.x + normal.y * viewDirection.y + normal.z * viewDirection.z;
    return dotProduct > 0;
}

// void drawCube(Mat& img, std::vector<Point2d> points) {
//     for (int i = 0; i < 4; ++i) {
//         drawLine(img, points[i], points[(i + 1) % 4]);
//     }

//     for (int i = 4; i < 8; ++i) {
//         drawLine(img, points[i], points[(i - 3) % 4 + 4]);
//     }

//     for (int i = 0; i < 4; ++i) {
//         drawLine(img, points[i], points[i + 4]);
//     }
// }

void drawCubePerspective(Mat& img, std::vector<Point3d> transoformedPoints, Point2d center) {
    img.setTo(Scalar(255, 255, 255));
    std::vector<std::vector<int>> faces = {
        {0, 1, 2, 3}, // Нижняя грань
        {7, 6, 5, 4}, // Верхняя грань
        {4, 5, 1, 0}, // Передняя грань
        {5, 6, 2, 1}, // Правая грань
        {6, 7, 3, 2}, // Задняя грань
        {7, 4, 0, 3}  // Левая грань
    };

    Point3d viewDir(0, 0, -1);

    int cnt = 0;

    std::vector<Vec3b> colors = {Vec3b(100, 0, 0), Vec3b(0, 100, 0), Vec3b(0, 0, 100), Vec3b(100, 100, 0)};

    for (const auto& face : faces) {
        Point3d normal = getNormal(
            transoformedPoints[face[0]],
            transoformedPoints[face[1]],
            transoformedPoints[face[2]]
        );

        if (!isFrontFace(normal, viewDir)) {
            continue;
        }

        for (int i = 0; i < face.size(); ++i) {
            Point2d p1(transoformedPoints[face[i]].x, transoformedPoints[face[i]].y);
            Point2d p2(transoformedPoints[face[(i + 1) % face.size()]].x, transoformedPoints[face[(i + 1) % face.size()]].y);
            drawLine(img, p1, p2, center);
        }

        std::vector<Point2d> facePoints;
        Point2d imgCenter = Point2d(img.cols / 2, img.rows / 2);
        for (int i = 0; i < face.size(); ++i) {
            facePoints.push_back(Point2d(transoformedPoints[face[i]].x + imgCenter.x - center.x, img.rows - (transoformedPoints[face[i]].y + imgCenter.y - center.y)));
        }
        fillPolygon(img, facePoints, colors[cnt % colors.size()]);
        cnt++;
    }
}

void drawCubeParallel(Mat& img, std::vector<Point2d> newPoints, std::vector<Point3d> originalPoints, Point2d center) {
    img.setTo(Scalar(255, 255, 255));
    std::vector<std::vector<int>> faces = {
        {0, 1, 2, 3}, // Нижняя грань
        {7, 6, 5, 4}, // Верхняя грань
        {4, 5, 1, 0}, // Передняя грань
        {5, 6, 2, 1}, // Правая грань
        {6, 7, 3, 2}, // Задняя грань
        {7, 4, 0, 3}  // Левая грань
    };

    Point3d viewDir(0, 0, -1);

    int cnt = 0;

    std::vector<Vec3b> colors = {Vec3b(100, 0, 0), Vec3b(0, 100, 0), Vec3b(0, 0, 100), Vec3b(100, 100, 0)};

    for (const auto& face : faces) {
        Point3d normal = getNormal(
            originalPoints[face[0]],
            originalPoints[face[1]],
            originalPoints[face[2]]
        );

        if (!isFrontFace(normal, viewDir)) {
            continue;
        }

        for (int i = 0; i < face.size(); ++i) {
            drawLine(img, newPoints[face[i]], newPoints[face[(i + 1) % face.size()]], center);
        }

        std::vector<Point2d> facePoints;
        Point2d imgCenter = Point2d(img.cols / 2, img.rows / 2);
        for (int i = 0; i < face.size(); ++i) {
            facePoints.push_back(Point2d(newPoints[face[i]].x + imgCenter.x - center.x, img.rows - (newPoints[face[i]].y + imgCenter.y - center.y)));
        }
        fillPolygon(img, facePoints, colors[cnt % colors.size()]);
        cnt++;
    }
}

void getPerspectiveProjection(Mat& img, std::vector<Point3d> points, double k) {
    std::vector<Point3d> transformedPoints;
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        Point3d transformedPoint = transformPerspective(points[i], 1.0 / k);
        transformedPoints.push_back(transformedPoint);
    }
    Point2d center(0, 0);
    for (int i = 0; i < transformedPoints.size(); ++i) {
        center.x += transformedPoints[i].x;
        center.y += transformedPoints[i].y;
    }
    center = Point2d(int(round(center.x / transformedPoints.size())), int(round(center.y / transformedPoints.size())));
    drawCubePerspective(img, transformedPoints, center);
    // return transformedPoints;
}

void getParallelProjection(Mat& img, std::vector<Point3d> points) {
    std::vector<Point2d> newPoints;
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        Point2d newPoint = transformParallel(points[i]);
        newPoints.push_back(newPoint);
    }
    Point2d center(0, 0);
    for (int i = 0; i < newPoints.size(); ++i) {
        center.x += newPoints[i].x;
        center.y += newPoints[i].y;
    }
    center = Point2d(int(round(center.x / newPoints.size())), int(round(center.y / newPoints.size())));
    drawCubeParallel(img, newPoints, points, center);
    // return newPoints;
}

void animatePerspective(Mat& img, std::vector<Point3d> points, Point3d normal, int numFrames, int k) {
    double norm = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    normal.x = int(round(double(normal.x) / norm));
    normal.y = int(round(double(normal.y) / norm));
    normal.z = int(round(double(normal.z) / norm));
    for (int i = 0; i < numFrames; ++i) {
        getPerspectiveProjection(img, points, k);
        std::string fileName = "../Animation/frame" + std::to_string(i) + ".jpg";
        imwrite(fileName, img);
        points = makeRotation(points, normal, 6);
    }
}

void animateParallel(Mat& img, std::vector<Point3d> points, Point3d normal, int numFrames) {
    double norm = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    normal.x = int(round(double(normal.x) / norm));
    normal.y = int(round(double(normal.y) / norm));
    normal.z = int(round(double(normal.z) / norm));
    for (int i = 0; i < numFrames; ++i) {
        getParallelProjection(img, points);
        std::string fileName = "../Animation/frame" + std::to_string(i) + ".jpg";
        imwrite(fileName, img);
        points = makeRotation(points, normal, 6);
    }
}

int main() {
    Mat img(1440, 2560, CV_8UC3, Scalar(255, 255, 255));

    int n;
    std::cin >> n;

    std::vector<Point3d> points;
    for (int i = 0; i < n; ++i) {
        int x, y, z;
        std::cin >> x >> y >> z;
        points.push_back(Point3d(x, y, z));
    }
    
    Point3d normal(1, 0, 0);
    animateParallel(img, points, normal, 120);
    // animateParallel(img, points, normal, 120);

    // char projectionType;
    // std::cout << "Enter projection type (p - parallel, o - perspective): ";
    // std::cin >> projectionType;

    // if (projectionType == 'o') {
    //     double k;
    //     std::cout << "Enter center of projection(k): ";
    //     std::cin >> k;
    //     getPerspectiveProjection(img, points, k);
    // }
    // else {
    //     getParallelProjection(img, points);
    // }
    // namedWindow("img");
    // imshow("img", img);
    // imwrite("../projection.jpg", img);
    // waitKey(0);
    return 0;
}
