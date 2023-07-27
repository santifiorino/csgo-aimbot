#pragma once
#include <iostream>

#define PI 3.14159265358979323846

class Vector3 {

public:
	float x, y, z;

	Vector3(const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f) noexcept :
		x(x), y(y), z(z) { }

	friend Vector3 operator+(const Vector3& lhs, const Vector3& rhs);
	friend Vector3 operator-(const Vector3& lhs, const Vector3& rhs);
	friend Vector3 operator*(const Vector3& vec, const float scalar);
	friend float dot(const Vector3& lhs, const Vector3& rhs);
	friend float magnitude(const Vector3& vector);

	Vector3 toDegrees() const;
	Vector3 toDirection() const;
	bool isZero() const;
};