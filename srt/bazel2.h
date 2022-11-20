#pragma once
// not finish this file yet,
// don't use anything of this file
#include <algorithm>

template<class T>
int gwinding1(T* b, double x0, double x1, double y0) {
	if (!b->intersect(x0, x1, y0)) {
		return 0;
	} else {
		if (b->x0 > x0 && b->ex < x1) {
			return (b->ey > b->y0) ? 1 : -1;
		} else {
			double dy = b->ey - b->y0;
			if (dy == 0) return 0;
			double dx = b->ex - b->x0;
			double dyp = y0 - b->y0;
			double dxp0 = x0 - b->x0;
			double dxp1 = x1 - b->x0;
			double dxp = dyp / dy * dx;
			if (dxp > dxp0 && dxp < dxp1) {
				return (b->ey > b->y0) ? 1 : -1;
			} else {
				return 0;
			}
		}
	}

}

struct bazel1 {
	double x0, y0;
	double ex, ey;

	bool intersect(double x0, double x1, double y0) {
		if (this->y0 > y0 && this->ey >= y0) {
			return false;
		} else if (this->y0 < y0 && this->ey <= y0) {
			return false;
		} else if (this->x0 > x1 && this->ex >= x1) {
			return false;
		} else if (this->x0 < x0 && this->ex <= x0) {
			return false;
		}
		return true; // possible
	}
	int winding(double x0, double x1, double y0, int n = 20) {
		return gwinding1(this, x0, x1, y0);

	}
};

template<class T>
int gwinding(T* b, double x0, double x1, double y0, int n = 20) {
	if (n == 0) {
		// assume a line
		return gwinding1(b, x0, x1, y0);
	}
	if (!b->intersect(x0, x1, y0)) {
		return 0;
	} else if (b->isMonotone()) {
		if (b->x0 > x0 && b->ex < x1) {
			return (b->ey > b->y0) ? 1 : -1;
		}
	}
	// not sure
	T sub1, sub2;
	b->splitAtMid(sub1, sub2);
	return gwinding(&sub1, x0, x1, y0, n - 1)
		+ gwinding(&sub2, x0, x1, y0, n - 1);

}

static constexpr double mix(double a, double b, double t) {
	return (1 - t) * a + b * t;
}

static constexpr double sqr(double x) {
	return x * x;
}

struct bazel3 {
	double x0, y0;
	double cx, cy;
	double cx1, cy1;
	double ex, ey;

	void splitAtMid(bazel3& sub1, bazel3& sub2) const {
		bazel3 s1;
		bazel3 s2;
		double x1 = 0.5 * (x0 + cx);  // E
		double y1 = 0.5 * (y0 + cy);
		double x2 = 0.5 * (cx + cx1); // F
		double y2 = 0.5 * (cy + cy1);
		double x3 = 0.5 * (ex + cx1); // G
		double y3 = 0.5 * (ey + cy1);
		double x4 = 0.5 * (x1 + x2);  // H
		double y4 = 0.5 * (y1 + y2);
		double x5 = 0.5 * (x2 + x3);  // J
		double y5 = 0.5 * (y2 + y3);
		double x6 = 0.5 * (x4 + x5);  // K
		double y6 = 0.5 * (y4 + y5);
		s1.x0 = x0;
		s1.y0 = y0;
		s1.cx = x1;
		s1.cy = y1;
		s1.cx1 = x4;
		s1.cy1 = y4;
		s1.ex = x6;
		s1.ey = y6;

		s2.x0 = x6;
		s2.y0 = y6;
		s2.cx = x5;
		s2.cy = y5;
		s2.cx1 = x3;
		s2.cy1 = y3;
		s2.ex = ex;
		s2.ey = ey;
		sub1 = s1;
		sub2 = s2;
	}
	bool isMonotone() {
		bool a = ((ex - cx1) > 0) && ((cx1 - cx) > 0) && ((cx - x0) > 0);
		bool b = ((ex - cx1) < 0) && ((cx1 - cx) < 0) && ((cx - x0) < 0);
		bool c = ((ey - cy1) > 0) && ((cy1 - cy) > 0) && ((cy - y0) > 0);
		bool d = ((ey - cy1) < 0) && ((cy1 - cy) < 0) && ((cy - y0) < 0);

		return (a || b) && (c || d);
	}

