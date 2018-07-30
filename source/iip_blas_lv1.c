#include "iip_blas_lv1.h"
#include <math.h>

/*
 *
 * NTYPE == 0 -> float ->  <T> = real : s | complex : c
 * NTYPE == 1 -> double -> <T> = real : d | complex : z
 *  
 *
 */

/*********************************************
************ AXPY  ***************************
**********************************************/
/*
 *
 *  y = alpha * x + y
 *  
 *
 *  <?>axpy(integer N, DTYPE alpha, DTYPE *x, integer incx, DTYPE beta )
 * */
void axpy(DTYPE alpha, MAT *x, MAT *y)
{
	UINT size = x->d0 * x->d1 * x->d2;
#if DEBUG
	printf("%s\n", __func__);
#endif

#if USE_CBLAS

#if NTYPE == 0
	cblas_saxpy(size, alpha, x->data, 1, y->data, 1);

#elif NTYPE == 1
	cblas_daxpy(size, alpha, x->data, 1, y->data, 1);
#endif

#else
	mp_axpy(size, alpha, x->data, 1, y->data, 1);
#endif
}

void mp_axpy(UINT N, DTYPE alpha, DTYPE *X, UINT INCX, DTYPE *Y, UINT INCY)
{
	ITER i;

#if DEBUG
	printf("%s\n", __func__);
#endif

#pragma omp parallel for shared(X, Y) private(i)
	for (i = 0; i < N; i++)
	{
		Y[i * INCY] = X[i * INCX] * alpha + Y[i * INCY];
	}
}

void caxpy(CTYPE alpha, CMAT *x, CMAT *y)
{
	UINT size = x->d0 * x->d1 * x->d2;
#if DEBUG
	printf("%s\n", __func__);
#endif
#if USE_CBLAS
#if NTYPE == 0
	cblas_caxpy(size, &alpha, x->data, 1, y->data, 1);

#elif NTYPE == 1
	cblas_zaxpy(size, &alpha, x->data, 1, y->data, 1);
#endif

#else
	mp_caxpy(size, alpha, x->data, 1, y->data, 1);
#endif
}

void mp_caxpy(UINT N, CTYPE alpha, CTYPE *X, UINT INCX, CTYPE *Y, UINT INCY)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	ITER i;

#pragma omp parallel for shared(X, Y) private(i)
	for (i = 0; i < N; i++)
	{
		Y[i * INCY].re = X[i * INCX].re * alpha.re + Y[i * INCY].re;
		Y[i * INCY].im = X[i * INCX].im * alpha.im + Y[i * INCY].im;
	}
}

/*********************************************
************ COPY  ***************************
**********************************************/
/*  copies a vector, x, to a vector y,
 *
 *  y = x
 *
 *  uses unrolled loops for increments equal to 1.
 *
 *  <?>acopy(integer N, DTYPE* X, intefer INCX, DTYPE* Y, integer INCY)
 * */
void copy(MAT *src, MAT *des)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return 0;
	}

#if USE_CBLAS
#if NTYPE == 0
	cblas_scopy(mat_size, src->data, 1, des->data, 1);

#elif NTYPE == 1
	cblas_dcopy(mat_size, src->data, 1, des->data, 1);
#endif

//USE_BLAS = 0 -> just c implement
#else
	mp_copy(mat_size, src->data, 1, des->data, 1);
#endif
}

