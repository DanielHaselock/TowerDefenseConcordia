#pragma once
#include "Vector3.h"
namespace Math
{
	/**
	 * @class Vec4
	 * @brief Generic 4-component vector (x, y, z, w) with the usual vector algebra.
	 *
	 * Vec4 is a template so it can be instantiated over any numeric type T.
	 * Alongside the same component access, arithmetic operators, and
	 * named unit-direction constants as Vec3, it adds a w component (e.g.
	 * for homogeneous coordinates or, elsewhere in this codebase, as a
	 * packed x/y/width/height rectangle) and a constructor that extends a
	 * Vec3 with an explicit w.
	 */
	template <typename T> class Vec4
	{
	private:
		T x;
		T y;
		T z;
		T w;

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

		static const T unitVal;
		static const Vec4<T> zero;
		static const Vec4<T> up;
		static const Vec4<T> down;
		static const Vec4<T> left;
		static const Vec4<T> right;
		static const Vec4<T> forward;
		static const Vec4<T> backward;

		/** @brief Creates a zero-initialized vector. */
		Vec4()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
			this->w = 0;
		}

		/** @brief Initializes the vector components from one value. */
		Vec4(const T init)
		{
			this->x = init;
			this->y = init;
			this->z = init;
			this->w = 0;
		}

		/** @brief Creates a vector from explicit component values. */
		Vec4(const T x, const T y, const T z, const T w = 0)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		/** @brief Extends a three-component vector with a W value. */
		Vec4(const Vec3<T>& vec3, float w = 0)
		{
			this->x = vec3.X();
			this->y = vec3.Y();
			this->z = vec3.Z();
			this->w = w;
		}

		/** @brief Copy-constructs a vector from another vector. */
		Vec4(const Vec4<T>& vec4)
		{
			this->x = vec4.x;
			this->y = vec4.y;
			this->z = vec4.z;
			this->w = vec4.w;
		}

		/** @brief Move-constructs a vector from another vector. */
		Vec4(Vec4<T>&& vec4) noexcept
		{
			this->x = std::move(vec4.x);
			this->y = std::move(vec4.y);
			this->z = std::move(vec4.z);
			this->w = std::move(vec4.w);
		}

		/** @brief Assigns the components of another vector. */
		Vec4& operator=(const Vec4<T>& vec4)
		{
			if (this == &vec4)
				return *this;

			this->x = vec4.x;
			this->y = vec4.y;
			this->z = vec4.z;
			this->w = vec4.w;

			return *this;
		}

