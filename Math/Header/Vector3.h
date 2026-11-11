#pragma once
#include <math.h>
#include <cmath>

#include <limits>
#include <algorithm>

#ifndef HALF_CIRCLE
#define HALF_CIRCLE 180.0f
#endif

namespace Math
{
	/**
	 * @class Vec3
	 * @brief Generic 3-component vector (x, y, z) with the usual vector algebra.
	 *
	 * Vec3 is a template so it can be instantiated over any numeric type T
	 * (see the Math::vec3 alias for Vec3<float> below). It provides component
	 * access, arithmetic operators, normalization, dot/cross products,
	 * projection/reflection helpers, and a set of named unit-direction
	 * constants (up, down, left, right, forward, backward).
	 */
	template <typename T> class Vec3
	{
	public:
		/** @brief Converts an angle in radians to degrees. */
		const double static radiansToDegrees(const double rad)
		{
			return rad * (HALF_CIRCLE / M_PI);
		}

		/** @brief Converts an angle in degrees to radians. */
		const double static degreesToRadians(const double deg)
		{
			return deg * (M_PI / HALF_CIRCLE);
		}

	private:
		T x;
		T y;
		T z;

	public:
		static const T unitVal;
		static const Vec3<T> zero;
		static const Vec3<T> up;
		static const Vec3<T> down;
		static const Vec3<T> left;
		static const Vec3<T> right;
		static const Vec3<T> forward;
		static const Vec3<T> backward;
		static const Vec3<T> error;

		/** @brief Creates a zero-initialized vector. */
		Vec3()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
		}

		/** @brief Initializes the vector components from one value. */
		Vec3(const T init)
		{
			this->x = init;
			this->y = init;
			this->z = init;
		}