void mp_copy(UINT N, DTYPE *src, SINT src_inc, DTYPE *des, SINT des_inc)
{
	ITER iteration = 8;
	UINT repeat = N >> 3;
	UINT left = N & (UINT)(iteration - 1);
	UINT i = 0;
	UINT j = 0;

#pragma omp parallel for shared(des, src) private(j, i)
	for (j = 0; j < repeat; j++)
	{
		i = j * iteration;
		des[(i)*des_inc] = src[(i)*src_inc];
		des[(i + 1) * des_inc] = src[(i + 1) * src_inc];
		des[(i + 2) * des_inc] = src[(i + 2) * src_inc];
		des[(i + 3) * des_inc] = src[(i + 3) * src_inc];
		des[(i + 4) * des_inc] = src[(i + 4) * src_inc];
		des[(i + 5) * des_inc] = src[(i + 5) * src_inc];
		des[(i + 6) * des_inc] = src[(i + 6) * src_inc];
		des[(i + 7) * des_inc] = src[(i + 7) * src_inc];
	}

	for (j = 0; j < left; j++)
	{
		des[(i + j) * des_inc] = src[(i + j) * src_inc];
	}
}

void ccopy(CMAT *src, CMAT *des)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return 0;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_ccopy(mat_size, src->data, 1, des->data, 1);

//DTYPE = double
#elif NTYPE == 1
	cblas_zcopy(mat_size, src->data, 1, des->data, 1);
#endif

//USE_BLAS = 0 -> just c implement
#else
	mp_ccopy(mat_size, src->data, 1, des->data, 1);
#endif
}

void mp_ccopy(UINT N, CTYPE *src, SINT src_inc, CTYPE *des, SINT des_inc)
{
	ITER iteration = 8;
	UINT repeat = N >> 3;
	UINT left = N & (UINT)(iteration - 1);
	UINT i = 0, j = 0;

#pragma omp parallel for shared(des, src) private(j, i)
	for (j = 0; j < repeat; j++)
	{
		i = j * iteration;
		des[(i)*des_inc].re = src[(i)*src_inc].re;
		des[(i)*des_inc].im = src[(i)*src_inc].im;
		des[(i + 1) * des_inc].re = src[(i + 1) * src_inc].re;
		des[(i + 1) * des_inc].im = src[(i + 1) * src_inc].im;
		des[(i + 2) * des_inc].re = src[(i + 2) * src_inc].re;
		des[(i + 2) * des_inc].im = src[(i + 2) * src_inc].im;
		des[(i + 3) * des_inc].re = src[(i + 3) * src_inc].re;
		des[(i + 3) * des_inc].im = src[(i + 3) * src_inc].im;
		des[(i + 4) * des_inc].re = src[(i + 4) * src_inc].re;
		des[(i + 4) * des_inc].im = src[(i + 4) * src_inc].im;
		des[(i + 5) * des_inc].re = src[(i + 5) * src_inc].re;
		des[(i + 5) * des_inc].im = src[(i + 5) * src_inc].im;
		des[(i + 6) * des_inc].re = src[(i + 6) * src_inc].re;
		des[(i + 6) * des_inc].im = src[(i + 6) * src_inc].im;
		des[(i + 7) * des_inc].re = src[(i + 7) * src_inc].re;
		des[(i + 7) * des_inc].im = src[(i + 7) * src_inc].im;
	}

	for (j = 0; j < left; j++)
	{
		des[(i + j) * des_inc].re = src[(i + j) * src_inc].re;
		des[(i + j) * des_inc].im = src[(i + j) * src_inc].im;
	}
}

/*** Get sum of the magnitudes of elements of a vector ***/
DTYPE asum(MAT *mat, UINT inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = mat->d0 * mat->d1 * mat->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return 0;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	return cblas_sasum(mat_size, mat->data, inc);

//DTYPE = double
#elif NTYPE == 1
	return cblas_dasum(mat_size, mat->data, inc);
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_asum(mat_size, mat->data, inc);
#endif
}
DTYPE mp_asum(UINT N, DTYPE *data, UINT inc)
{
	UINT i = 0;
	DTYPE sum = 0;

	for (i = 0; i < N; i++)
	{
		sum += (data[i * inc] < 0 ? (-data[i * inc]) : data[i * inc]);
	}

	return sum;
}

DTYPE casum(CMAT *mat, UINT inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = mat->d0 * mat->d1 * mat->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return 0;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	return cblas_scasum(mat_size, mat->data, inc);

//DTYPE = double
#elif NTYPE == 1
	return cblas_dzasum(mat_size, mat->data, inc);
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_casum(mat_size, mat->data, inc);
#endif
}