	bool intersect(double x0, double x1, double y0) {
		if (this->y0 > y0 && this->cy > y0 && this->cy1 > y0 && this->ey >= y0) {
			return false;
		} else if (this->y0 < y0 && this->cy < y0 && this->cy1 < y0 && this->ey <= y0) {
			return false;
		} else if (this->x0 > x1 && this->cx > x1 && this->cx1 > x1 && this->ex >= x1) {
			return false;
		} else if (this->x0 < x0 && this->cx < x0 && this->cx1 < x0 && this->ex <= x0) {
			return false;
		}
		return true; // possible
	}

	double getX(double t) {
		double a = mix(this->x0, this->cx, t);
		double b = mix(this->cx, this->cx1, t);
		double c = mix(this->cx1, this->ex, t);
		double x = mix(a, b, t);
		double y = mix(b, c, t);
		double z = mix(x, y, t);
		return z;
	}
#if 0
	int solve(double a, double b, double c, double d, double x1, double x2, double y) {
		
		
	}
#endif

	int winding(double x0, double x1, double y0, int n = 20) {
		return gwinding(this, x0, x1, y0, n);
	}

	double cub(double x) {
		return x * x * x;
	}

};

struct bazel2 {
	double x0, y0;
	double cx, cy;
	double ex, ey;

	void splitAtMid(bazel2& sub1, bazel2& sub2) const {
		bazel2 s1;
		bazel2 s2;
		double x1 = 0.5 * (x0 + cx);
		double y1 = 0.5 * (y0 + cy);
		double x2 = 0.5 * (ex + cx);
		double y2 = 0.5 * (ey + cy);
		double x3 = 0.5 * (x1 + x2);
		double y3 = 0.5 * (y1 + y2);
		s1.x0 = x0;
		s1.y0 = y0;
		s1.cx = x1;
		s1.cy = y1;
		s1.ex = x3;
		s1.ey = y3;

		s2.x0 = x3;
		s2.y0 = y3;
		s2.cx = x2;
		s2.cy = y2;
		s2.ex = ex;
		s2.ey = ey;
		sub1 = s1;
		sub2 = s2;
	}

	bool isMonotone() {
		return ((ex - cx) * (cx - x0) >= 0)
			&& ((ey - cy) * (cy - y0) >= 0);
	}

	bool intersect(double x0, double x1, double y0) {
		if (this->y0 > y0 && this->cy > y0 && this->ey > y0) {
			return false;
		} else if (this->y0 < y0 && this->cy < y0 && this->ey < y0) {
			return false;
		} else if (this->x0 > x1 && this->cx > x1 && this->ex > x1) {
			return false;
		} else if (this->x0 < x0 && this->cx < x0 && this->ex < x0) {
			return false;
		}
		return true; // possible
	}


	double getX(double t) {
		return mix(mix(this->x0, this->cx, t),
			mix(this->cx, this->ex, t), t);
	}

	int winding(double x0, double x1, double y0, int n = 20) {
		return gwinding(this, x0, x1, y0, n);
	}

	double wind(double t, double x1, double x2, double w) {
		double x = getX(t);
		if (x > x1 && x < x2) {
			return w;
		}
	}