		/** @brief Creates a vector from explicit component values. */
		Vec3(const T x, const T y, const T z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		/** @brief Copy-constructs a vector from another vector. */
		Vec3(const Vec3<T>& vec3)
		{
			this->x = vec3.x;
			this->y = vec3.y;
			this->z = vec3.z;
		}

		/** @brief Move-constructs a vector from another vector. */
		Vec3(Vec3<T>&& vec3) noexcept
		{
			this->x = std::move(vec3.x);
			this->y = std::move(vec3.y);
			this->z = std::move(vec3.z);
		}

		/** @brief Assigns the components of another vector. */
		Vec3& operator=(const Vec3<T>& vec3)
		{
			if (this == &vec3)
				return *this;

			this->x = vec3.x;
			this->y = vec3.y;
			this->z = vec3.z;

			return *this;
		}

		/** @brief Assigns the components of another vector. */
		Vec3& operator=(Vec3<T>&& vec3) noexcept
		{
			if (this == &vec3)
				return *this;

			this->x = std::move(vec3.x);
			this->y = std::move(vec3.y);
			this->z = std::move(vec3.z);

			return *this;
		}

		/** @brief Returns a modifiable reference to the X component. */
		T& X()
		{
			return this->x;
		}

		/** @brief Returns a modifiable reference to the Y component. */
		T& Y()
		{
			return this->y;
		}

		/** @brief Returns a modifiable reference to the Z component. */
		T& Z()
		{
			return this->z;
		}

		/** @brief Returns the X component. */
		const T X() const
		{
			return this->x;
		}

		/** @brief Returns the Y component. */
		const T Y() const
		{
			return this->y;
		}

		/** @brief Returns the Z component. */
		const T Z() const
		{
			return this->z;
		}

		/** @brief Returns the vector magnitude. */
		const T length() const
		{
			return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
		}

		/** @brief Returns the squared magnitude without computing a square root. */
		const T length2() const
		{
			return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
		}

		/** @brief Checks whether the vector has approximately unit length. */
		const bool isUnit() const
		{
			return std::abs(this->length() - Vec3<T>::unitVal) <= std::numeric_limits<T>::epsilon() ||
				std::abs(this->length() - Vec3<T>::unitVal) <= std::numeric_limits<T>::epsilon() * std::max(std::abs(this->length()), std::abs(Vec3<T>::unitVal));
		}

		/** @brief Computes the dot product with another vector. */
		const T dotProduct(const Vec3<T>& vec3) const
		{
			return (this->x * vec3.x) + (this->y * vec3.y) + (this->z * vec3.z);
		}

		/** @brief Computes the cross product with another vector. */
		const Vec3<T> crossProduct(const Vec3<T>& vec3) const
		{
			return Vec3<T>((this->y * vec3.z) - (this->z * vec3.y),
				(this->z * vec3.x) - (this->x * vec3.z),
				(this->x * vec3.y) - (this->y * vec3.x));
		}

		/** @brief Computes the angle between vectors in radians. */
		const T radAngle(const Vec3<T>& vec3) const
		{
			const T product = this->dotProduct(vec3);

			/*const T lengthA2 = this->length2();
			const T lengthB2 = vec3.length2();
			const T length2Product = lengthA2 * lengthB2;

			const T length = sqrt(length2Product);*/

			//return std::acos(product / length);
			return std::acos(product / (this->length() * vec3.length()));
		}

		/** @brief Computes the angle between vectors in degrees. */
		const T degAngle(const Vec3<T>& vec3) const
		{
			return radiansToDegrees(this->radAngle(vec3));
		}

		/** @brief Projects this vector onto the supplied vector. */
		Vec3<T> project(const Vec3<T>& vec3) const
		{
			const T product = this->dotProduct(vec3);
			const T length = vec3.length2();
			const T division = product / length;

			return Vec3<T>(division * vec3.x, division * vec3.y, division * vec3.z);
		}

		/** @brief Computes the reflection relative to the supplied vector. */
		Vec3<T> reflexion(const Vec3<T>& vec3) const
		{
			if (!vec3.isUnit())
			{
				Vec3<T> proj = this->project(vec3);
				proj *= 2;
				return Vec3<T>(this->x - proj.x, this->y - proj.y, this->z - proj.z);
			}

			const T product = this->dotProduct(vec3);
			const T doubledProduct = 2 * product;
			const Vec3<T> vecProduct = doubledProduct * vec3;

			return Vec3<T>(this->x - vecProduct.x, this->y - vecProduct.y, this->z - vecProduct.z);
		}

		/** @brief Returns the component perpendicular to the supplied vector. */
		Vec3<T> perpendicular(const Vec3<T>& vec3) const
		{
			const Vec3<T> proj = this->project(vec3);
			return Vec3<T>(this->x - proj.x, this->y - proj.y, this->z - proj.z);
		}

		/** @brief Adds another vector and returns the result. */
		Vec3<T> add(const Vec3<T>& vec3) const
		{
			return Vec3<T>(this->x + vec3.x, this->y + vec3.y, this->z + vec3.z);
		}

		/** @brief Adds another vector to this vector. */
		Vec3<T>& add(const Vec3<T>& vec3)
		{
			this->x += vec3.x;
			this->y += vec3.y;
			this->z += vec3.z;
			return *this;
		}

		/** @brief Scales the vector and returns the result. */
		Vec3<T> scale(const T scale) const
		{
			return Vec3<T>(this->x * scale, this->y * scale, this->z * scale);
		}

		/** @brief Scales this vector in place. */
		Vec3<T>& scale(const T scale)
		{
			this->x *= scale;
			this->y *= scale;
			this->z *= scale;
			return *this;
		}

		/** @brief Scales this vector in place. */
		Vec3<T>& scale(const Vec3<T>& vec3)
		{
			this->x *= vec3.X();
			this->y *= vec3.Y();
			this->z *= vec3.Z();
			return *this;
		}

		/** @brief Returns a vector scaled component-wise by another vector. */
		Vec3<T> scaled(const Vec3<T>& vec3) const
		{
			return Math::Vec3<T>(this->x * vec3.X(), this->y * vec3.Y(), this->z * vec3.Z());
		}

		/** @brief Returns the distance to the supplied vector. */
		float distance(const Vec3<T>& vec3) const
		{
			return std::sqrt(((this->x - vec3.x) * (this->x - vec3.x)) + ((this->y - vec3.y) * (this->y - vec3.y)) + ((this->z - vec3.z) * (this->z - vec3.z)));
		}

		/** @brief Returns a normalized copy of this vector. */
		Vec3<T> normalized() const
		{
			T length = this->length();
			if (length == 0)
				return Vec3<T>(0, 0, 0);
			return Vec3<T>(this->x / length, this->y / length, this->z / length);
		}

		/** @brief Normalizes this vector in place. */
		void normalize()
		{
			T length = this->length();
			if (length == 0)
			{
				this->x = 0;
				this->y = 0;
				this->z = 0;
				return;
			}

			this->x /= length;
			this->y /= length;
			this->z /= length;
		}

		/** @brief Accesses a vector component using the supplied index. */
		T& operator[](const int idx)
		{
			switch (idx)
			{
			case 0:		return x;
			case 1:		return y;
			case 2:		return z;

			case 'x':	return x;
			case 'y':	return y;
			case 'z':	return z;

			default:	return x;
			}
		}

		/** @brief Accesses a vector component using the supplied index. */
		const T operator[](const int idx) const
		{
			switch (idx)
			{
			case 0:		return x;
			case 1:		return y;
			case 2:		return z;

			case 'x':	return x;
			case 'y':	return y;
			case 'z':	return z;

			default:	return x;
			}
		}

		/** @brief Accesses a vector component using the supplied index. */
		T& operator[](const char* idx)
		{
			switch (idx[0])
			{
			case 'x': return x;
			case 'y': return y;
			case 'z': return z;

			default: return x;
			}
		}

		/** @brief Accesses a vector component using the supplied index. */
		const T operator[](const char* idx) const
		{
			switch (idx[0])
			{
			case 'x': return x;
			case 'y': return y;
			case 'z': return z;

			default: return x;
			}
		}

		/** @brief Compares vectors for equality. */
		const bool operator==(const Vec3<T>& vec3) const
		{
			if (this == &vec3)
				return true;

			return std::abs(this->length() - vec3.length()) <= std::numeric_limits<T>::epsilon() ||
				std::abs(this->length() - vec3.length()) <= std::numeric_limits<T>::epsilon() * std::max(std::abs(this->length()), std::abs(vec3.length()));
		}

		/** @brief Compares vectors for inequality. */
		const bool operator!=(const Vec3<T>& vec3) const
		{
			return !(*this == vec3);
		}

		/** @brief Compares vector ordering. */
		const bool operator<(const Vec3<T>& vec3) const
		{
			return this->length() < vec3.length();
		}

		/** @brief Compares vector ordering. */
		const bool operator<=(const Vec3<T>& vec3) const
		{
			return this->length() < vec3.length() || (*this == vec3);
		}

		/** @brief Compares vector ordering. */
		const bool operator>(const Vec3<T>& vec3) const
		{
			return !(*this < vec3);
		}

		/** @brief Compares vector ordering. */
		const bool operator>=(const Vec3<T>& vec3) const
		{
			return !(*this <= vec3);
		}

		/** @brief Returns the sum of two vectors. */
		Vec3<T> operator+(const Vec3<T>& vec3) const
		{
			return Vec3(this->x + vec3.x, this->y + vec3.y, this->z + vec3.z);
		}

		/** @brief Adds another vector in place. */
		void operator+=(const Vec3<T>& vec3)
		{
			this->x += vec3.x;
			this->y += vec3.y;
			this->z += vec3.z;
		}

		/** @brief Returns the difference or negation of vectors. */
		Vec3<T> operator-(const Vec3<T>& vec3) const
		{
			return Vec3(this->x - vec3.x, this->y - vec3.y, this->z - vec3.z);
		}

		/** @brief Subtracts another vector in place. */
		void operator-=(const Vec3<T>& vec3)
		{
			this->x -= vec3.x;
			this->y -= vec3.y;
			this->z -= vec3.z;
		}

		/** @brief Returns the difference or negation of vectors. */
		Vec3<T> operator-() const
		{
			return Vec3(-this->x, -this->y, -this->z);
		}

		/** @brief Returns the product with a scalar or vector. */
		Vec3<T> operator*(const T value) const
		{
			return this->scale(value);
		}

		/** @brief Multiplies this vector in place. */
		void operator*=(const T value)
		{
			*this = this->scale(value);
		}

		/** @brief Returns the product with a scalar or vector. */
		Vec3<T> operator*(const Vec3<T>& vec3) const
		{
			return this->crossProduct(vec3);
		}

		/** @brief Multiplies this vector in place. */
		void operator*=(const Vec3<T>& vec3)
		{
			*this = this->crossProduct(vec3);
		}

		/** @brief Returns the result of scalar division. */
		Vec3<T> operator/(const T value) const
		{
			return Vec3(this->x / value, this->y / value, this->z / value);
		}

		/** @brief Divides this vector by a scalar in place. */
		void operator/=(const T value)
		{
			this->x /= value;
			this->y /= value;
			this->z /= value;
		}

		/** @brief Increments the vector components. */
		void operator++(int)
		{
			T length = this->length();
			this->x *= (length + 1) / length;
			this->y *= (length + 1) / length;
			this->z *= (length + 1) / length;
		}

		/** @brief Decrements the vector components. */
		void operator--(int)
		{
			T length = this->length();
			this->x *= (length - 1) / length;
			this->y *= (length - 1) / length;
			this->z *= (length - 1) / length;
		}

		/** @brief Linearly interpolates between two vectors using the given factor. */
		static Math::Vec3<T> lerp(Math::Vec3<T>& a, Math::Vec3<T>& b, float t)
		{
			return a + (b - a) * t;
		}
	};

