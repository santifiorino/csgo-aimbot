#include "Vector3.h"

Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
    return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

Vector3 operator-(const Vector3& lhs, const Vector3& rhs) {
    return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

Vector3 operator*(const Vector3& vec, const float scalar) {
    return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

float magnitude(const Vector3& vector) {
    return sqrt(pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2));
}

float distance(const Vector3& lhs, const Vector3& rhs) {
    return magnitude(lhs - rhs);
}

Vector3 normalize(const Vector3& vector) {
    return vector * (1 / magnitude(vector));
}

Vector3 Vector3::radiansToDegrees() const {
    return Vector3(x * 180 / PI, y * 180 / PI, z * 180 / PI);
}

Vector3 Vector3::anglesToDirection() const {
    Vector3 directionVector = Vector3(cos(y * PI / 180), sin(y * PI / 180), tan(x * PI / 180));
    return normalize(directionVector);
}

bool Vector3::isZero() const {
    return (x == 0.0f && y == 0.0f && z == 0.0f);
}