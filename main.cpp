#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

enum CLPointType {LEFT, RIGHT, BEYOND, BEHIND, BETWEEN, ORIGIN, DESTINATION};
enum IntersectType {SAME, PARALLEL, SKEW, SKEW_CROSS, SKEW_NO_CROSS};

struct Edge {
    std::vector<Point2f> points;
    std::vector<int> intersectsWith;
};

struct Intersection {
    Point2f p;
    std::vector<int> intersectionOf;
};

CLPointType classify(Point2f p1, Point2f p2, Point2f p) {
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

IntersectType intersect(Point2f a, Point2f b, Point2f c, Point2f d, double* t) {
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

IntersectType cross(Point2f a, Point2f b, Point2f c, Point2f d, double* tab, double* tcd) {
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

std::vector<Edge> getEdges(std::vector<Point2f> points) {
    std::vector<Edge> edges;
    for (int i = 0; i < points.size(); ++i) {
        Edge edge;
        edge.points.push_back(points[i]);
        edge.intersectsWith.push_back(-1);
        edge.points.push_back(points[(i + 1) % points.size()]);
        edge.intersectsWith.push_back(-1);
        edges.push_back(edge);
    }
    return edges;
}

std::vector<Intersection> getIntersections(std::vector<Point2f> points) {
    int n = points.size();
    std::vector<Intersection> intersections;
    for (int i = 0; i < points.size(); ++i) {
        for (int j = i + 1; j < points.size(); ++j) {
            if (j == (i + 1) % n || (j + 1) % n == i) {
                continue;
            }
            double tab, tcd;
            IntersectType type = cross(points[i], points[(i + 1) % n], points[j], points[(j + 1) % n], &tab, &tcd);
            if (type == SKEW_CROSS) {
                Intersection intersection;
                Point2f inter(points[i] + tab * (points[(i + 1) % n] - points[i]));
                intersection.p = inter;
                intersection.intersectionOf.push_back(i);
                intersection.intersectionOf.push_back(j);
                intersections.push_back(intersection);
            }
        }
    }
    return intersections;
}

std::vector<Intersection> getAllIntersections(std::vector<Intersection> intersections) {
    std::vector<Intersection> allIntersections(intersections.begin(), intersections.end());
    int n = intersections.size();
    for (int i = 0; i < n; ++i) {
        Intersection newIntersection;
        std::vector<int> intersectionOf(intersections[i].intersectionOf.rbegin(), intersections[i].intersectionOf.rend());
        Point2f inter = intersections[i].p;
        newIntersection.p = inter;
        newIntersection.intersectionOf = intersectionOf;
        allIntersections.push_back(newIntersection);
    }
    return allIntersections;
}

void addIntersectionsToEdges(std::vector<Edge>& edges, std::vector<Intersection> intersections) {
    for (int i = 0; i < edges.size(); ++i) {
        for (int k = 0; k < intersections.size(); ++k) {
            if (intersections[k].intersectionOf[0] == i) {
                for (int j = 0; j < edges[i].points.size(); ++j) {
                    if (intersections[k].p.x <= std::max(edges[i].points[j].x, edges[i].points[(j + 1) % edges[i].points.size()].x) &&intersections[k].p.x >= std::min(edges[i].points[j].x, edges[i].points[(j + 1) % edges[i].points.size()].x) 
                    && intersections[k].p.y <= std::max(edges[i].points[j].y, edges[i].points[(j + 1) % edges[i].points.size()].y) && intersections[k].p.y >= std::min(edges[i].points[j].y, edges[i].points[(j + 1) % edges[i].points.size()].y)) {
                        edges[i].points.insert(edges[i].points.begin() + j + 1, intersections[k].p);
                        edges[i].intersectsWith.insert(edges[i].intersectsWith.begin() + j + 1, intersections[k].intersectionOf[1]);
                        break;
                    }
                }
            }
        }
    }
}

std::vector<Point2f> getContour(std::vector<Point2f> points) {
    std::vector<Point2f> result;
    std::vector<Edge> edges = getEdges(points);
    std::vector<Intersection> intersections = getAllIntersections(getIntersections(points));
    addIntersectionsToEdges(edges, intersections);
    Point2f point = edges[0].points[0];
    Point2f intersectionPoint;
    int i = 0, j = 1;
    do {
        if (point == edges[i].points[edges[i].points.size() - 1]) {
            i++;
            j = 0;
        }
        result.push_back(point);
        point = edges[i].points[j];
        while (edges[i].intersectsWith[j] == -1) {
            j++;
            result.push_back(point);
            point = edges[i].points[j];
        }
        result.push_back(point);
        intersectionPoint = edges[i].points[j];
        int intersectingEdge = edges[i].intersectsWith[j];
        j = 0;
        while (edges[intersectingEdge].points[j] != intersectionPoint) {
            j++;
        }
        j++;
        point = edges[intersectingEdge].points[j];
        i = intersectingEdge;
    } while (point != points[0]);
    return result;
}


void drawLine(Mat& img, Point2f p1, Point2f p2) {
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

void drawPolygon(Mat& img, std::vector<Point2f>& points) {
    for (int i = 0; i < points.size(); ++i) {
        drawLine(img, points[i], points[(i + 1) % points.size()]);
    }
}



int main() {
    Mat img(1000, 1000, CV_8UC3, Scalar(255, 255, 255));
    std::vector<Point2f> points = {
        {256, 144},
        {426, 748},
        {700, 130},
        {52 ,524},        
        {814, 558},
    };

    std::vector<Intersection> intersections = getAllIntersections(getIntersections(points));

    for (int i = 0; i < intersections.size(); ++i) {
        std::cout << intersections[i].p << " ";
        for (int j = 0; j < intersections[i].intersectionOf.size(); ++j) {
            std::cout << intersections[i].intersectionOf[j] << " ";
        }
        std::cout << std::endl;
    }
    std::vector<Edge> edges = getEdges(points);
    addIntersectionsToEdges(edges, intersections);
    for (int i = 0; i < edges.size(); ++i) {
        std::cout << "Edge " << i << ": [";
        for (int j = 0; j < edges[i].points.size(); ++j) {
            std::cout << edges[i].points[j] << " ";
        }
        std::cout << "]" << std::endl << "Intersects with edge: [";
        for (int j = 0; j < edges[i].intersectsWith.size(); ++j) {
            std::cout << " " << edges[i].intersectsWith[j];
        }
        std::cout << "]" << std::endl;
    }
    std::vector<Point2f> contour = getContour(points);
    drawPolygon(img, points);
    imwrite("../polygon.jpg", img);
    img.setTo(Scalar(255, 255, 255));
    drawPolygon(img, contour);
    imwrite("../contour.jpg", img);
    return 0;
}