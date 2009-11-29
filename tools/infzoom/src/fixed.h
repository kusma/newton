#ifndef _FIXED_H_
#define _FIXED_H_

#include <limits.h> // for INT_MAX / INT_MIN
#include <assert.h>
#ifndef ASSERT
#define ASSERT assert
#endif

#include "math.h"

template <int frac_bits>
class fixed
{
public:
	INLINE fixed() {}
	INLINE fixed(const int val) : val(val << frac_bits) {}
	INLINE fixed(const double val) : val(int(val * (1<<frac_bits))) {}

	INLINE static fixed make_raw(const int val)
	{
		fixed f;
		f.val = val;
		return f;
	}

	INLINE operator int() const
	{
		return val >> frac_bits;
	}

	INLINE operator float() const
	{
		return float(val) * (1.f / (1 << frac_bits));
	}

	INLINE operator double() const
	{
		return double(val) * (1.0 / (1 << frac_bits));
	}

	INLINE fixed operator-() const
	{
		return make_raw(-val);
	}

	INLINE fixed operator+(const fixed f) const
	{
		ASSERT(((long long)val + f.val) <= INT_MAX);
		ASSERT(((long long)val + f.val) >= INT_MIN);
		return make_raw(val + f.val);
	}

	INLINE fixed operator-(const fixed f) const
	{
		ASSERT(((long long)val - f.val) <= INT_MAX);
		ASSERT(((long long)val - f.val) >= INT_MIN);
		return make_raw(val - f.val);
	}

	INLINE fixed operator/(const fixed f)
	{
		return *this * f.rcp();
	}

	INLINE fixed operator*(const fixed f) const
	{
		ASSERT((((long long)val * f.val) >> frac_bits) <= INT_MAX);
		ASSERT((((long long)val * f.val) >> frac_bits) >= INT_MIN);
		return make_raw(int(((long long)val * f.val) >> frac_bits));
	}

	INLINE fixed operator*(const int i) const
	{
		ASSERT(((long long)val * i) <= INT_MAX);
		ASSERT(((long long)val * i) >= INT_MIN);
		return make_raw(val * i);
	}

	INLINE fixed operator>>(const int i) const
	{
		return make_raw(val >> i);
	}

	INLINE fixed operator<<(const int i) const
	{
		ASSERT(((long long)val << i) <= INT_MAX);
		return make_raw(val << i);
	}

	INLINE const fixed &operator+=(const fixed f)
	{
		ASSERT(((long long)val + f.val) <= INT_MAX);
		ASSERT(((long long)val + f.val) >= INT_MIN);
		val += f.val;
		return *this;
	}

	INLINE const fixed &operator-=(const fixed f)
	{
		ASSERT(((long long)val - f.val) <= INT_MAX);
		ASSERT(((long long)val - f.val) >= INT_MIN);
		val -= f.val;
		return *this;
	}

	INLINE const fixed &operator/=(const fixed f)
	{
		*this *= f.rcp();
		return *this;
	}

	INLINE const fixed &operator*=(const fixed f)
	{
		ASSERT((((long long)val * f.val) >> frac_bits) <= INT_MAX);
		ASSERT((((long long)val * f.val) >> frac_bits) >= INT_MIN);
		val = int(((long long)val * f.val) >> frac_bits);
		return *this;
	}

	INLINE const fixed &operator*=(const int i)
	{
		ASSERT(((long long)val * i) <= INT_MAX);
		ASSERT(((long long)val * i) >= INT_MIN);
		val = val * i;
		return *this;
	}

	INLINE const fixed &operator>>=(const int i)
	{
		val = val >> i;
		return *this;
	}

	INLINE const fixed &operator<<=(const int i)
	{
		ASSERT(((long long)val << i) <= INT_MAX);
		val = val << i;
		return *this;
	}

	INLINE const bool operator>(const fixed f) const
	{
		return this->val > f.val;
	}

	INLINE const bool operator<(const fixed f) const
	{
		return this->val < f.val;
	}

	INLINE const bool operator>(const int i) const
	{
		return this->val > fixed(i);
	}

	INLINE const bool operator<(const int i) const
	{
		return this->val < fixed(i);
	}

	INLINE const bool operator>=(const fixed f) const
	{
		return this->val >= f.val;
	}

	INLINE const bool operator<=(const fixed f) const
	{
		return this->val <= f.val;
	}

	INLINE const bool operator==(const fixed f) const
	{
		return this->val == f.val;
	}

	INLINE fixed abs()
	{
		if (val > 0) return *this;
		return -(*this);
	}

	// wrap rcp
	INLINE fixed rcp() const
	{
		return make_raw(::rcp(val));
	}

	// wrap rsq
	INLINE fixed rsq() const
	{
		return make_raw(::rsq(val));
	}

	// wrap sin
	INLINE fixed sin() const
	{
		return make_raw(::fixed_sin(val));
	}

	// wrap cos
	INLINE fixed cos() const
	{
		return make_raw(::fixed_cos(val));
	}

	INLINE int get_val() const
	{
		return val;
	}

	INLINE int &get_val()
	{
		return val;
	}

protected:
	int val;
};

typedef fixed<16> fixed16;

INLINE PURE fixed16 abs(const fixed16 &in)
{
	if (in.get_val() > 0) return in;
	return -in;
}

INLINE PURE fixed16 rcp(const fixed16 &in)
{
	return fixed16::make_raw(::rcp(in.get_val()));
}

// wrap rsq
INLINE PURE fixed16 rsq(const fixed16 &in)
{
	return fixed16::make_raw(::rsq(in.get_val()));
}

INLINE PURE fixed16 sqrt(const fixed16 &in)
{
	return rcp(rsq(in));
}

// wrap sin
INLINE PURE fixed16 sin(const fixed16 &in)
{
	return fixed16::make_raw(::fixed_sin(in.get_val()));
}

// wrap cos
INLINE PURE fixed16 cos(const fixed16 &in)
{
	return fixed16::make_raw(::fixed_cos(in.get_val()));
}

#endif /* _FIXED_H_ */
