#ifndef MATRIX_SIMD_H
#define MATRIX_SIMD_H

#define PROXY_ENABLE 1

#include "Vect4D_SIMD.h"
#include <xmmintrin.h>
#include <smmintrin.h> 


class Matrix_SIMD
{
public:
	Matrix_SIMD()
	{
	}

	Matrix_SIMD( const Vect4D_SIMD &tV0,const Vect4D_SIMD &tV1,const Vect4D_SIMD &tV2,const Vect4D_SIMD &tV3)
		: v0(tV0), v1(tV1), v2(tV2), v3(tV3)
	{
		
	}

	Matrix_SIMD(const __m128 &tV0, const __m128 &tV1, const __m128 &tV2, const __m128 &tV3)
		: v0(tV0), v1(tV1), v2(tV2), v3(tV3)
	{

	}
	
	~Matrix_SIMD()
	{

	}

	Matrix_SIMD(const Matrix_SIMD &tmp)
		: v0(tmp.v0), v1(tmp.v1), v2(tmp.v2), v3(tmp.v3)
	{
		
	}

	Matrix_SIMD &operator=(const Matrix_SIMD &tmp)
	{
		this->v0 = tmp.v0;
		this->v1 = tmp.v1;
		this->v2 = tmp.v2;
		this->v3 = tmp.v3;
		return *this;
	}

#if !PROXY_ENABLE

	Matrix_SIMD operator * (const Matrix_SIMD & t);

	Vect4D_SIMD operator * (const Vect4D_SIMD &v);

#endif

	union
	{
		struct
		{
			Vect4D_SIMD v0;
			Vect4D_SIMD v1;
			Vect4D_SIMD v2;
			Vect4D_SIMD v3;
		};

		struct
		{
			float m0;
			float m1;
			float m2;
			float m3;
			float m4;
			float m5;
			float m6;
			float m7;
			float m8;
			float m9;
			float m10;
			float m11;
			float m12;
			float m13;
			float m14;
			float m15;
		};
	};
};


#if PROXY_ENABLE

inline Vect4D_SIMD operator * (const Matrix_SIMD & m, const Vect4D_SIMD & v)
{
	return
		_mm_hadd_ps
		(
			_mm_hadd_ps(/* x */_mm_mul_ps(m.v0._m, v._m), /* y */_mm_mul_ps(m.v1._m, v._m)),
			_mm_hadd_ps(/* z */_mm_mul_ps(m.v2._m, v._m), /* w */_mm_mul_ps(m.v3._m, v._m))
		);
}

inline Matrix_SIMD MtM(const Matrix_SIMD& m1, const Matrix_SIMD& m2)
{
	__m128 colX = _mm_setr_ps(m2.m0, m2.m4, m2.m8, m2.m12);
	__m128 colY = _mm_setr_ps(m2.m1, m2.m5, m2.m9, m2.m13);
	__m128 colZ = _mm_setr_ps(m2.m2, m2.m6, m2.m10, m2.m14);
	__m128 colW = _mm_setr_ps(m2.m3, m2.m7, m2.m11, m2.m15);


	return
		Matrix_SIMD(
			_mm_hadd_ps /*new Matrix.m1.v0._m*/
			(
				_mm_hadd_ps(/*[0,0]*/_mm_mul_ps(m1.v0._m, colX), /*[0,1]*/_mm_mul_ps(m1.v0._m, colY)),
				_mm_hadd_ps(/*[0,2]*/_mm_mul_ps(m1.v0._m, colZ), /*[0,3]*/_mm_mul_ps(m1.v0._m, colW))
			),

			_mm_hadd_ps /*new Matrix.m1.v0._m*/
			(
				_mm_hadd_ps(/*[1,0]*/_mm_mul_ps(m1.v1._m, colX), /*[1,1]*/_mm_mul_ps(m1.v1._m, colY)),
				_mm_hadd_ps(/*[1,2]*/_mm_mul_ps(m1.v1._m, colZ), /*[1,3]*/_mm_mul_ps(m1.v1._m, colW))
			),

			_mm_hadd_ps /*new Matrix.m1.v0._m*/
			(
				_mm_hadd_ps(/*[2,0]*/_mm_mul_ps(m1.v2._m, colX), /*[2,1]*/_mm_mul_ps(m1.v2._m, colY)),
				_mm_hadd_ps(/*[2,2]*/_mm_mul_ps(m1.v2._m, colZ), /*[2,3]*/_mm_mul_ps(m1.v2._m, colW))
			),

			_mm_hadd_ps /*new Matrix.m1.v0._m*/
			(
				_mm_hadd_ps(/*[3,0]*/_mm_mul_ps(m1.v3._m, colX), /*[3,1]*/_mm_mul_ps(m1.v3._m, colY)),
				_mm_hadd_ps(/*[3,2]*/_mm_mul_ps(m1.v3._m, colZ), /*[3,3]*/_mm_mul_ps(m1.v3._m, colW))
			)
		);
}