		/** @brief Assigns the components of another vector. */
		Vec4& operator=(Vec4<T>&& vec4) noexcept
		{
			if (this == &vec4)
				return *this;

			this->x = std::move(vec4.x);
			this->y = std::move(vec4.y);
			this->z = std::move(vec4.z);
			this->w = std::move(vec4.w);

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

		/** @brief Returns a modifiable reference to the W component. */
		T& W()
		{
			return this->w;
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

		/** @brief Returns the W component. */
		const T W() const
		{
			return this->w;
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
			return std::abs(this->length() - Vec4<T>::unitVal) <= std::numeric_limits<T>::epsilon() ||
				std::abs(this->length() - Vec4<T>::unitVal) <= std::numeric_limits<T>::epsilon() * std::max(std::abs(this->length()), std::abs(Vec4<T>::unitVal));
		}

		/** @brief Computes the dot product with another vector. */
		const T dotProduct(const Vec4<T>& vec4) const
		{
			return (this->x * vec4.x) + (this->y * vec4.y) + (this->z * vec4.z);
		}

		/** @brief Computes the cross product with another vector. */
		Vec4<T> crossProduct(const Vec4<T>& vec4) const
		{
			return Vec4<T>((this->y * vec4.z) - (this->z * vec4.y),
				(this->z * vec4.x) - (this->x * vec4.z),
				(this->x * vec4.y) - (this->y * vec4.x), 0);
		}

		/** @brief Computes the angle between vectors in radians. */
		const T radAngle(const Vec4<T>& vec4) const
		{
			const T product = this->dotProduct(vec4);

			const T lengthA2 = this->length2();
			const T lengthB2 = vec4.length2();
			const T length2Product = lengthA2 * lengthB2;

			const T length = sqrt(length2Product);

			return std::acos(product / length);
		}

		/** @brief Computes the angle between vectors in degrees. */
		const T degAngle(const Vec4<T>& vec4) const
		{
			return radiansToDegrees(this->radAngle(vec4));
		}

		/** @brief Projects this vector onto the supplied vector. */
		Vec4<T> project(const Vec4<T>& vec4) const
		{
			const T product = this->dotProduct(vec4);
			const T length = vec4.length2();
			const T division = product / length;

			return Vec4<T>(division * vec4.x, division * vec4.y, division * vec4.z, 1);
		}

		/** @brief Computes the reflection relative to the supplied vector. */
		Vec4<T> reflexion(const Vec4<T>& vec4) const
		{
			if (!vec4.isUnit())
			{
				Vec4<T> proj = this->project(vec4);
				proj *= 2;
				return Vec4<T>(this->x - proj.x, this->y - proj.y, this->z - proj.z, 1);
			}

			const T product = this->dotProduct(vec4);
			const T doubledProduct = 2 * product;
			const Vec4<T> vecProduct = doubledProduct * vec4;

			return Vec4<T>(this->x - vecProduct.x, this->y - vecProduct.y, this->z - vecProduct.z, 1);
		}

		/** @brief Returns the component perpendicular to the supplied vector. */
		Vec4<T> perpendicular(const Vec4<T>& vec4) const
		{
			const Vec4<T> proj = this->project(vec4);
			return Vec4<T>(this->x - proj.x, this->y - proj.y, this->z - proj.z, 1);
		}

		/** @brief Adds another vector and returns the result. */
		Vec4<T> add(const Vec4<T>& vec4) const
		{
			return Vec4<T>(this->x + vec4.x, this->y + vec4.y, this->z + vec4.z, 1);
		}

		/** @brief Adds another vector to this vector. */
		Vec4<T>& add(const Vec4<T>& vec4)
		{
			this->x += vec4.x;
			this->y += vec4.y;
			this->z += vec4.z;
			return *this;
		}

		/** @brief Scales the vector and returns the result. */
		Vec4<T> scale(const T scale) const
		{
			return Vec4<T>(this->x * scale, this->y * scale, this->z * scale, this->w * scale);
		}

		/** @brief Scales this vector in place. */
		Vec4<T>& scale(const T scale)
		{
			this->x *= scale;
			this->y *= scale;
			this->z *= scale;
			return *this;
		}

		/** @brief Returns a normalized copy of this vector. */
		Vec4<T> normalized() const
		{
			T length = this->length();
			return Vec4<T>(this->x / length, this->y / length, this->z / length, 0);
		}

		/** @brief Normalizes this vector in place. */
		void normalize()
		{
			T length = this->length();
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
			case 3:		return w;

			case 'x':	return x;
			case 'y':	return y;
			case 'z':	return z;
			case 'w':	return w;

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
			case 3:		return w;

			case 'x':	return x;
			case 'y':	return y;
			case 'z':	return z;
			case 'w':	return w;

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
			case 'w': return w;

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
			case 'w': return w;

			default: return x;
			}
		}

		/** @brief Compares vectors for equality. */
		const bool operator==(const Vec4<T>& vec4) const
		{
			if (this == &vec4)
				return true;

			return std::abs(this->length() - vec4.length()) <= std::numeric_limits<T>::epsilon() ||
				std::abs(this->length() - vec4.length()) <= std::numeric_limits<T>::epsilon() * std::max(std::abs(this->length()), std::abs(vec4.length()));
		}

		/** @brief Compares vectors for inequality. */
		const bool operator!=(const Vec4<T>& vec4) const
		{
			return !(*this == vec4);
		}

		/** @brief Compares vector ordering. */
		const bool operator<(const Vec4<T>& vec4) const
		{
			return this->length() < vec4.length();
		}

		/** @brief Compares vector ordering. */
		const bool operator<=(const Vec4<T>& vec4) const
		{
			return this->length() < vec4.length() || (*this == vec4);
		}

		/** @brief Compares vector ordering. */
		const bool operator>(const Vec4<T>& vec4) const
		{
			return !(*this < vec4);
		}

		/** @brief Compares vector ordering. */
		const bool operator>=(const Vec4<T>& vec4) const
		{
			return !(*this <= vec4);
		}

		/** @brief Returns the sum of two vectors. */
		Vec4<T> operator+(const Vec4<T>& vec4) const
		{
			return Vec4(this->x + vec4.x, this->y + vec4.y, this->z + vec4.z, this->w + vec4.w);
		}

		/** @brief Adds another vector in place. */
		void operator+=(const Vec4<T>& vec4)
		{
			this->x += vec4.x;
			this->y += vec4.y;
			this->z += vec4.z;
		}

		/** @brief Returns the difference or negation of vectors. */
		Vec4<T> operator-(const Vec4<T>& vec4) const
		{
			return Vec4(this->x - vec4.x, this->y - vec4.y, this->z - vec4.z, this->w - vec4.w);
		}

		/** @brief Subtracts another vector in place. */
		void operator-=(const Vec4<T>& vec4)
		{
			this->x -= vec4.x;
			this->y -= vec4.y;
			this->z -= vec4.z;
		}

		/** @brief Returns the difference or negation of vectors. */
		Vec4<T> operator-() const
		{
			return Vec4(-this->x, -this->y, -this->z);
		}

		/** @brief Returns the product with a scalar or vector. */
		Vec4<T> operator*(const T value) const
		{
			return this->scale(value);
		}

		/** @brief Multiplies this vector in place. */
		void operator*=(const T value)
		{
			*this = this->scale(value);
		}

		/** @brief Returns the product with a scalar or vector. */
		Vec4<T> operator*(const Vec4<T>& vec4) const
		{
			return this->crossProduct(vec4);
		}

		/** @brief Multiplies this vector in place. */
		void operator*=(const Vec4<T>& vec4)
		{
			*this = this->crossProduct(vec4);
		}

		/** @brief Returns the result of scalar division. */
		Vec4<T> operator/(const T value) const
		{
			return Vec4(this->x / value, this->y / value, this->z / value);
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

		/** @brief Divides the spatial components by W to convert from homogeneous coordinates. */
		void homogenize()
		{
			if (this->w == 0)
				return;

			this->x /= this->w;
			this->y /= this->w;
			this->z /= this->w;
		}

		Vec3<T> homogenized()
		{
			if (this->w == 0)
				return Vec3<T>();

			return Vec3<T>(this->x / this->w, this->y / this->w, this->z / this->w);
		}

		/** @brief Returns the vector magnitude. */
		float GetMagnitude()
		{
			return this->length();
		}
	};

	template<class T> Vec4<T> operator*(const double value, const Vec4<T>& vec4)
	{
		return vec4 * T(value);
	}

	template<class T> Vec4<T> operator/(const double value, const Vec4<T>& vec4)
	{
		return Vec4<T>(T(value / vec4.X()), T(value / vec4.Y()), T(value / vec4.Z()));
	}

	template<class T> const T operator,(const Vec4<T>& vec1, const Vec4<T>& vec2)
	{
		return vec1.dotProduct(vec2);
	}

	template<class T> const Vec4<T> Vec4<T>::zero = Vec4();
	template<class T> const Vec4<T> Vec4<T>::up(0, 1, 0, 0);
	template<class T> const Vec4<T> Vec4<T>::down(0, -1, 0, 0);
	template<class T> const Vec4<T> Vec4<T>::left(-1, 0, 0, 0);
	template<class T> const Vec4<T> Vec4<T>::right(1, 0, 0, 0);
	template<class T> const Vec4<T> Vec4<T>::forward(0, 0, -1, 0);
	template<class T> const Vec4<T> Vec4<T>::backward(0, 0, 1, 0);
	template<class T> const T Vec4<T>::unitVal = 1;
}
