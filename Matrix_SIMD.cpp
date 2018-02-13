#include "Matrix_SIMD.h"	

#if !PROXY_ENABLE

Matrix_SIMD Matrix_SIMD::operator * (const Matrix_SIMD &t)
{	
	__m128 colX = _mm_setr_ps(t.m0, t.m4, t.m8, t.m12);
	__m128 colY = _mm_setr_ps(t.m1, t.m5, t.m9, t.m13);
	__m128 colZ = _mm_setr_ps(t.m2, t.m6, t.m10, t.m14);
	__m128 colW = _mm_setr_ps(t.m3, t.m7, t.m11, t.m15);


	return
	Matrix_SIMD(
		_mm_hadd_ps /*new Matrix.v0._m*/
		(
			_mm_hadd_ps(/*[0,0]*/_mm_mul_ps(v0._m, colX), /*[0,1]*/_mm_mul_ps(v0._m, colY)),
			_mm_hadd_ps(/*[0,2]*/_mm_mul_ps(v0._m, colZ), /*[0,3]*/_mm_mul_ps(v0._m, colW))
		),

		_mm_hadd_ps /*new Matrix.v1._m*/
		(
			_mm_hadd_ps(/*[1,0]*/_mm_mul_ps(v1._m, colX), /*[1,1]*/_mm_mul_ps(v1._m, colY)),
			_mm_hadd_ps(/*[1,2]*/_mm_mul_ps(v1._m, colZ), /*[1,3]*/_mm_mul_ps(v1._m, colW))
		),

		_mm_hadd_ps /*new Matrix.v2._m*/
		(
			_mm_hadd_ps(/*[2,0]*/_mm_mul_ps(v2._m, colX), /*[2,1]*/_mm_mul_ps(v2._m, colY)),
			_mm_hadd_ps(/*[2,2]*/_mm_mul_ps(v2._m, colZ), /*[2,3]*/_mm_mul_ps(v2._m, colW))
		),

		_mm_hadd_ps /*new Matrix.v3._m*/
		(
			_mm_hadd_ps(/*[3,0]*/_mm_mul_ps(v3._m, colX), /*[3,1]*/_mm_mul_ps(v3._m, colY)),
			_mm_hadd_ps(/*[3,2]*/_mm_mul_ps(v3._m, colZ), /*[3,3]*/_mm_mul_ps(v3._m, colW))
		)
	);
}

Vect4D_SIMD Matrix_SIMD::operator * (const Vect4D_SIMD &v)
{
	return
		_mm_hadd_ps
		(
			_mm_hadd_ps(/* x */_mm_mul_ps(v0._m, v._m), /* y */_mm_mul_ps(v1._m, v._m)),
			_mm_hadd_ps(/* z */_mm_mul_ps(v2._m, v._m), /* w */_mm_mul_ps(v3._m, v._m))
		);
}

#endif