struct MM
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;

	MM(const Matrix_SIMD& s1, const Matrix_SIMD& s2) : m1(s1), m2(s2)
	{

	}

	MM& operator =(const MM&)
	{
		return *this;
	}

	operator Matrix_SIMD()
	{
		return MtM(m1, m2);
	}
};

inline MM operator *(const Matrix_SIMD& s1, const Matrix_SIMD& s2)
{
	return MM(s1, s2);
}

struct MMM
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Matrix_SIMD& m3;

	MMM(const MM& s1, const Matrix_SIMD& s2) : m1(s1.m1), m2(s1.m2), m3(s2)
	{

	}

	MMM& operator =(const MMM&)
	{
		return *this;
	}

	operator Matrix_SIMD()
	{
		return MtM(m1, MtM(m2, m3));
	}
};

inline MMM operator *(const MM& s1, const Matrix_SIMD& s2)
{
	return MMM(s1, s2);
}

struct MMMM
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Matrix_SIMD& m3;
	const Matrix_SIMD& m4;

	MMMM(const MMM& s1, const Matrix_SIMD& s2) : m1(s1.m1), m2(s1.m2), m3(s1.m3), m4(s2)
	{

	}

	MMMM& operator =(const MMMM&)
	{
		return *this;
	}

	operator Matrix_SIMD()
	{
		return MtM(m1, MtM(m2, MtM(m3, m4)));
	}
};

inline MMMM operator *(const MMM& s1, const Matrix_SIMD& s2)
{
	return MMMM(s1, s2);
}

struct MMMMM
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Matrix_SIMD& m3;
	const Matrix_SIMD& m4;
	const Matrix_SIMD& m5;

	MMMMM(const MMMM& s1, const Matrix_SIMD& s2) : m1(s1.m1), m2(s1.m2), m3(s1.m3), m4(s1.m4), m5(s2)
	{

	}

	MMMMM& operator =(const MMMMM&)
	{
		return *this;
	}

	operator Matrix_SIMD()
	{
		return MtM(m1, MtM(m2, MtM(m3, MtM(m4, m5))));
	}
};

inline MMMMM operator *(const MMMM& s1, const Matrix_SIMD& s2)
{
	return MMMMM(s1, s2);
}

struct MMV
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Vect4D_SIMD& v;

	MMV(const MM& s1, const Vect4D_SIMD& s2) : m1(s1.m1), m2(s1.m2), v(s2)
	{

	}

	MMV& operator =(const MMV&)
	{
		return *this;
	}

	operator Vect4D_SIMD()
	{
		return Vect4D_SIMD(m1 * (m2 * v));
	}
};

inline MMV operator *(const MM& s1, const Vect4D_SIMD& s2)
{
	return MMV(s1, s2);
}

struct MMMV
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Matrix_SIMD& m3;
	const Vect4D_SIMD& v;

	MMMV(const MMM& s1, const Vect4D_SIMD& s2) : m1(s1.m1), m2(s1.m2), m3(s1.m3), v(s2)
	{

	}

	MMMV& operator =(const MMMV&)
	{
		return *this;
	}

	operator Vect4D_SIMD()
	{
		return Vect4D_SIMD(m1 * (m2 *(m3 * v)));
	}
};

inline MMMV operator *(const MMM& s1, const Vect4D_SIMD& s2)
{
	return MMMV(s1, s2);
}

struct MMMMV
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Matrix_SIMD& m3;
	const Matrix_SIMD& m4;
	const Vect4D_SIMD& v;

	MMMMV(const MMMM& s1, const Vect4D_SIMD& s2) : m1(s1.m1), m2(s1.m2), m3(s1.m3), m4(s1.m4), v(s2)
	{

	}

	MMMMV& operator =(const MMMMV&)
	{
		return *this;
	}

	operator Vect4D_SIMD()
	{
		return Vect4D_SIMD(m1 * (m2 *(m3 *(m4 * v))));
	}
};

inline MMMMV operator *(const MMMM& s1, const Vect4D_SIMD& s2)
{
	return MMMMV(s1, s2);
}

struct MMMMMV
{
	const Matrix_SIMD& m1;
	const Matrix_SIMD& m2;
	const Matrix_SIMD& m3;
	const Matrix_SIMD& m4;
	const Matrix_SIMD& m5;
	const Vect4D_SIMD& v;

	MMMMMV(const MMMMM& s1, const Vect4D_SIMD& s2) : m1(s1.m1), m2(s1.m2), m3(s1.m3), m4(s1.m4), m5(s1.m5), v(s2)
	{

	}

	MMMMMV& operator =(const MMMMMV&)
	{
		return *this;
	}

	operator Vect4D_SIMD()
	{
		return Vect4D_SIMD(m1 * (m2 *(m3 *(m4 *(m5 * v)))));
	}
};

inline MMMMMV operator *(const MMMMM& s1, const Vect4D_SIMD& s2)
{
	return MMMMMV(s1, s2);
}

#endif


#endif