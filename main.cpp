#include <SFML/Graphics.hpp>
#include <ctime>
#include <random>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#define PI 3.14159265358979323846
#define Square(x) ((x)*(x)) //제곱 함수
using namespace std;
using namespace sf;
static float G = 0.5;//중력(0.1~9)
static const int num = 80; //생성할 공의 갯수
static float friction = 1.2; //마찰력은 1보다 커야한다. 1이면 완전 탄성 충돌
static int diameter = 50;
void simplehit();
class Point : private pair<float,float> {
private:
	float _length = -1;
public:

	Point() {
	}
	Point(float _x, float _y){
		this->first = _x;
		this->second = _y;
	}
	void set(bool x_flip, float x_friction, bool y_flip, float y_friction) {
		if (x_flip) first = -first / x_friction;
		if (!x_flip) first = first / x_friction;
		if (y_flip) second = -second / y_friction;
		if (!y_flip) second = second / y_friction;
		if (Square(first) < (0.29) * G * G) first = 0;
		if (Square(second) < (0.29) * G * G) second = 0;
	}
	float x(void) {
		return first;
	}
	float y(void) {
		return second;
	}
	void x(float _x) {
		first = _x;
		_length = -1;
	}
	void y(float _y) {
		second = _y;
		_length = -1;
	}
	void xPlus(float _x) {
		first += _x;
		_length = -1;
	}
	void yPlus(float _y) {
		second += _y;
		_length = -1;
	}
	void swap(Point& _swap) {
		Point temp = _swap;
		_swap = *this;
		*this = temp;
	}
	float lengthSquared() {
		return first * first + second * second;
	}
	float length() {
		if(_length < 0) _length = sqrt(first * first + second * second);
		return _length;
	}
	Point unitVector() {
		float temp = length();
		return Point(first / temp, second / temp);
	}
	Point operator-(const Point& other){
		return Point(first - other.first,second - other.second);
	}
	Point operator+(const Point& other) {
		return Point(first + other.first, second + other.second);
	}
	float operator*(const Point& other) {//dot product
		return first * other.first + second * other.second;
	}
	Point operator/(const float& constant) {
		return Point(first/constant, second/constant);
	}
	Point operator*(const float& constant) {
		return Point(first*constant, second*constant);
	}
};
class Object {
public:
	Point pos;//위치
	Point vel; //속도
	Point acc; //가속도
	Object(){
	}
	virtual float max_x() {
		return 0;
	};
	virtual float max_y() {
		return 0;
	};
	virtual Shape* getShape() {
		return nullptr;
	};
	virtual bool hitTest(Object &obj) {
		return false;
	}
	virtual void penetrationCorrection(Object &obj) {

	}
	void draw(RenderWindow &window) {
		window.draw(*getShape());
	}
	void move() {
		if (this->pos.x() < 0 || this->pos.x() + this->max_x() > 600) {
			this->vel.set(true, friction, false, friction), this->acc.set(true, friction, false, friction);
			if (this->pos.x() < 0) {
				this->pos.x(0);
			}
			else if (this->pos.x() + this->max_x() > 600) {
				this->pos.x(600 - this->max_x());
			}
		}
		if (this->pos.y() + this->max_y() > 480) {
			this->vel.set(false, friction, true, friction), this->acc.set(false, friction, true, friction);
			this->pos.y(480 - this->max_y());
		}

		this->vel = this->vel + this->acc + Point(0, G);
		this->pos = this->pos + this->vel;
		this->getShape()->setPosition(this->pos.x(), this->pos.y());
	}
};
class AABB :public Object{
public:
	RectangleShape shape;
	AABB() {
		shape.setSize(Vector2f(20,20));
		Color color(rand() % 100 + 130, rand() % 100 + 130, rand() % 100 + 130);
		getShape()->setFillColor(color);
		getShape()->setPosition(0, 0);
	}
	AABB(Point size) {
		shape.setSize(Vector2f(size.x(),size.y()));
		Color color(rand() % 100 + 130, rand() % 100 + 130, rand() % 100 + 130);
		getShape()->setFillColor(color);
		getShape()->setPosition(0, 0);
	}
	float max_x() {
		return shape.getSize().x;
	}
	float max_y() {
		return shape.getSize().y;
	}
	Shape* getShape() {
		return &shape;
	};
	bool hitTest(AABB &other) {
		if (this->shape.getPosition().x + max_x() < other.shape.getPosition().x ||
			this->shape.getPosition().x > other.shape.getPosition().x + other.max_x())
			return false;
		if (this->shape.getPosition().y + max_y() < other.shape.getPosition().y ||
			this->shape.getPosition().y > other.shape.getPosition().y + other.max_y())
			return false;
		return true;
	}
	void penetrationCorrection(AABB& other) {
		Point distance = this->pos - other.pos;
		float x_penetration_length = this->max_x()/2 + other.max_x()/2 - fabs(distance.x());
		float y_penetration_length = this->max_y()/2 + other.max_y()/2 - fabs(distance.y());
		float k;
		Point unit;
		if (x_penetration_length < y_penetration_length) {
			k = x_penetration_length;
			unit = Point(distance.x() > 0 ? -1 : 1, 0);
		}
		else {
			k = y_penetration_length;
			unit = Point(0,distance.y() > 0 ? -1 : 1);
		}
		if (k > 0.01) {
			float percent = 0.8;
			Point correction = (unit * (k * percent / 2));
			this->pos = this->pos - correction;
			other.pos = other.pos + correction;
		}
	}
};
class Ball :public Object{
public:
	CircleShape shape;
	Ball() {
		Color color(rand() % 100 + 130, rand() % 100 + 130, rand() % 100 + 130);
		getShape()->setFillColor(color);
		getShape()->setPosition(0, 0);
		shape.setRadius(diameter/2);
	};
	float max_x() {
		return diameter;
	}
	float max_y() {
		return diameter;
	}
	Shape* getShape() {
		return &shape;
	};
	bool hitTest(Ball& other) {
		return (this->pos-other.pos).lengthSquared() <= Square(this->shape.getRadius() + other.shape.getRadius());
	}
	Point unit(Ball& other) {
		return (this->pos - other.pos).unitVector();//충돌방향 단위벡터
	}
	void penetrationCorrection(Ball& other) {
		float k = diameter - (this->pos - other.pos).length();//겹친 거리
		if (k > 0.01) {
			float percent = 0.8;
			Point correction = (unit(other) * (k * percent / 2));
			this->pos = this->pos + correction;
			other.pos = other.pos - correction;
		}
	}
};
static vector<Ball> balls(num);