DTYPE mp_casum(UINT N, CTYPE *data, UINT inc)
{
	UINT i = 0;
	DTYPE sum = 0;

	for (i = 0; i < N; i++)
	{
		sum += (data[i * inc].re < 0 ? (-data[i * inc].re) : data[i * inc].re);
		sum += (data[i * inc].im < 0 ? (-data[i * inc].im) : data[i * inc].im);
	}

	return sum;
}

/*** Get a vector-vector dot product ***/
DTYPE dot(MAT *src_x, UINT x_increment, MAT *src_y, UINT y_increment)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return 0;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	return cblas_sdot(mat_size, src_x->data, x_increment, src_y->data, y_increment);

//DTYPE = double
#elif NTYPE == 1
	return cblas_ddot(mat_size, src_x->data, x_increment, src_y->data, y_increment);
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_dot(mat_size, src_x->data, x_increment, src_y->data, y_increment);
#endif
}

DTYPE mp_dot(UINT N, DTYPE *src_x, UINT x_inc, DTYPE *src_y, UINT y_inc)
{
	UINT i = 0;
	DTYPE dot = 0;

#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 0; i < N; i++)
	{
		dot += src_x[i * x_inc] * src_y[i * y_inc];
	}

	return dot;
}

CTYPE cdot(CMAT *src_x, UINT x_increment, MAT *src_y, UINT y_increment)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;
	CTYPE result = {0, 0};

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return result;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_cdotc_sub(mat_size, src_x->data, x_increment, src_y->data, y_increment, &result);
	return result;

//DTYPE = double
#elif NTYPE == 1
	cblas_zdotc_sub(mat_size, src_x->data, x_increment, src_y->data, y_increment, &result);
	return result;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_cdot(mat_size, src_x->data, x_increment, src_y->data, y_increment);
#endif
}

CTYPE mp_cdot(UINT N, CTYPE *src_x, UINT x_inc, DTYPE *src_y, UINT y_inc)
{
	UINT i = 0;
	CTYPE dot;

	dot.re = 0;
	dot.im = 0;

#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 0; i < N; i++)
	{
		dot.re += src_x[i * x_inc].re * src_y[i * y_inc];
		dot.im += src_x[i * x_inc].im * src_y[i * y_inc];
	}

	return dot;
}

CTYPE udot(CMAT *src_x, UINT x_increment, CMAT *src_y, UINT y_increment)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;
	CTYPE result = {0, 0};

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return result;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_cdotu_sub(mat_size, src_x->data, x_increment, src_y->data, y_increment, &result);
	return result;

//DTYPE = double
#elif NTYPE == 1
	cblas_zdotu_sub(mat_size, src_x->data, x_increment, src_y->data, y_increment, &result);
	return result;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_udot(mat_size, src_x->data, x_increment, src_y->data, y_increment);
#endif
}

CTYPE mp_udot(UINT N, CTYPE *src_x, UINT x_inc, CTYPE *src_y, UINT y_inc)
{
	UINT i = 0;
	CTYPE dot;

	dot.re = 0;
	dot.im = 0;

#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 0; i < N; i++)
	{
		dot.re += src_x[i * x_inc].re * src_y[i * y_inc].re;
		dot.im += src_x[i * x_inc].re * src_y[i * y_inc].im;
		dot.re -= src_x[i * x_inc].im * src_y[i * y_inc].im;
		dot.im += src_x[i * x_inc].im * src_y[i * y_inc].re;
	}

	return dot;
}

