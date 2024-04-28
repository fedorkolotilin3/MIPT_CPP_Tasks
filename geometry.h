#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

const double kEps = 0.0001;

using std::sqrt;
using std::vector;

double abs(double val) {
  return (val < 0 ? -val : val);
}

bool isEqual(double num1, double num2) {
  return abs((num1 - num2)) < kEps;
}

int sign(double val) {
  if (isEqual(val, 0)) {
    return 0;
  } else {
    return (val > 0 ? 1 : -1);
  }
}

struct Point {
  double x;
  double y;
  bool is_infinite = false;

  Point(double x = 0, double y = 0) : x(x), y(y) {}

  bool operator==(Point point) const {
    return (isEqual(x, point.x) && isEqual(y, point.y));
  }

  bool operator!=(Point point) const {
    return (!this->operator==(point));
  }

  Point(bool is_infinite1) : Point() {
    is_infinite = is_infinite1;
  }

  Point(const Point &point) = default;

  Point &operator-=(Point point) {
    x -= point.x;
    y -= point.y;
    return *this;
  }

  Point &operator=(Point point) {
    x = point.x;
    y = point.y;
    return *this;
  }

  Point &operator+=(Point point) {
    x += point.x;
    y += point.y;
    return *this;
  }

  double scalMult(Point point) const {
    return x * point.x + y * point.y;
  }

  double vecMult(Point point) const {
    return x * point.y - y * point.x;
  }

  Point &operator*=(double koef) {
    x *= koef;
    y *= koef;
    return *this;
  }

  Point &operator/=(double koef) {
    x /= koef;
    y /= koef;
    return *this;
  }

  Point operator+(Point rhs) const {
    Point copy(*this);
    return copy += rhs;
  }

  Point operator-(Point rhs) const {
    Point copy(*this);
    return copy -= rhs;
  }

  Point operator*(double rhs) const {
    Point copy(*this);
    return copy *= rhs;
  }

  Point operator/(double rhs) const {
    Point copy(*this);
    return copy /= rhs;
  }

  double len() const {
    double square_of_len = ((*this).scalMult(*this));
    return sqrt(square_of_len);
  }

  Point ortogonalPoint(int scal_sign) {
    Point ort_point(y * -1, x);
    return (scal_sign == 1 ? ort_point : ort_point * -1);
  }

  void rotateVec(double angle) {
    double rad_angle = acos(-1) * (angle / 180);
    double r_x(cos(rad_angle));
    double r_y(sin(rad_angle));
    double new_x = r_x * x - r_y * y;
    double new_y = r_y * x + r_x * y;
    x = new_x;
    y = new_y;
  }

  void rotate(Point center, double angle) {
    Point vec_to_center = *this - center;
    vec_to_center.rotateVec(angle);
    *this = center + vec_to_center;
  }

  void reflect(Point center) {
    rotate(center, 180);
  }

  void scale(Point center, double coefficient) {
    Point to_cent_vec = *this - center;
    to_cent_vec *= (coefficient - 1);
    *this += to_cent_vec;
  }

  Point normieredPoint() const {
    return (*this / len());
  }
};

std::istream &operator>>(std::istream &is, Point &point) {
  return (is >> point.x >> point.y);
}

std::ostream &operator<<(std::ostream &os, Point point) {
  return (os << "(" << point.x << " " << point.y << ") ");
}

struct Line {
  Point point_1;
  Point point_2;

  Line(const Point point_1, const Point point_2) : point_1(point_1), point_2(point_2) {}

  Line(Point start_p, double angle_k) : Line(start_p, Point(start_p.x + 1, start_p.y + angle_k)) {}

  Line(double angle_k, double delta_y) : Line(Point(0, delta_y), angle_k) {}

  bool contains(Point point) const {
    if (sign((point - point_1).vecMult(point_2 - point_1)) == 0) {
      return true;
    }
    return false;
  }

  bool isParallel(const Line &line) const {
    Point vec1 = point_2 - point_1;
    Point vec2 = line.point_2 - line.point_1;
    return vec1.vecMult(vec2) == 0;
  }

  bool operator==(const Line &line) const {
    return contains(line.point_1) && contains(line.point_2);
  }

  bool operator!=(const Line &line) const {
    return !(*this == line);
  }

  Point directionVector() {
    return point_2 - point_1;
  }
};