void push() {
	for (int i = 0; i < num; ++i) {
		for (int j = 0; j < num; ++j) {
			if (i != j) {
				if ( (balls[i].pos - balls[j].pos).lengthSquared() <= Square(diameter)) {
					float angle = ((rand() % 180) - 90) * (PI / 180);
					int m = 0;
					if (i > j) {
						m = i;
					}
					else {
						m = j;
					}
					float distance = (balls[i].pos - balls[j].pos).length();
					if (balls[m].pos.x() + (diameter -distance) * cos(angle) < 590) balls[m].pos.xPlus((diameter - distance) * cos(angle));
					if (balls[m].pos.y() + (diameter - distance) * sin(angle) < 450) balls[m].pos.yPlus((diameter - distance) * sin(angle));
					balls[m].getShape()->setPosition(balls[m].pos.x(), balls[m].pos.y());
				};
			};
		};
	};
};


bool comp(Object a, Object b) {
	return a.pos.x() < b.pos.x();
}
void simplehit() {
	//x좌표 기준으로 정렬하자.
	sort(balls.begin(), balls.end(), comp);
	int size = balls.size();

	for (int i = 0; i < size; ++i) {
		int count = 0;
		for (int j = i + 1; j < size; ++j) {

			if (balls[i].hitTest(balls[j])) {
				Point rv = balls[j].vel - balls[i].vel;//상대속도
				Point unit = (balls[j].pos - balls[i].pos).unitVector();//충돌방향 단위벡터
				float vel = unit * rv;
				vel = -vel;
				balls[i].vel = balls[i].vel - unit * vel;
				balls[j].vel = balls[j].vel + unit * vel;

				balls[i].penetrationCorrection(balls[j]);//박힌 위치 조정
				count++;
			}
			else if(count >= 5) break;

		}
	}
}
int main() {
	srand(time(NULL));
	for (int i = 0; i < 100; ++i) push();
	RenderWindow window(VideoMode(600, 480), "engine v1");
	window.setFramerateLimit(60);
	for (int i = 0; i < num; ++i) {
		float temp1 = (rand() % 5) * 1.3 / (num);
		float temp2 = (rand() % 5) * 1.3 / (num);
		balls[i].acc = Point(temp1, temp2);
		balls[i].pos = Point(rand() % 580, rand() % 450);
	};
	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
		}
		window.clear();
		simplehit();
		for (int i = 0; i < num; ++i) {
			balls[i].move();
			balls[i].draw(window);
		};
		window.display();
	}
}