/*** Swaps vector ***/
void swap(MAT *src_x, MAT *src_y)
{
	swap_inc(src_x, 1, src_y, 1);
}
void swap_inc(MAT *src_x, UINT x_inc, MAT *src_y, UINT y_inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_sswap(mat_size, src_x->data, x_inc, src_y->data, y_inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_dswap(mat_size, src_x->data, x_inc, src_y->data, y_inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_swap(mat_size, src_x->data, x_inc, src_y->data, y_inc);
#endif
}
void mp_swap(UINT N, DTYPE *src_x, UINT x_inc, DTYPE *src_y, UINT y_inc)
{
	UINT i = 0;
	DTYPE temp = 0;

#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 0; i < N; i++)
	{
		temp = src_x[i * x_inc];
		src_x[i * x_inc] = src_y[i * y_inc];
		src_y[i * y_inc] = temp;
	}
}

void cswap(CMAT *src_x, CMAT *src_y)
{
	cswap_inc(src_x, 1, src_y, 1);
}
void cswap_inc(CMAT *src_x, UINT x_inc, CMAT *src_y, UINT y_inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_cswap(mat_size, src_x->data, x_inc, src_y->data, y_inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_zswap(mat_size, src_x->data, x_inc, src_y->data, y_inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_cswap(mat_size, src_x->data, x_inc, src_y->data, y_inc);
#endif
}
void mp_cswap(UINT N, CTYPE *src_x, UINT x_inc, CTYPE *src_y, UINT y_inc)
{
	UINT i = 0;
	CTYPE temp = {0, 0};

#pragma omp parallel for shared(src_x, src_y) private(i, temp)
	for (i = 0; i < N; i++)
	{
		temp.re = src_x[i * x_inc].re;
		temp.im = src_x[i * x_inc].im;
		src_x[i * x_inc].re = src_y[i * y_inc].re;
		src_x[i * x_inc].im = src_y[i * y_inc].im;
		src_y[i * y_inc].re = temp.re;
		src_y[i * y_inc].im = temp.im;
	}
}

/*** Finds MAX_ABS_VALUE_ELEMENT's index ***/
UINT amax(MAT *src)
{
	return amax_inc(src, 1);
}
UINT amax_inc(MAT *src, UINT inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_isamax(mat_size, src->data, inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_idamax(mat_size, src->data, inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_amax(mat_size, src->data, inc);
#endif
}
UINT mp_amax(UINT N, DTYPE *src, UINT inc)
{
	UINT i = 0;
	UINT idx = 0;
	DTYPE max = src[0];

	//#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 1; i < N; i++)
	{
		if (max < (src[i * inc] < 0 ? -src[i * inc] : src[i * inc]))
		{
			idx = i;
			max = (src[i * inc] < 0 ? -src[i * inc] : src[i * inc]);
		}
	}

	return idx;
}

UINT camax(CMAT *src)
{
	return camax_inc(src, 1);
}
UINT camax_inc(CMAT *src, UINT inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_isamax(mat_size, src->data, inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_idamax(mat_size, src->data, inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_camax(mat_size, src->data, inc);
#endif
}
UINT mp_camax(UINT N, CTYPE *src, UINT inc)
{
	UINT i = 0;
	UINT idx = 0;
	DTYPE max = ABS_CTYPE(src[0]);

	//#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 1; i < N; i++)
	{
		if (max < ABS_CTYPE(src[i]))
		{
			idx = i;
			max = ABS_CTYPE(src[i]);
		}
	}

	return idx;
}

/*** Finds MIN_ABS_VALUE_ELEMENT's index ***/
UINT amin(MAT *src)
{
	return amin_inc(src, 1);
}
UINT amin_inc(MAT *src, UINT inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_isamin(mat_size, src->data, inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_idamin(mat_size, src->data, inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_amin(mat_size, src->data, inc);
#endif
}
UINT mp_amin(UINT N, DTYPE *src, UINT inc)
{
	UINT i = 0;
	UINT idx = 0;
	DTYPE min = src[0];

	//#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 1; i < N; i++)
	{
		if (min > (src[i * inc] < 0 ? -src[i * inc] : src[i * inc]))
		{
			idx = i;
			min = (src[i * inc] < 0 ? -src[i * inc] : src[i * inc]);
		}
	}

	return idx;
}

UINT camin(CMAT *src)
{
	return camin_inc(src, 1);
}
UINT camin_inc(CMAT *src, UINT inc)
{
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_isamin(mat_size, src->data, inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_idamin(mat_size, src->data, inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_camin(mat_size, src->data, inc);
#endif
}
UINT mp_camin(UINT N, CTYPE *src, UINT inc)
{
	UINT i = 0;
	UINT idx = 0;
	DTYPE min = ABS_CTYPE(src[0]);

	//#pragma omp parallel for shared(src_x, src_y) private(i)
	for (i = 1; i < N; i++)
	{
		if (min > ABS_CTYPE(src[i]))
		{
			idx = i;
			min = ABS_CTYPE(src[i]);
		}
	}

	return idx;
}

/*** Get absolute value of complex number ***/
DTYPE cabs1(CTYPE val){
#if DEBUG
	printf("%s\n", __func__);
#endif

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	return cblas_scabs1 (&val);

//DTYPE = double
#elif NTYPE == 1
	return cblas_dcabs1 (&val);
#endif

//USE_BLAS = 0 -> just c implement
#else
	return ABS_CTYPE(val);
#endif
}



DTYPE nrm2(MAT* src){
	return nrm2_inc(src, 1);
}
DTYPE nrm2_inc(MAT* src, UINT inc){
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_snrm2(mat_size, src->data, inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_dnrm2(mat_size, src->data, inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_nrm2(mat_size, src->data, inc);
#endif
}
DTYPE mp_nrm2(UINT N, DTYPE* data, UINT inc){
	UINT i=0;
	DTYPE temp = 0;

	for(i=0; i<N; i++){
		temp += data[i]*data[i];
	}

	return sqrt(temp);
}

DTYPE cnrm2(CMAT* src){
	return cnrm2_inc(src, 1);
}
DTYPE cnrm2_inc(CMAT* src, UINT inc){
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src->d0 * src->d1 * src->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_scnrm2(mat_size, src->data, inc);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_dznrm2(mat_size, src->data, inc);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_cnrm2(mat_size, src->data, inc);
#endif
}
DTYPE mp_cnrm2(UINT N, CTYPE* data, UINT inc){
	UINT i=0;
	DTYPE temp = 0;

	for(i=0; i<N; i++){
		temp += data[i].re * data[i].re;	// - data[i].im * data[i].im);
		temp += data[i].im * data[i].im;	// * 2.0);
	}

	return sqrt(temp);
}

/*** Performs rotation of points in the plane. ***/
void rot(MAT* src_x, MAT* src_y, DTYPE c, DTYPE s){
	rot_inc(src_x, 1, src_y, 1, c, s);
}
void rot_inc(MAT* src_x, UINT x_inc, MAT* src_y, UINT y_inc, DTYPE c, DTYPE s){
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_srot(mat_size, src_x->data, x_inc, src_y->data, y_inc, c, s);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_drot(mat_size, src_x->data, x_inc, src_y->data, y_inc, c, s);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_rot(mat_size, src_x->data, x_inc, src_y, y_inc, c, s);
#endif
}
void mp_rot(UINT N, DTYPE* src_x, UINT x_inc, DTYPE* src_y, UINT y_inc, DTYPE c, DTYPE s){
	UINT i=0;
	DTYPE temp_x, temp_y;

	for(i=0; i<N; i++){
		temp_x = src_x[i*x_inc];
		temp_y = src_y[i*y_inc];

		src_x[i*x_inc] = c * temp_x + s * temp_y;
		src_y[i*y_inc] = c * temp_y + s * temp_x;
	}
}

void crot(CMAT* src_x, CMAT* src_y, DTYPE c, DTYPE s){
	crot_inc(src_x, 1, src_y, 1, c, s);
}
void crot_inc(CMAT* src_x, UINT x_inc, CMAT* src_y, UINT y_inc, DTYPE c, DTYPE s){
#if DEBUG
	printf("%s\n", __func__);
#endif
	UINT mat_size = src_x->d0 * src_x->d1 * src_x->d2;

	if (mat_size == 0)
	{
		printf("Wrong MAT size!\n");
		return;
	}

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_csrot(mat_size, src_x->data, x_inc, src_y->data, y_inc, c, s);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_zdrot(mat_size, src_x->data, x_inc, src_y->data, y_inc, c, s);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_crot(mat_size, src_x->data, x_inc, src_y, y_inc, c, s);
#endif
}
void mp_crot(UINT N, CTYPE* src_x, UINT x_inc, CTYPE* src_y, UINT y_inc, DTYPE c, DTYPE s){
	UINT i=0;
	CTYPE temp_x, temp_y;

	for(i=0; i<N; i++){
		temp_x = src_x[i*x_inc];
		temp_y = src_y[i*y_inc];

		src_x[i*x_inc].re = c * temp_x.re + s * temp_y.re;
		src_x[i*x_inc].im = c * temp_x.im + s * temp_y.im;
		src_y[i*y_inc].re = c * temp_y.re + s * temp_x.re;
		src_y[i*y_inc].im = c * temp_y.im + s * temp_x.im;
	}
}

/*** Computes the parameters for a Givens rotation. ***/
void rotg(DTYPE *a, DTYPE *b, DTYPE *c, DTYPE *s){
#if DEBUG
	printf("%s\n", __func__);
#endif

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_srotg(a, b, c, s);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_drotg(a, b, c, s);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_rotg(a, b, c, s);
#endif
}
void mp_rotg(DTYPE *a, DTYPE *b, DTYPE *c, DTYPE *s)
{
	DTYPE a_ = *a;
	DTYPE b_ = *b;
	DTYPE r_, z_;

	r_ = (*c) * (*a) + (*b) * (*s);

	if ((*b) * (*c) - (*a) * (*s) != 0)
	{
		printf("Wrong data!\n");
		return;
	}

	a_ = a_ < 0 ? -a_ : a_;
	b_ = b_ < 0 ? -b_ : b_;

	if (a_ > b_)
	{
		z_ = *s;
	}
	else if (*c != 0)
	{
		z_ = 1 / (*c);
	}
	else
	{
		z_ = 1;
	}

	(*a) = r_;
	(*b) = z_;
}

void crotg(CTYPE *a, CTYPE *b, DTYPE *c, CTYPE *s){
#if DEBUG
	printf("%s\n", __func__);
#endif

#if USE_CBLAS
//DTYPE = float
#if NTYPE == 0
	cblas_crotg(a, b, c, s);
	return;

//DTYPE = double
#elif NTYPE == 1
	cblas_zrotg(a, b, c, s);
	return;
#endif

//USE_BLAS = 0 -> just c implement
#else
	return mp_crotg(a, b, c, s);
#endif
}
void mp_crotg(CTYPE *a, CTYPE *b, DTYPE *c, CTYPE *s){
	CTYPE a_ = *a;
	CTYPE b_ = *b;
	CTYPE r_, z_;
	DTYPE factor = 0;

	r_.re = (*c) * (*a).re;
	r_.re += (*b).re * (*s).re - (*b).im * (*s).im;
	r_.im = (*c) * (*a).im;
	r_.im += (*b).re * (*s).im + (*b).im * (*s).re;

	// for exception handle.... later....
	/*if ((*b) * (*c) - (*a) * (*s) != 0)
	{
		printf("Wrong data!\n");
		return;
	}*/

	if (ABS_CTYPE(a_) > ABS_CTYPE(b_))
	{
		z_ = (*s);
	}
	else if (*c != 0)
	{
		z_.re = 1 / (*c);
		z_.im = 0;
	}
	else
	{
		z_.re = 1;
		z_.im = 0;
	}

	(*a) = r_;
	(*b) = z_;
}