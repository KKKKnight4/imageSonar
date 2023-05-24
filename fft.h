#define _USE_MATH_DEFINES        //M_PI
#include <cmath>
#include <complex>
#include <iostream>
using namespace std;
/*
位反转置换
*/
void trans(complex<double>*& x,int size_x);
void fft(complex<double>*& x,int size,complex<double>*& X);