void reflect(Point &point, const Line &axis) {
  Point line_base = axis.point_1;
  Point line_vec = axis.point_2 - line_base;
  Point project = line_base + line_vec * (point - line_base).scalMult(line_vec) / line_vec.scalMult(line_vec);
  point = project * 2 - point;
}

Point intersection(Line l1, Line l2) {
  if (l1.isParallel(l2)) {
    return Point(true);
  }
  Point base1 = l1.point_1;
  Point base2 = l2.point_1;
  Point vec1 = l1.point_2 - l1.point_1;
  Point vec2 = l2.point_2 - l2.point_1;
  double _c1 = base1.vecMult(vec1);
  double _c2 = base2.vecMult(vec2);
  double _x = (_c1 * vec2.x - _c2 * vec1.x) / (vec2.vecMult(vec1));
  double _y;
  if (vec2.x == 0) {
    _y = (_x * vec1.y - _c1) / vec1.x;
  } else {
    _y = (_x * vec2.y - _c2) / vec2.x;
  }
  return {_x, _y};
}

bool inSegment(Point point, Point segment_begin, Point segment_end) {
  if (Line(segment_begin, segment_end).contains(point)) {
    if (sign((point - segment_begin).scalMult(segment_end - segment_begin)) == 1
        && sign((point - segment_end).scalMult(segment_begin - segment_end)) == 1) {
      return true;
    }
  }
  return false;
}

bool intersects(Point segment_1_begin, Point segment_1_end, Point segment_2_begin, Point segment_2_end) {
  Point intersect = (intersection(Line(segment_1_end, segment_1_begin), Line(segment_2_end, segment_2_begin)));
  if (intersect.is_infinite) {
    return false;
  }
  if (intersect == segment_2_end) {
    return false;
  }
  return (inSegment(intersect, segment_2_begin, segment_2_end)
      && inSegment(intersect, segment_1_begin, segment_1_end));
}

double dist(Point point, Line line) {
  return abs((point - line.point_1).vecMult(line.directionVector().normieredPoint()));
}

struct Shape {
  virtual double perimeter() const = 0;
  virtual double area() const = 0;
  virtual bool operator==(const Shape &another) const = 0;
  virtual bool operator!=(const Shape &another) const = 0;
  virtual bool isCongruentTo(const Shape &another) const = 0;
  virtual bool isSimilarTo(const Shape &another) const = 0;
  virtual bool containsPoint(Point point) const = 0;
  virtual void rotate(Point center, double angle) = 0;
  virtual void reflect(Point center) = 0;
  virtual void reflect(const Line &axis) = 0;
  virtual void scale(Point center, double coefficient) = 0;
  virtual ~Shape() = default;
};
struct Polygon : Shape {
  vector<Point> vertices;

  Polygon(const vector<Point> &vertices) : vertices(vertices) {}

  template <typename... T>
  Polygon(T... vertices) : vertices({vertices...}) {}

  Polygon(std::initializer_list<Point> list) {
    for (auto point : list) {
      vertices.push_back(point);
    }
  }

  int verticesCount() const {
    return vertices.size();
  }

  vector<Point> getVertices() const {
    return vertices;
  }

  bool isConvex() const {
    int size = verticesCount();
    if (size <= 3) {
      return true;
    }
    Point previous_point = vertices[size - 2];
    Point point = vertices[size - 1];
    Point next_point = vertices[0];
    int signum = sign((point - previous_point).vecMult(next_point - point));
    for (int i = 0; i < verticesCount() - 1; i++) {
      previous_point = point;
      point = next_point;
      next_point = vertices[i + 1];
      int next_sign = sign((point - previous_point).vecMult(next_point - point));
      if (signum != next_sign) {
        return false;
      }
    }
    return true;
  }

  ~Polygon() override = default;

  double perimeter() const override {
    double perimetr_counter = (vertices[0] - vertices[verticesCount() - 1]).len();
    for (int i = 0; i < verticesCount() - 1; i++) {
      perimetr_counter += (vertices[i] - vertices[i + 1]).len();
    }
    return (perimetr_counter);
  }

  double area() const override {
    double area_counter(0);
    for (int i = 1; i < verticesCount() - 1; i++) {
      area_counter += (vertices[i] - vertices[0]).vecMult(vertices[i + 1] - vertices[0]);
    }
    return (abs(area_counter));
  }

  void reverse() {
    std::reverse(vertices.begin(), vertices.end());
  }

