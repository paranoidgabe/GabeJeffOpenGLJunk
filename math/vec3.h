#pragma once

#include "Def.h"

#include <ostream>
class MATH_API Vec3 {

private:
	float v[3];
public:
	Vec3(void);
	Vec3(float x, float y, float z);
	Vec3(const float*);

	float getX() const;
	float getY() const;
	float getZ() const;

	Vec3 operator+(const Vec3 &rhs) const;
	Vec3 operator-(const Vec3 &rhs) const;
	float operator*(const Vec3 &rhs) const;

	void normalize();
	void scale(float);

	Vec3 clone() const;

	Vec3 cross(const Vec3&) const;

};

std::ostream& operator<<(std::ostream& os, Vec3 &obj);