	template<class T> Vec3<T> operator*(const double value, const Vec3<T>& vec3)
	{
		return vec3 * T(value);
	}

	template<class T> Vec3<T> operator/(const double value, const Vec3<T>& vec3)
	{
		return Vec3<T>(T(value / vec3.X()), T(value / vec3.Y()), T(value / vec3.Z()));
	}

	//template<class T> const T operator,(const Vec3<T>& vec1, const Vec3<T>& vec2)
	//{
	//	return vec1.dotProduct(vec2);
	//}

	template<class T> const Vec3<T> Vec3<T>::zero = Vec3();
	template<class T> const Vec3<T> Vec3<T>::up(0, 1, 0);
	template<class T> const Vec3<T> Vec3<T>::down(0, -1, 0);
	template<class T> const Vec3<T> Vec3<T>::left(-1, 0, 0);
	template<class T> const Vec3<T> Vec3<T>::right(1, 0, 0);
	template<class T> const Vec3<T> Vec3<T>::forward(0, 0, -1);
	template<class T> const Vec3<T> Vec3<T>::backward(0, 0, 1);
	template<class T> const Vec3<T> Vec3<T>::error(-1, -1, -1);
	template<class T> const T Vec3<T>::unitVal = 1;

	typedef Vec3<float> vec3;
}
