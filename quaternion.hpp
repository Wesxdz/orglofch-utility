#ifndef _QUATERNION_HPP_
#define _QUATERNION_HPP_

#include "algebra.hpp"

union Quaternion 
{
	Quaternion() {
		x = y = z = 0;
		w = 1;
	}
	Quaternion(double _x, double _y, double _z, double _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	Quaternion(const Vector3 &v, double _w) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = _w;
	}
	Quaternion(const Vector4 &v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	Quaternion conjugate() {
		return Quaternion(-x, -y, -z, w);
	}

	double norm() const {
		return sqrt(x*x + y*y + z*z + w*w);
	}

	/*Quaternion inverse() {
		return conjugate() / norm();
	}*/

	Quaternion unit() {
		double n = norm();
		return Quaternion(x / n, y / n, z / n, w / n);
	}

	Matrix4x4 matrix() const {
		double m1 = 2 * y * y;
		double m2 = 2 * z * z;
		double m3 = 2 * x * y;
		double m4 = 2 * z * w;
		double m5 = 2 * x * z;
		double m6 = 2 * y * w;
		double m7 = 2 * x * x;
		double m8 = 2 * y * z;
		double m9 = 2 * x * w;
		return Matrix4x4(Vector4(1 - m1 - m2, m3 - m4, m5 + m6, 0),
			Vector4(m3 + m4, 1 - m7 - m2, m8 - m9, 0),
			Vector4(m5 - m6, m8 + m9, 1 - m7 - m1, 0),
			Vector4(0, 0, 0, 1));
	}

	void operator *= (const Quaternion &q) {
		x = y * q.z - z * q.y + x * q.w + w * q.x;
		y = z * q.x - x * q.z + y * q.w + w * q.y;
		z = x * q.y - y * q.x + z * q.w + w * q.z;
		w = w * q.w - x * q.x - y * q.y - z * q.z;
	}

	struct
	{
		double x, y, z, w;
	};
private:
	double d[4];
};

inline
Quaternion operator / (const Quaternion &a, const double s) {
	return Quaternion(a.x / s, a.y / s, a.z / s, a.w / s);
}

inline
Quaternion operator * (const Quaternion &a, const Quaternion &b) {
	return Quaternion(a.y * b.z - a.z * b.y + a.x * b.w + a.w * b.x,
		a.z * b.x - a.x * b.z + a.y * b.w + a.w * b.y,
		a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z,
		a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);
}

inline
Quaternion operator + (const Quaternion &a, const Quaternion &b) {
	return Quaternion(a.x + b.x, a.y + b.y, a.z + b.y, a.w + b.w);
}

inline
Quaternion operator - (const Quaternion &a, Quaternion &b) {
	return Quaternion(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

#endif