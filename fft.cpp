#define _USE_MATH_DEFINES        //M_PI
#include <cmath>
#include <complex>
#include <iostream>

#define N 2048  //window size

using namespace std;
/*
位反转置换
*/
void trans(complex<double>*& x,int size_x)
{
    int p = 0; int a, b;
    for (int i = 1; i < size_x; i *= 2)
    {
        p++;    //计算二进制位数
    }
    for (int i = 0; i < size_x; i++)
    {
        a = i;
        b = 0;
        for (int j = 0; j < p; j++)
        {
            b = (b << 1) + (a & 1);     //b存储当前下标的回文值
            a = a >> 1;
        }
        if (b > i)           //避免重复交换
        {
            complex<double> temp;
            temp = x[i];
            x[i] = x[b];
            x[b] = temp;
        }
    }
}

void fft(complex<double>*& x,int size,complex<double>*& X)
{
    complex<double> Wn[N];								//这里可以自己新建长度为size的数组
    for (int i = 0; i < size; i++)
    {
        X[i] = x[i];
        double real = cos(-2 * M_PI * i / size);
        double img = sin(-2 * M_PI * i / size);
        Wn[i] = complex<double>(real, img);                        //初始化Wn
    }
    complex<double>* p = X;
    trans(p, size);									//位反转置换
    int t;
    for (int m = 2; m <= size; m *= 2)     //小序列点数
    {
        for (int k = 0; k < size; k += m)   //小序列起始下标
        {
            for (int j = 0; j < m / 2; j++)  //小序列的DFT计算
            {
                int index1 = k + j;
                int index2 = index1 + m / 2;
                t = j * size / m;				//t是在完整序列中的下标，找到对应的旋转因子
                complex<double> temp1,temp2;
                temp2 = X[index2] * Wn[t];
                temp1 = X[index1];
                X[index1] = temp1 + temp2;   //Wn的性质
                X[index2] = temp1 - temp2;
            }
        }
    }
}