  bool operator==(const Shape &another) const override {
    const Polygon* copy_pointer = dynamic_cast<const Polygon*>(&another);
    if (copy_pointer != nullptr) {
      Polygon copy = *copy_pointer;
      if (copy.verticesCount() != verticesCount()) {
        return false;
      }
      for (int i = 0; i < verticesCount(); i++) {
        if (copy.vertices[i] == vertices[0]) {
          for (int j = 1; j < verticesCount(); j++) {
            int ind = (j + i) % verticesCount();
            if (vertices[j] != copy.vertices[ind]) {
              break;
            }
          }
          return true;
        }
      }
      copy.reverse();
      for (int i = 0; i < verticesCount(); i++) {
        if (copy.vertices[i] == vertices[0]) {
          for (int j = 1; j < verticesCount(); j++) {
            int ind = (j + i) % verticesCount();
            if (vertices[j] != copy.vertices[ind]) {
              break;
            }
          }
          return true;
        }
      }
      return false;
    }
    return false;
  }

  bool operator!=(const Shape &another) const override {
    return !(*this == another);
  }

  bool isCongruentTo(const Shape &another) const override {
    const Polygon* copy_pointer = dynamic_cast<const Polygon*>(&another);
    if (copy_pointer != nullptr) {
      Polygon copy = *copy_pointer;
      return (isSimilarTo(another) && (isEqual(perimeter(), copy.perimeter())));
    }
    return false;
  }

  Point getVert(int ind) const {
    return vertices[(ind % verticesCount() + verticesCount()) % verticesCount()];
  }

  void SetVectors(int ind1, int ind2, const Polygon &another, vector<Point*> &vectors) const {
    Point first_vec1 = getVert(ind1 - 1) - getVert(ind1 - 2);
    Point first_vec2 = another.getVert(ind2 - 1) - another.getVert(ind2 - 2);
    Point second_vec1 = getVert(ind1) - getVert(ind1 - 1);
    Point second_vec2 = another.getVert(ind2) - another.getVert(ind2 - 1);
    *vectors[0] = first_vec1;
    *vectors[1] = first_vec2;
    *vectors[2] = second_vec1;
    *vectors[3] = second_vec2;
  }

  bool isSimilarToPolygon(const Polygon &another) const {
    if (another.verticesCount() != verticesCount()) {
      return false;
    }
    for (int i = 0; i < verticesCount(); i++) {
      bool fl = true;
      Point first_vec1;
      Point first_vec2;
      Point second_vec1;
      Point second_vec2;
      vector<Point*> vectors = {&first_vec1, &first_vec2, &second_vec1, &second_vec2};
      SetVectors(i, 0, another, vectors);
      bool loc_fl = false;
      double sim_k = first_vec1.len() / first_vec2.len();
      if (isEqual(second_vec1.len(), sim_k * (second_vec2).len())) {
        if (isEqual((first_vec1).scalMult(second_vec1), sim_k * sim_k * (first_vec2).scalMult(second_vec2))) {
          loc_fl = true;
        }
      }
      fl = fl & loc_fl;
      for (int j = 1; j < verticesCount(); j++) {
        int ind = (j + i) % verticesCount();
        SetVectors(ind, j, another, vectors);
        loc_fl = false;
        if (isEqual((first_vec1).len(), sim_k * (first_vec2).len())) {
          if (isEqual((second_vec1).len(), sim_k * (second_vec2).len())) {
            if (isEqual((first_vec1).scalMult(second_vec1),
                        sim_k * sim_k * (first_vec2).scalMult(second_vec2))) {
              loc_fl = true;
            }
          }
        }
        fl = fl & loc_fl;
      }
      if (fl) {
        return true;
      }
    }
    return false;
  }

  bool isSimilarTo(const Shape &another) const override {
    const Polygon* copy_pointer = dynamic_cast<const Polygon*>(&another);
    if (copy_pointer != nullptr) {
      Polygon copy = *copy_pointer;
      if (isSimilarToPolygon(copy)) {
        return true;
      }
      copy.reverse();
      if (isSimilarToPolygon(copy)) {
        return true;
      }
      return false;
    }
    return false;
  }

