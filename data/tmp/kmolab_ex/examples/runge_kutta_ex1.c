#include<stdio.h>
#include <math.h>
#include<conio.h>
//dy/dx = y - x
#define F(x,y)  (y)-(x)
void main()
{
  double y0,x0,y1,n,h,f,f1,k1,k2;
  int j;
  x0 = 0;
  y0 = 2;
  h = 0.05;
  n = 1;
  for(; x0<n; x0=x0+h)
  {
    f=F(x0,y0);
    k1 = h * f;
    f1 = F(x0+h,y0+k1);
    k2 = h * f1;
    y1 = y0 + ( k1 + k2)/2;
    printf("\n\n  k1 = %.4lf ",k1);
    printf("\n\n  k2 = %.4lf ",k2);
    printf("\n\n  y(%.4lf) = %.3lf ",x0+h,y1);
    y0=y1;
  }
}
/*
____________________________________

         OUT PUT
____________________________________


Enter the value of x0: 0

Enter the value of y0: 2

Enter the value of h: 0.05

Enter the value of last point: 0.1


     k1 = 0.1000

     k2 = 0.1025

     y(0.0500) = 2.101

     k1 = 0.1026

     k2 = 0.1052

     y(0.1000) = 2.205
*/