	double solve(double a, double b, double c, double x1, double x2, double y)
	{
		// y = (1-t)[(1-t) a + t b] + t[(1-t) b + t c]
		// 0 <= t < 1
		if (c < a) {
			return -solve(-a, -b, -c, x1, x2, -y);
		}

		double d = (a - 2 * b + c);
		double Delta = sqr(a - b) + d * (y - a);
		if (y > c) {
			if (b > c && Delta > 0) {
				double del = sqrt(Delta) / d;
				double mid = (a - b) / d;
				return wind(mid - del, x1, x2, -1)
					+ wind(mid + del, x1, x2, +1);
			}
		} else if (y == c) {
			if (a == c) {
				if(b > a){
					return wind(1., x1, x2, -0.5)
						+ wind(0, x1, x2, +0.5);
				} else if (b < a) {
					return wind(0., x1, x2, -0.5)
						+ wind(1, x1, x2, +0.5);
				}
			}
			if (b > c) {
				double t = (a - c) / d;
				return wind(1., x1, x2, -0.5)
					+ wind(t, x1, x2, +1);
			} else if ((b < c) || (a < b)) {
				return wind(1., x1, x2, 0.5);
			}
		} else {
			// y < c
			if (y > a) {
				if (b > a && b < c) {
					double M = sqrt(Delta < 0 ? 0 : Delta) + (b - a);
					double t = (y - a) / M;
					return wind(t, x1, x2, 1);
				} else {
					double del = sqrt(Delta) / d;
					double mid = (a - b) / d;
					return wind(mid - del, x1, x2, -1)
						+ wind(mid + del, x1, x2, +1);
				}
			} else if (y == a) {
				if (b < a) {
					double t = 2 * (a - b) / d;
					return wind(0., x1, x2, -0.5)
						+ wind(t, x1, x2, +1);
				} else if ((b > a) || (c > b)) {
					return wind(0., x1, x2, 0.5);
				}
			} else {
				// y < a
				if (b < a && Delta > 0) {
					double del = sqrt(Delta) / d;
					double mid = (a - b) / d;
					return wind(mid - del, x1, x2, -1)
						+ wind(mid + del, x1, x2, +1);
				}
			}
		}
		return 0;
	}

	int winding2(double x0, double x1, double y0)
	{
		if (!intersect(x0, x1, y0)) {
			return 0;
		} else {
			// y = (1-t)[(1-t) y0 + t cy] + t[(1-t) cy + t ey]
			double a = this->y0, b = cy, c = ey;
			return solve(a, b, c, x0, x1, y0);
		}
	}


};

struct bazelx {

	int type;
	union {
		bazel1 b1;
		bazel2 b2;
		bazel3 b3;
	};
};

struct Box2
{
	double x0, x1, y0, y1;

	bool contains(double x, double y) {
		return (x >= x0) && (x <= x1) && (y >= y0) && (y <= y1);
	}
};

struct KDNode
{

	std::shared_ptr<KDNode> node1;
	std::shared_ptr<KDNode> node2;
	double sep;
	int dim;
	void* data = nullptr;

	void* get(double x, double y);
};

void* KDNode::get(double x, double y) {
	if (dim == 0) {
		if (x <= sep) {
			if (node1)
				return node1->get(x, y);
		} else {
			if (node2)
				return node2->get(x, y);
		}
	} else if (dim == 1) {
		if (y <= sep) {
			if (node1)
				return node1->get(x, y);
		} else {
			if (node2)
				return node2->get(x, y);
		}
	} else {
		return data;
	}
	return nullptr;
}

struct Data {
	double getX0(int) {
		return 0;
	}
	double getX1(int) {
		return 0;
	}
	double getY0(int) {
		return 0;
	}
	double getY1(int) {
		return 0;
	}
	int getN() {
		return 0;
	}
};
#include <vector>
struct node {
	int rangeIdx;
	int open;
	int prev;
};
void foo(Data &data) {
	int n = data.getN();

	std::vector<node> nodes(2 * n);
	for (int i = 0; i < n; ++i) {
		nodes[2 * i + 0].open = true;
		nodes[2 * i + 1].open = false;
		nodes[2 * i + 0].rangeIdx = i;
		nodes[2 * i + 1].rangeIdx = i;
	}

	std::sort(nodes.begin(), nodes.end(), [&](node &i, node &j) {
		double a = i.open ? data.getX0(i.rangeIdx) : data.getX1(i.rangeIdx);
		double b = j.open ? data.getX0(j.rangeIdx) : data.getX1(j.rangeIdx);
		return a < b;
		});

	std::vector<int> open;
	open.push_back(-1);
	for (int i = 0; i < 2*n; ++i) {
		if (nodes[i].open) {
			nodes[i].prev = open.back();
			open.push_back(nodes[i].rangeIdx);
		} else {
			auto f = std::find(open.begin(), open.end(), nodes[i].rangeIdx);
			if (f != open.end()) {
				open.erase(f);
			}
		}
		nodes[i].prev = open.back();
	}


}

struct bazelbox
{

	Box2 box;
	std::vector<std::shared_ptr<bazelbox>> subs;

	bool contains(double x, double y) {
		if (!box.contains(x, y)) {
			return false;
		}
	}

};

struct shape {

	std::vector<bazelbox> b;
};