  bool containsPoint(Point point) const override {
    double maxCoord = std::max(abs(point.x), abs(point.y));
    for (int i = 0; i < verticesCount(); i++) {
      maxCoord = std::max(maxCoord, abs(vertices[i].x));
      maxCoord = std::max(maxCoord, abs(vertices[i].y));
    }
    Point point2(maxCoord * 4, maxCoord * 4);
    int intersection_counter = 0;
    for (int i = 0; i < verticesCount(); i++) {
      if (intersects(point, point2, vertices[i], vertices[(i + 1) % verticesCount()])) {
        intersection_counter += 1;
      }
    }
    return (intersection_counter % 2 == 1);
  }

  void rotate(Point center, double angle) override {
    for (auto &point : vertices) {
      point.rotate(center, angle);
    }
  }

  void reflect(Point center) override {
    for (auto &point : vertices) {
      point.reflect(center);
    }
  }

  void reflect(const Line &axis) override {
    for (auto &point : vertices) {
      ::reflect(point, axis);
    }
  }

  void scale(Point center, double coefficient) override {
    for (auto &point : vertices) {
      point.scale(center, coefficient);
    }
  }
};

struct Ellipse : Shape {
  Point f1;
  Point f2;
  double dist;

  Ellipse(Point f_1, Point f_2, const double &dist) : f1(f_1), f2(f_2), dist(dist) {}

  std::pair<Point, Point> focuses() const {
    return {f1, f2};
  }

  std::pair<Line, Line> directrices() const {
    double a_var = (dist / 2);
    double c_var = ((f2 - f1).len() / 2);
    Point x_vec = ((f2 - f1) / (f2 - f1).len());
    Point y_vec = ((f2 - f1) / (f2 - f1).len()).ortogonalPoint(1);
    Point right_d0 = x_vec * a_var * a_var / c_var;
    Point left_d0 = x_vec * (-a_var) * a_var / c_var;
    Line right_d = Line(right_d0, right_d0 + y_vec);
    Line left_d = Line(left_d0, left_d0 + y_vec);
    return {right_d, left_d};
  }

  double eccentricity() const {
    double a_var = (dist / 2);
    double c_var = ((f2 - f1).len() / 2);
    return c_var / a_var;
  }

  double perimeter() const override {
    double pi(acos(-1.0));
    double a_var = (dist / 2);
    double c_var = ((f2 - f1).len() / 2);
    double b_var = sqrt(a_var * a_var - c_var * c_var);
    double d_var = 3 * (a_var - b_var) * (a_var - b_var) / ((a_var + b_var) * (a_var + b_var));
    return pi * (a_var + b_var) * (1 + (d_var / (10 + sqrt(4 - d_var))));
  }

  double area() const override {
    double pi(acos(-1.0));
    double a_var = (dist / 2);
    double c_var = ((f2 - f1).len() / 2);
    double b_var = sqrt(a_var * a_var - c_var * c_var);
    return (pi * a_var * b_var);
  }

  bool equals(const Ellipse &another) const {
    return ((f1 == another.f1 && f2 == another.f2 && dist == another.dist) ||
        (f1 == another.f2 && f2 == another.f1 && dist == another.dist));
  };

  bool operator==(const Shape &another) const override {
    const Ellipse* copy_ref = dynamic_cast<const Ellipse*>(&another);
    if (copy_ref != nullptr) {
      Ellipse copy = *copy_ref;
      return (*this).equals(copy);
    }
    return false;
  }

  bool operator!=(const Shape &another) const override {
    return !(*this == another);
  }

  bool isCongruentTo(const Shape &another) const override {
    const Ellipse* copy_ref = dynamic_cast<const Ellipse*>(&another);
    if (copy_ref != nullptr) {
      Ellipse copy = *copy_ref;
      return ((f2 - f1).len() == (copy.f2 - copy.f1).len() && dist == copy.dist);
    }
    return false;
  }

  bool isSimilarTo(const Shape &another) const override {
    const Ellipse* copy_ref = dynamic_cast<const Ellipse*>(&another);
    if (copy_ref != nullptr) {
      Ellipse copy = *copy_ref;
      return ((f2 - f1).len() / (copy.f2 - copy.f1).len() == dist / copy.dist);
    }
    return false;
  }

  bool containsPoint(Point point) const override {
    return (point - f1).len() + (point - f2).len() <= dist;
  }

  void rotate(Point center, double angle) override {
    f1.rotate(center, angle);
    f2.rotate(center, angle);
  }

  void reflect(Point center) override {
    f1.reflect(center);
    f2.reflect(center);
  }

  void reflect(const Line &axis) override {
    ::reflect(f1, axis);
    ::reflect(f2, axis);
  }

  void scale(Point center, double coefficient) override {
    f1.scale(center, coefficient);
    f2.scale(center, coefficient);
    dist *= coefficient;
  }

  ~Ellipse() = default;
};

struct Circle : Ellipse {
  Circle(Point f_1, double radius) : Ellipse(f_1, f_1, 2 * radius) {}

  Point center() {
    return f1;
  }

  double radius() {
    return double(dist / 2);
  }
};

struct Rectangle : Polygon {
  std::pair<Point, Point> vert_find(Point vert1, Point vert2, double rel) {
    double dist1 = (vert2 - vert1).scalMult(vert2 - vert1) / (1 + rel * rel);
    double dist2 = (vert2 - vert1).scalMult(vert2 - vert1) / (1 + rel * rel) * rel * rel;
    if (dist1 > dist2) {
      std::swap(dist1, dist2);
    }
    double ab_dist = (vert2 - vert1).len() / (dist2 / dist1 + 1);
    double ab_normal_dist = ab_dist * sqrt(dist1) / sqrt(dist2);
    Point ans1 = (vert2 - vert1).ortogonalPoint(1) / (vert2 - vert1).len() * ab_normal_dist;
    return {ans1, (vert2 + vert1) * 2 - ans1};
  }

  Rectangle(Point vert1, Point vert2, double rel) : Polygon(vert1,
                                                            vert_find(vert1, vert2, rel).first,
                                                            vert2,
                                                            vert_find(vert1, vert2, rel).second) {}

  Point center() {
    return vertices[0] + vertices[2] / 2;
  }

  std::pair<Line, Line> diagonals() {
    return {Line(vertices[0], vertices[2]), Line(vertices[1], vertices[3])};
  }
};
struct Square : Rectangle {
  Square(Point vert1, Point vert2) : Rectangle(vert1, vert2, 1) {}

  Circle circumscribedCircle() {
    return Circle(center(), (vertices[2] - vertices[0]).len() / 2);
  }

  Circle inscribedCircle() {
    Point cent = center();
    double radius = (cent - vertices[0]).vecMult(vertices[2] - vertices[0]);
    radius /= (vertices[2] - vertices[0]).len();
    return Circle(cent, radius);
  }
};
struct Triangle : Polygon {
  Point _a = vertices[0];
  Point _b = vertices[1];
  Point _c = vertices[2];
  using Polygon::Polygon;

  Point centroid() { return (_a + _b + _c) / 3; };

  Circle circumscribedCircle() {
    Line middle_perp1((_a + _b) / 2, (_a + _b) / 2 + (_b - _a).ortogonalPoint(1));
    Line middle_perp2((_b + _c) / 2, (_b + _c) / 2 + (_c - _b).ortogonalPoint(1));
    Point center = intersection(middle_perp1, middle_perp2);
    double radius = (_a - center).len();
    return {center, radius};
  }

  Circle inscribedCircle() {
    Line bisectr1(_a, _a + ((_b - _a).normieredPoint() + (_c - _a).normieredPoint()));
    Line bisectr2(_b, _b + ((_c - _b).normieredPoint() + (_a - _b).normieredPoint()));
    Point center = intersection(bisectr1, bisectr2);
    double radius = dist(center, Line(_a, _b));
    return {center, radius};
  }

  Point orthocenter() {
    Line perp1(_a, _a + (_c - _b).ortogonalPoint(1));
    Line perp2(_b, _b + (_a - _c).ortogonalPoint(1));
    return intersection(perp1, perp2);
  }

  Line EulerLine() {
    return {centroid(), circumscribedCircle().f1};
  }

  Circle ninePointsCircle() {
    Circle ans_circle = circumscribedCircle();
    ans_circle.scale(orthocenter(), 0.5);
    return ans_circle;
  }
};

/*

1) Пробелы между методами и дебаг вывод, ссылки рядом с типом    //ok
6: без комментариев - мат константы                              //ok
2) Неп назв перем и мет                                          //ok
3) const Point - не имеет смысла, т.к. копируется                //ok
136: magic const                                                 //ok
4) Лучше использовать dynamic_cast через попытку приведения указателей - try catch слишком энергоёмкая штука   //ok
5) 327-336 - копипаста - вынеси в отдельные методы получения методов //ok
6) Следи за const, например contatins point должно быть const    //ok
7) Окружность девяти точек вычисляется неправильно

 */