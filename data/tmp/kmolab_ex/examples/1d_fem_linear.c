# include <math.h>
# include <stdlib.h>
# include <stdio.h>
# include <time.h>

//# include "fem1d_bvp_linear.h"
double *fem1d_bvp_linear ( int n, double a ( double x ), double c ( double x ), 
  double f ( double x ), double x[] );
double h1s_error_linear ( int n, double x[], double u[], 
  double exact_ux ( double x ) );
int i4_power ( int i, int j );
int *i4vec_zero_new ( int n );
double l1_error ( int n, double x[], double u[], 
  double exact ( double x ) );
double l2_error_linear ( int n, double x[], double u[], 
  double exact ( double x ) );
double max_error_linear ( int n, double x[], double u[], 
  double exact ( double x ) );
double r8_max ( double x, double y );
double *r8mat_solve2 ( int n, double a[], double b[], int *ierror );
double *r8mat_zero_new ( int m, int n );
double *r8vec_linspace_new ( int n, double alo, double ahi );
double *r8vec_zero_new ( int n );
void timestamp ( );

/******************************************************************************/

double *fem1d_bvp_linear ( int n, double a ( double x ), double c ( double x ), 
  double f ( double x ), double x[] )

/******************************************************************************/
/*
  Purpose:

    FEM1D_BVP_LINEAR solves a two point boundary value problem.

  Location:

    http://people.sc.fsu.edu/~jburkardt/c_src/fem1d_bvp_linear/fem1d_bvp_linear.c

  Discussion:

    The program uses the finite element method, with piecewise linear basis
    functions to solve a boundary value problem in one dimension.

    The problem is defined on the region 0 <= x <= 1.

    The following differential equation is imposed between 0 and 1:

      - d/dx a(x) du/dx + c(x) * u(x) = f(x)

    where a(x), c(x), and f(x) are given functions.

    At the boundaries, the following conditions are applied:

      u(0.0) = 0.0
      u(1.0) = 0.0

    A set of N equally spaced nodes is defined on this
    interval, with 0 = X(1) < X(2) < ... < X(N) = 1.0.

    At each node I, we associate a piecewise linear basis function V(I,X),
    which is 0 at all nodes except node I.  This implies that V(I,X) is
    everywhere 0 except that

    for X(I-1) <= X <= X(I):

      V(I,X) = ( X - X(I-1) ) / ( X(I) - X(I-1) ) 

    for X(I) <= X <= X(I+1):

      V(I,X) = ( X(I+1) - X ) / ( X(I+1) - X(I) )

    We now assume that the solution U(X) can be written as a linear
    sum of these basis functions:

      U(X) = sum ( 1 <= J <= N ) U(J) * V(J,X)

    where U(X) on the left is the function of X, but on the right,
    is meant to indicate the coefficients of the basis functions.

    To determine the coefficient U(J), we multiply the original
    differential equation by the basis function V(J,X), and use
    integration by parts, to arrive at the I-th finite element equation:

        Integral A(X) * U'(X) * V'(I,X) + C(X) * U(X) * V(I,X) dx 
      = Integral F(X) * V(I,X) dx

    We note that the functions U(X) and U'(X) can be replaced by
    the finite element form involving the linear sum of basis functions,
    but we also note that the resulting integrand will only be nonzero
    for terms where J = I - 1, I, or I + 1.

    By writing this equation for basis functions I = 2 through N - 1,
    and using the boundary conditions, we have N linear equations
    for the N unknown coefficients U(1) through U(N), which can
    be easily solved.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    18 June 2014

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of nodes.

    Input, double A ( double X ), evaluates a(x);

    Input, double C ( double X ), evaluates c(x);

    Input, double F ( double X ), evaluates f(x);

    Input, double X[N], the mesh points.

    Output, double FEM1D_BVP_LINEAR[N], the finite element coefficients, 
    which are also the value of the computed solution at the mesh points.
*/
{
# define QUAD_NUM 2

  double abscissa[QUAD_NUM] = {
    -0.577350269189625764509148780502,
    +0.577350269189625764509148780502 };
  double *amat;
  double axq;
  double *b;
  double cxq;
  int e;
  int e_num;
  double fxq;
  int i;
  int ierror;
  int j;
  int l;
  int q;
  int quad_num = QUAD_NUM;
  int r;
  double *u;
  double weight[QUAD_NUM] = { 1.0, 1.0 };
  double wq;
  double vl;
  double vlp;
  double vr;
  double vrp;
  double xl;
  double xq;
  double xr;
/*
  Zero out the matrix and right hand side.
*/
  amat = r8mat_zero_new ( n, n );
  b = r8vec_zero_new ( n );

  e_num = n - 1;

  for ( e = 0; e < e_num; e++ )
  {
    l = e;
    r = e + 1;

    xl = x[l];
    xr = x[r];

    for ( q = 0; q < quad_num; q++ )
    {
      xq = ( ( 1.0 - abscissa[q] ) * xl   
           + ( 1.0 + abscissa[q] ) * xr ) 
           /   2.0;

      wq = weight[q] * ( xr - xl ) / 2.0;

      vl =  ( xr - xq ) / ( xr - xl );
      vlp =      - 1.0  / ( xr - xl );

      vr =  ( xq - xl ) / ( xr - xl );
      vrp =  + 1.0      / ( xr - xl );

      axq = a ( xq );
      cxq = c ( xq );
      fxq = f ( xq );

      amat[l+l*n] = amat[l+l*n] + wq * ( vlp * axq * vlp + vl * cxq * vl );
      amat[l+r*n] = amat[l+r*n] + wq * ( vlp * axq * vrp + vl * cxq * vr );
      b[l]        = b[l]        + wq * ( vl * fxq );

      amat[r+l*n] = amat[r+l*n] + wq * ( vrp * axq * vlp + vr * cxq * vl );
      amat[r+r*n] = amat[r+r*n] + wq * ( vrp * axq * vrp + vr * cxq * vr );
      b[r]        = b[r]        + wq * ( vr * fxq );
    }
  }
/*
  Equation 1 is the left boundary condition, U(0.0) = 0.0;
*/
  for ( j = 0; j < n; j++ )
  {
    amat[0+j*n] = 0.0;
  }
  b[0] = 0.0;
  for ( i = 1; i < n; i++ )
  {
    b[i] = b[i] - amat[i+0*n] * b[0];
  }
  for ( i = 0; i < n; i++ )
  {
    amat[i+0*n] = 0.0;
  }
  amat[0+0*n] = 1.0;
/*
  Equation N is the right boundary condition, U(1.0) = 0.0;
*/
  for ( j = 0; j < n; j++ )
  {
    amat[n-1+j*n] = 0.0;
  }
  b[n-1] = 0.0;
  for ( i = 0; i < n - 1; i++ )
  {
    b[i] = b[i] - amat[i+(n-1)*n] * b[n-1];
  }
  for ( i = 0; i < n; i++ )
  {
    amat[i+(n-1)*n] = 0.0;
  }
  amat[n-1+(n-1)*n] = 1.0;
/*
  Solve the linear system.
*/
  u = r8mat_solve2 ( n, amat, b, &ierror );

  free ( amat );
  free ( b );

  return u;
# undef QUAD_NUM
}
/******************************************************************************/

double h1s_error_linear ( int n, double x[], double u[], 
  double exact_ux ( double x ) )

/******************************************************************************/
/*
  Purpose:

    H1S_ERROR_LINEAR estimates the seminorm error of a finite element solution.

  Discussion:

    We assume the finite element method has been used, over an interval [A,B]
    involving N nodes, with piecewise linear elements used for the basis.

    The coefficients U(1:N) have been computed, and a formula for the
    exact derivative is known.

    This function estimates the seminorm of the error:

      SEMINORM = Integral ( A <= X <= B ) ( dU(X)/dx - EXACT_UX(X) )^2 dX

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    18 June 2014

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of nodes.

    Input, double X(N), the mesh points.

    Input, double U(N), the finite element coefficients.

    Input, function EQ = EXACT_UX ( X ), returns the value of the exact
    derivative at the point X.

    Output, double H1S_ERROR_LINEAR, the estimated seminorm of 
    the error.
*/
{
# define QUAD_NUM 2

  double abscissa[QUAD_NUM] = {
    -0.577350269189625764509148780502,
    +0.577350269189625764509148780502 };
  double exq;
  double h1s;
  int i;
  int q;
  int quad_num = QUAD_NUM;
  double ul;
  double ur;
  double uxq;
  double weight[QUAD_NUM] = { 1.0, 1.0 };
  double wq;
  double xl;
  double xq;
  double xr;

  h1s = 0.0;
/*
  Integrate over each interval.
*/
  for ( i = 0; i < n - 1; i++ )
  {
    xl = x[i];
    xr = x[i+1];
    ul = u[i];
    ur = u[i+1];

    for ( q = 0; q < quad_num; q++ )
    {
      xq = ( ( 1.0 - abscissa[q] ) * xl   
           + ( 1.0 + abscissa[q] ) * xr ) 
           /   2.0;

      wq = weight[q] * ( xr - xl ) / 2.0;
/*
  The piecewise linear derivative is a constant in the interval.
*/
      uxq = ( ur - ul ) / ( xr - xl );

      exq = exact_ux ( xq );
 
      h1s = h1s + wq * pow ( uxq - exq, 2);
    }
  }
  h1s = sqrt ( h1s );

  return h1s;
# undef QUAD_NUM
}
/******************************************************************************/

int i4_power ( int i, int j )

/******************************************************************************/
/*
  Purpose:

    I4_POWER returns the value of I^J.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    23 October 2007

  Author:

    John Burkardt

  Parameters:

    Input, int I, J, the base and the power.  J should be nonnegative.

    Output, int I4_POWER, the value of I^J.
*/
{
  int k;
  int value;

  if ( j < 0 )
  {
    if ( i == 1 )
    {
      value = 1;
    }
    else if ( i == 0 )
    {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "I4_POWER - Fatal error!\n" );
      fprintf ( stderr, "  I^J requested, with I = 0 and J negative.\n" );
      exit ( 1 );
    }
    else
    {
      value = 0;
    }
  }
  else if ( j == 0 )
  {
    if ( i == 0 )
    {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "I4_POWER - Fatal error!\n" );
      fprintf ( stderr, "  I^J requested, with I = 0 and J = 0.\n" );
      exit ( 1 );
    }
    else
    {
      value = 1;
    }
  }
  else if ( j == 1 )
  {
    value = i;
  }
  else
  {
    value = 1;
    for ( k = 1; k <= j; k++ )
    {
      value = value * i;
    }
  }
  return value;
}
/******************************************************************************/

int *i4vec_zero_new ( int n )

/******************************************************************************/
/*
  Purpose:

    I4VEC_ZERO_NEW creates and zeroes an I4VEC.

  Discussion:

    An I4VEC is a vector of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    05 September 2008

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, int I4VEC_ZERO_NEW[N], a vector of zeroes.
*/
{
  int *a;
  int i;

  a = ( int * ) malloc ( n * sizeof ( int ) );

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0;
  }
  return a;
}
/******************************************************************************/

double l1_error ( int n, double x[], double u[], double exact ( double x ) )

/******************************************************************************/
/*
  Purpose:

    L1_ERROR estimates the l1 error norm of a finite element solution.

  Discussion:

    We assume the finite element method has been used, over an interval [A,B]
    involving N nodes.

    The coefficients U(1:N) have been computed, and a formula for the
    exact solution is known.

    This function estimates the little l1 norm of the error:
      L1_NORM = sum ( 1 <= I <= N ) abs ( U(i) - EXACT(X(i)) )

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    14 June 2014

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of nodes.

    Input, double X[N], the mesh points.

    Input, double U[N], the finite element coefficients.

    Input, function EQ = EXACT ( X ), returns the value of the exact
    solution at the point X.

    Output, double L1_ERROR, the little l1 norm of the error.
*/
{
  int i;
  double e1;

  e1 = 0.0;

  for ( i = 0; i < n; i++ )
  {
    e1 = e1 + fabs ( u[i] - exact ( x[i] ) );
  }
  e1 = e1 / ( double ) n;

  return e1;
}
/******************************************************************************/

double l2_error_linear ( int n, double x[], double u[], 
  double exact ( double x ) )

/******************************************************************************/
/*
  Purpose:

    L2_ERROR_LINEAR estimates the L2 error norm of a finite element solution.

  Discussion:

    We assume the finite element method has been used, over an interval [A,B]
    involving N nodes, with piecewise linear elements used for the basis.

    The coefficients U(1:N) have been computed, and a formula for the
    exact solution is known.

    This function estimates the L2 norm of the error:

      L2_NORM = Integral ( A <= X <= B ) ( U(X) - EXACT(X) )^2 dX

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    18 June 2014

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of nodes.

    Input, double X[N], the mesh points.

    Input, double U[N], the finite element coefficients.

    Input, function EQ = EXACT ( X ), returns the value of the exact
    solution at the point X.

    Output, double L2_ERROR_LINEAR, the estimated L2 norm of the error.
*/
{
# define QUAD_NUM 2

  double abscissa[QUAD_NUM] = {
    -0.577350269189625764509148780502,
    +0.577350269189625764509148780502 };
  double e2;
  double eq;
  int i;
  int q;
  int quad_num = QUAD_NUM;
  double ul;
  double ur;
  double uq;
  double weight[QUAD_NUM] = { 1.0, 1.0 };
  double wq;
  double xl;
  double xq;
  double xr;

  e2 = 0.0;
/*
  Integrate over each interval.
*/
  for ( i = 0; i < n - 1; i++ )
  {
    xl = x[i];
    xr = x[i+1];
    ul = u[i];
    ur = u[i+1];

    for ( q = 0; q < quad_num; q++ )
    {
      xq = ( ( 1.0 - abscissa[q] ) * xl   
           + ( 1.0 + abscissa[q] ) * xr ) 
           /   2.0;

      wq = weight[q] * ( xr - xl ) / 2.0;
/*
  Use the fact that U is a linear combination of piecewise linears.
*/
      uq = ( ( xr - xq      ) * ul 
           + (      xq - xl ) * ur ) 
           / ( xr      - xl );

      eq = exact ( xq );

      e2 = e2 + wq * pow ( uq - eq, 2 );
    }
  }
  e2 = sqrt ( e2 );

  return e2;
# undef QUAD_NUM
}
/******************************************************************************/

double max_error_linear ( int n, double x[], double u[], 
  double exact ( double x ) )

/******************************************************************************/
/*
  Purpose:

    MAX_ERROR_LINEAR estimates the max error norm of a finite element solution.

  Discussion:

    We assume the finite element method has been used, over an interval [A,B]
    involving N nodes, with piecewise linear elements used for the basis.

    The coefficients U(1:N) have been computed, and a formula for the
    exact solution is known.

    This function estimates the max norm of the error:

      MAX_NORM = Integral ( A <= X <= B ) max ( abs ( U(X) - EXACT(X) ) ) dX

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 July 2015

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of nodes.

    Input, double X[N], the mesh points.

    Input, double U[N], the finite element coefficients.

    Input, function EQ = EXACT ( X ), returns the value of the exact
    solution at the point X.

    Output, double MAX_ERROR_LINEAR, the estimated max norm of the error.
*/
{
  int e;
  int e_num;
  double eq;
  int l;
  int q;
  int quad_num = 8;
  int r;
  double ul;
  double ur;
  double uq;
  double value;
  double xl;
  double xq;
  double xr;

  value = 0.0;
/*
  Integrate over each interval.
*/
  e_num = n - 1;

  for ( e = 0; e < e_num; e++ )
  {
    l = e;
    xl = x[l];
    ul = u[l];

    r = e + 1;
    xr = x[r];
    ur = u[r];

    for ( q = 0; q < quad_num; q++ )
    {
      xq = ( ( double ) ( quad_num - q ) * xl   
           + ( double ) (            q ) * xr ) 
           / ( double ) ( quad_num );
/*
  Use the fact that U is a linear combination of piecewise linears.
*/
      uq = ( ( xr - xq      ) * ul 
           + (      xq - xl ) * ur ) 
           / ( xr      - xl );

      eq = exact ( xq );

      value = r8_max ( value, fabs ( uq - eq ) );
    }
  }
/*
  For completeness, check last node.
*/
  xq = x[n-1];
  uq = u[n-1];
  eq = exact ( xq );
  value = r8_max ( value, fabs ( uq - eq ) );
/*
  Integral approximation requires multiplication by interval length.
*/
  value = value * ( x[n-1] - x[0] );

  return value;
}
/******************************************************************************/

double r8_max ( double x, double y )

/******************************************************************************/
/*
  Purpose:

    R8_MAX returns the maximum of two R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    07 May 2006

  Author:

    John Burkardt

  Parameters:

    Input, double X, Y, the quantities to compare.

    Output, double R8_MAX, the maximum of X and Y.
*/
{
  double value;

  if ( y < x )
  {
    value = x;
  }
  else
  {
    value = y;
  }
  return value;
}
/******************************************************************************/

double *r8mat_solve2 ( int n, double a[], double b[], int *ierror )

/******************************************************************************/
/*
  Purpose:

    R8MAT_SOLVE2 computes the solution of an N by N linear system.

  Discussion: 							    

    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector 
    in column-major order.

    The linear system may be represented as

      A*X = B

    If the linear system is singular, but consistent, then the routine will
    still produce a solution.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    20 August 2010

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of equations.

    Input/output, double A[N*N].
    On input, A is the coefficient matrix to be inverted.
    On output, A has been overwritten.

    Input/output, double B[N].
    On input, B is the right hand side of the system.
    On output, B has been overwritten.

    Output, double R8MAT_SOLVE2[N], the solution of the linear system.

    Output, int *IERROR.
    0, no error detected.
    1, consistent singularity.
    2, inconsistent singularity.
*/
{
  double amax;
  int i;
  int imax;
  int j;
  int k;
  int *piv;
  double *x;

  *ierror = 0;

  piv = i4vec_zero_new ( n );
  x = r8vec_zero_new ( n );
/*
  Process the matrix.
*/
  for ( k = 1; k <= n; k++ )
  {
/*
  In column K:
    Seek the row IMAX with the properties that:
      IMAX has not already been used as a pivot;
      A(IMAX,K) is larger in magnitude than any other candidate.
*/
    amax = 0.0;
    imax = 0;
    for ( i = 1; i <= n; i++ )
    {
      if ( piv[i-1] == 0 )
      {
        if ( amax < fabs ( a[i-1+(k-1)*n] ) )
        {
          imax = i;
          amax = fabs ( a[i-1+(k-1)*n] );
        }
      }
    }
/*
  If you found a pivot row IMAX, then,
    eliminate the K-th entry in all rows that have not been used for pivoting.
*/
    if ( imax != 0 )
    {
      piv[imax-1] = k;
      for ( j = k+1; j <= n; j++ )
      {
        a[imax-1+(j-1)*n] = a[imax-1+(j-1)*n] / a[imax-1+(k-1)*n];
      }
      b[imax-1] = b[imax-1] / a[imax-1+(k-1)*n];
      a[imax-1+(k-1)*n] = 1.0;

      for ( i = 1; i <= n; i++ )
      {
        if ( piv[i-1] == 0 )
        {
          for ( j = k+1; j <= n; j++ )
          {
            a[i-1+(j-1)*n] = a[i-1+(j-1)*n] - a[i-1+(k-1)*n] * a[imax-1+(j-1)*n];
          }
          b[i-1] = b[i-1] - a[i-1+(k-1)*n] * b[imax-1];
          a[i-1+(k-1)*n] = 0.0;
        }
      }
    }
  }
/*
  Now, every row with nonzero PIV begins with a 1, and
  all other rows are all zero.  Begin solution.
*/
  for ( j = n; 1 <= j; j-- )
  {
    imax = 0;
    for ( k = 1; k <= n; k++ )
    {
      if ( piv[k-1] == j )
      {
        imax = k;
      }
    }

    if ( imax == 0 )
    {
      x[j-1] = 0.0;

      if ( b[j-1] == 0.0 )
      {
        *ierror = 1;
        printf ( "\n" );
        printf ( "R8MAT_SOLVE2 - Warning:\n" );
        printf ( "  Consistent singularity, equation = %d\n", j );
      }
      else
      {
        *ierror = 2;
        printf ( "\n" );
        printf ( "R8MAT_SOLVE2 - Warning:\n" );
        printf ( "  Inconsistent singularity, equation = %d\n", j );
      }
    }
    else
    {
      x[j-1] = b[imax-1];

      for ( i = 1; i <= n; i++ )
      {
        if ( i != imax )
        {
          b[i-1] = b[i-1] - a[i-1+(j-1)*n] * x[j-1];
        }
      }
    }
  }

  free ( piv );

  return x;
}
/******************************************************************************/

double *r8mat_zero_new ( int m, int n )

/******************************************************************************/
/*
  Purpose:

    R8MAT_ZERO_NEW returns a new zeroed R8MAT.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    26 September 2008

  Author:

    John Burkardt

  Parameters:

    Input, int M, N, the number of rows and columns.

    Output, double R8MAT_ZERO[M*N], the new zeroed matrix.
*/
{
  double *a;
  int i;
  int j;

  a = ( double * ) malloc ( m * n * sizeof ( double ) );

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      a[i+j*m] = 0.0;
    }
  }
  return a;
}
/******************************************************************************/

double *r8vec_linspace_new ( int n, double a, double b )

/******************************************************************************/
/*
  Purpose:

    R8VEC_LINSPACE_NEW creates a vector of linearly spaced values.

  Discussion:

    An R8VEC is a vector of R8's.

    4 points evenly spaced between 0 and 12 will yield 0, 4, 8, 12.
 
    In other words, the interval is divided into N-1 even subintervals,
    and the endpoints of intervals are used as the points.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    29 March 2011

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Input, double A, B, the first and last entries.

    Output, double R8VEC_LINSPACE_NEW[N], a vector of linearly spaced data.
*/
{
  int i;
  double *x;

  x = ( double * ) malloc ( n * sizeof ( double ) );

  if ( n == 1 )
  {
    x[0] = ( a + b ) / 2.0;
  }
  else
  {
    for ( i = 0; i < n; i++ )
    {
      x[i] = ( ( double ) ( n - 1 - i ) * a 
             + ( double ) (         i ) * b ) 
             / ( double ) ( n - 1     );
    }
  }
  return x;
}
/******************************************************************************/

double *r8vec_zero_new ( int n )

/******************************************************************************/
/*
  Purpose:

    R8VEC_ZERO_NEW creates and zeroes an R8VEC.

  Discussion:

    An R8VEC is a vector of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    25 March 2009

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, double R8VEC_ZERO_NEW[N], a vector of zeroes.
*/
{
  double *a;
  int i;

  a = ( double * ) malloc ( n * sizeof ( double ) );

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0.0;
  }
  return a;
}
/******************************************************************************/

void timestamp ( void )

/******************************************************************************/
/*
  Purpose:

    TIMESTAMP prints the current YMDHMS date as a time stamp.

  Example:

    31 May 2001 09:45:54 AM

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    24 September 2003

  Author:

    John Burkardt

  Parameters:

    None
*/
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  fprintf ( stdout, "%s\n", time_buffer );

  return;
# undef TIME_SIZE
}

//#######################
//#######################
# include <stdlib.h>
# include <stdio.h>
# include <time.h>

int main ( void );
void assemble ( double adiag[], double aleft[], double arite[], double f[], 
  double h[], int indx[], int nl, int node[], int nu, int nquad, int nsub, 
  double ul, double ur, double xn[], double xquad[] );
double ff ( double x );
void geometry ( double h[], int ibc, int indx[], int nl, int node[], int nsub, 
  int *nu, double xl, double xn[], double xquad[], double xr );
void init ( int *ibc, int *nquad, double *ul, double *ur, double *xl, 
  double *xr );
void output ( double f[], int ibc, int indx[], int nsub, int nu, double ul, 
  double ur, double xn[] );
void phi ( int il, double x, double *phii, double *phiix, double xleft, 
  double xrite );
double pp ( double x );
void prsys ( double adiag[], double aleft[], double arite[], double f[], 
  int nu );
double qq ( double x );
void solve ( double adiag[], double aleft[], double arite[], double f[], 
  int nu );
void timestamp ( );

/******************************************************************************/

int main ( )

/******************************************************************************/
/*
  Purpose:

    MAIN is the main program for FEM1D.

  Discussion:

    FEM1D solves a one dimensional ODE using the finite element method.

    The differential equation solved is

      - d/dX (P dU/dX) + Q U  =  F

    The finite-element method uses piecewise linear basis functions.

    Here U is an unknown scalar function of X defined on the
    interval [XL,XR], and P, Q and F are given functions of X.

    The values of U or U' at XL and XR are also specified.

    The interval [XL,XR] is "meshed" with NSUB+1 points,

    XN(0) = XL, XN(1)=XL+H, XN(2)=XL+2*H, ..., XN(NSUB)=XR.

    This creates NSUB subintervals, with interval number 1
    having endpoints XN(0) and XN(1), and so on up to interval
    NSUB, which has endpoints XN(NSUB-1) and XN(NSUB).

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    double ADIAG(NU), the "diagonal" coefficients.  That is, ADIAG(I) is
    the coefficient of the I-th unknown in the I-th equation.

    double ALEFT(NU), the "left hand" coefficients.  That is, ALEFT(I) 
    is the coefficient of the (I-1)-th unknown in the I-th equation.
    There is no value in ALEFT(1), since the first equation
    does not refer to a "0-th" unknown.

    double ARITE(NU).
    ARITE(I) is the "right hand" coefficient of the I-th
    equation in the linear system.  ARITE(I) is the coefficient
    of the (I+1)-th unknown in the I-th equation.  There is
    no value in ARITE(NU) because the NU-th equation does not
    refer to an "NU+1"-th unknown.

    double F(NU).
    ASSEMBLE stores into F the right hand side of the linear
    equations.
    SOLVE replaces those values of F by the solution of the
    linear equations.

    double H(NSUB)
    H(I) is the length of subinterval I.  This code uses
    equal spacing for all the subintervals.

    int IBC.
    IBC declares what the boundary conditions are.
    1, at the left endpoint, U has the value UL,
       at the right endpoint, U' has the value UR.
    2, at the left endpoint, U' has the value UL,
       at the right endpoint, U has the value UR.
    3, at the left endpoint, U has the value UL,
       and at the right endpoint, U has the value UR.
    4, at the left endpoint, U' has the value UL,
       at the right endpoint U' has the value UR.

    int INDX[NSUB+1].
    For a node I, INDX(I) is the index of the unknown
    associated with node I.
    If INDX(I) is equal to -1, then no unknown is associated
    with the node, because a boundary condition fixing the
    value of U has been applied at the node instead.
    Unknowns are numbered beginning with 1.
    If IBC is 2 or 4, then there is an unknown value of U
    at node 0, which will be unknown number 1.  Otherwise,
    unknown number 1 will be associated with node 1.
    If IBC is 1 or 4, then there is an unknown value of U
    at node NSUB, which will be unknown NSUB or NSUB+1,
    depending on whether there was an unknown at node 0.

    int NL.
    The number of basis functions used in a single
    subinterval.  (NL-1) is the degree of the polynomials
    used.  For this code, NL is fixed at 2, meaning that
    piecewise linear functions are used as the basis.

    int NODE[NL*NSUB].
    For each subinterval I:
    NODE[0+I*2] is the number of the left node, and
    NODE[1+I*2] is the number of the right node.

    int NQUAD.
    The number of quadrature points used in a subinterval.
    This code uses NQUAD = 1.

    int NSUB.
    The number of subintervals into which the interval [XL,XR] is broken.

    int NU.
    NU is the number of unknowns in the linear system.
    Depending on the value of IBC, there will be NSUB-1,
    NSUB, or NSUB+1 unknown values, which are the coefficients
    of basis functions.

    double UL.
    If IBC is 1 or 3, UL is the value that U is required
    to have at X = XL.
    If IBC is 2 or 4, UL is the value that U' is required
    to have at X = XL.

    double UR.
    If IBC is 2 or 3, UR is the value that U is required
    to have at X = XR.
    If IBC is 1 or 4, UR is the value that U' is required
    to have at X = XR.

    double XL.
    XL is the left endpoint of the interval over which the
    differential equation is being solved.

    double XN(0:NSUB).
    XN(I) is the location of the I-th node.  XN(0) is XL,
    and XN(NSUB) is XR.

    double XQUAD(NSUB)
    XQUAD(I) is the location of the single quadrature point
    in interval I.

    double XR.
    XR is the right endpoint of the interval over which the
    differential equation is being solved.
*/
{
# define NSUB 5
# define NL 2

  double adiag[NSUB+1];
  double aleft[NSUB+1];
  double arite[NSUB+1];
  double f[NSUB+1];
  double h[NSUB];
  int ibc;
  int indx[NSUB+1];
  int node[NL*NSUB];
  int nquad;
  int nu;
  double ul;
  double ur;
  double xl;
  double xn[NSUB+1];
  double xquad[NSUB];
  double xr;

  timestamp ( );

  printf ( "\n" );
  printf ( "FEM1D\n" );
  printf ( "  C version\n" );
  printf ( "\n" );
  printf ( "  Solve the two-point boundary value problem\n" );
  printf ( "\n" );
  printf ( "  - d/dX (P dU/dX) + Q U  =  F\n" );
  printf ( "\n" );
  printf ( "  on the interval [XL,XR], specifying\n" );
  printf ( "  the value of U or U' at each end.\n" );
  printf ( "\n" );
  printf ( "  The interval [XL,XR] is broken into NSUB = %d subintervals\n", NSUB );
  printf ( "  Number of basis functions per element is NL = %d\n", NL );
/*
  Initialize the data.
*/
  init ( &ibc, &nquad, &ul, &ur, &xl, &xr );
/*
  Compute the geometric quantities.
*/
  geometry ( h, ibc, indx, NL, node, NSUB, &nu, xl, xn, xquad, xr );
/*
  Assemble the linear system.
*/
  assemble ( adiag, aleft, arite, f, h, indx, NL, node, nu, nquad, 
    NSUB, ul, ur, xn, xquad );
/*
  Print out the linear system.
*/
  prsys ( adiag, aleft, arite, f, nu );
/*
  Solve the linear system.
*/
  solve ( adiag, aleft, arite, f, nu );
/*
  Print out the solution.
*/
  output ( f, ibc, indx, NSUB, nu, ul, ur, xn );
/*
  Terminate.
*/
  printf ( "\n" );
  printf ( "FEM1D:\n" );
  printf ( "  Normal end of execution.\n" );

  printf ( "\n" );
  timestamp ( );

  return 0;
# undef NL
# undef NSUB
}
/******************************************************************************/

void assemble ( double adiag[], double aleft[], double arite[], double f[], 
  double h[], int indx[], int nl, int node[], int nu, int nquad, int nsub, 
  double ul, double ur, double xn[], double xquad[] )

/******************************************************************************/
/*
  Purpose:

    ASSEMBLE assembles the matrix and right-hand-side of the linear system.

  Discussion:

    The linear system has the form:

      K * C = F

    that is to be solved for the coefficients C.

    Numerical integration is used to compute the entries of K and F.

    Note that a 1 point quadrature rule, which is sometimes used to
    assemble the matrix and right hand side, is just barely accurate
    enough for simple problems.  If you want better results, you
    should use a quadrature rule that is more accurate.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    Output, double ADIAG(NU), the "diagonal" coefficients.  That is, 
    ADIAG(I) is the coefficient of the I-th unknown in the I-th equation.

    Output, double ALEFT(NU), the "left hand" coefficients.  That is, 
    ALEFT(I) is the coefficient of the (I-1)-th unknown in the I-th equation.
    There is no value in ALEFT(1), since the first equation
    does not refer to a "0-th" unknown.

    Output, double ARITE(NU).
    ARITE(I) is the "right hand" coefficient of the I-th
    equation in the linear system.  ARITE(I) is the coefficient
    of the (I+1)-th unknown in the I-th equation.  There is
    no value in ARITE(NU) because the NU-th equation does not
    refer to an "NU+1"-th unknown.

    Output, double F(NU).
    ASSEMBLE stores into F the right hand side of the linear
    equations.
    SOLVE replaces those values of F by the solution of the
    linear equations.

    Input, double H(NSUB)
    H(I) is the length of subinterval I.  This code uses
    equal spacing for all the subintervals.

    Input, int INDX[NSUB+1].
    For a node I, INDX(I) is the index of the unknown
    associated with node I.
    If INDX(I) is equal to -1, then no unknown is associated
    with the node, because a boundary condition fixing the
    value of U has been applied at the node instead.
    Unknowns are numbered beginning with 1.
    If IBC is 2 or 4, then there is an unknown value of U
    at node 0, which will be unknown number 1.  Otherwise,
    unknown number 1 will be associated with node 1.
    If IBC is 1 or 4, then there is an unknown value of U
    at node NSUB, which will be unknown NSUB or NSUB+1,
    depending on whether there was an unknown at node 0.

    Input, int NL.
    The number of basis functions used in a single
    subinterval.  (NL-1) is the degree of the polynomials
    used.  For this code, NL is fixed at 2, meaning that
    piecewise linear functions are used as the basis.

    Input, int NODE[NL*NSUB].
    For each subinterval I:
    NODE[0+I*2] is the number of the left node, and
    NODE[1+I*2] is the number of the right node.

    Input, int NU.
    NU is the number of unknowns in the linear system.
    Depending on the value of IBC, there will be NSUB-1,
    NSUB, or NSUB+1 unknown values, which are the coefficients
    of basis functions.

    Input, int NQUAD.
    The number of quadrature points used in a subinterval.
    This code uses NQUAD = 1.

    Input, int NSUB.
    The number of subintervals into which the interval [XL,XR] is broken.

    Input, double UL.
    If IBC is 1 or 3, UL is the value that U is required
    to have at X = XL.
    If IBC is 2 or 4, UL is the value that U' is required
    to have at X = XL.

    Input, double UR.
    If IBC is 2 or 3, UR is the value that U is required
    to have at X = XR.
    If IBC is 1 or 4, UR is the value that U' is required
    to have at X = XR.

    Input, double XL.
    XL is the left endpoint of the interval over which the
    differential equation is being solved.

    Input, double XR.
    XR is the right endpoint of the interval over which the
    differential equation is being solved.
*/
{
  double aij;
  double he;
  int i;
  int ie;
  int ig;
  int il;
  int iq;
  int iu;
  int jg;
  int jl;
  int ju;
  double phii;
  double phiix;
  double phij;
  double phijx;
  double x;
  double xleft;
  double xquade;
  double xrite;
/*
  Zero out the arrays that hold the coefficients of the matrix
  and the right hand side.
*/
  for ( i = 0; i < nu; i++ )
  {
    f[i] = 0.0;
  }
  for ( i = 0; i < nu; i++ )
  {
    adiag[i] = 0.0;
  }
  for ( i = 0; i < nu; i++ )
  {
    aleft[i] = 0.0;
  }
  for ( i = 0; i < nu; i++ )
  {
    arite[i] = 0.0;
  }
/*
  For interval number IE,
*/
  for ( ie = 0; ie < nsub; ie++ )
  {
    he = h[ie];
    xleft = xn[node[0+ie*2]];
    xrite = xn[node[1+ie*2]];
/*
  consider each quadrature point IQ,
*/
    for ( iq = 0; iq < nquad; iq++ )
    {
      xquade = xquad[ie];
/*
  and evaluate the integrals associated with the basis functions
  for the left, and for the right nodes.
*/
      for ( il = 1; il <= nl; il++ )
      {
        ig = node[il-1+ie*2];
        iu = indx[ig] - 1;

        if ( 0 <= iu )
        {
          phi ( il, xquade, &phii, &phiix, xleft, xrite );
          f[iu] = f[iu] + he * ff ( xquade ) * phii;
/*
  Take care of boundary nodes at which U' was specified.
*/
          if ( ig == 0 )
          {
            x = 0.0;
            f[iu] = f[iu] - pp ( x ) * ul;
          }
          else if ( ig == nsub )
          {
            x = 1.0;
            f[iu] = f[iu] + pp ( x ) * ur;
          }
/*
  Evaluate the integrals that take a product of the basis
  function times itself, or times the other basis function
  that is nonzero in this interval.
*/
          for ( jl = 1; jl <= nl; jl++ )
          {
            jg = node[jl-1+ie*2];
            ju = indx[jg] - 1;

            phi ( jl, xquade, &phij, &phijx, xleft, xrite );

            aij = he * ( pp ( xquade ) * phiix * phijx 
                       + qq ( xquade ) * phii  * phij   );
/*
  If there is no variable associated with the node, then it's
  a specified boundary value, so we multiply the coefficient
  times the specified boundary value and subtract it from the
  right hand side.
*/
            if ( ju < 0 )
            {
              if ( jg == 0 )
              {
                f[iu] = f[iu] - aij * ul;
              }
              else if ( jg == nsub )
              {               
                f[iu] = f[iu] - aij * ur;
              }
            }
/*
  Otherwise, we add the coefficient we've just computed to the
  diagonal, or left or right entries of row IU of the matrix.
*/
            else
            {
              if ( iu == ju )
              {
                adiag[iu] = adiag[iu] + aij;
              }
              else if ( ju < iu )
              {
                aleft[iu] = aleft[iu] + aij;
              }
              else
              {
                arite[iu] = arite[iu] + aij;
              }
            }
          }
        }
      }
    }
  }
  return;
}
/******************************************************************************/

double ff ( double x )

/******************************************************************************/
/*
  Purpose:

    FF evaluates the right hand side function.

  Discussion:

    This routine evaluates the function F(X) in the differential equation.

      -d/dx (p du/dx) + q u  =  f

    at the point X.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    John Burkardt

  Parameters:

    Input, double X, the argument of the function.

    Output, double FF, the value of the function.
*/
{
  double value;

  value = 0.0;

  return value;
}
/******************************************************************************/

void geometry ( double h[], int ibc, int indx[], int nl, int node[], int nsub, 
  int *nu, double xl, double xn[], double xquad[], double xr )

/******************************************************************************/
/*
  Purpose: 

    GEOMETRY sets up the geometry for the interval [XL,XR].

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    Output, double H(NSUB)
    H(I) is the length of subinterval I.  This code uses
    equal spacing for all the subintervals.

    Input, int IBC.
    IBC declares what the boundary conditions are.
    1, at the left endpoint, U has the value UL,
       at the right endpoint, U' has the value UR.
    2, at the left endpoint, U' has the value UL,
       at the right endpoint, U has the value UR.
    3, at the left endpoint, U has the value UL,
       and at the right endpoint, U has the value UR.
    4, at the left endpoint, U' has the value UL,
       at the right endpoint U' has the value UR.

    Output, int INDX[NSUB+1].
    For a node I, INDX(I) is the index of the unknown
    associated with node I.
    If INDX(I) is equal to -1, then no unknown is associated
    with the node, because a boundary condition fixing the
    value of U has been applied at the node instead.
    Unknowns are numbered beginning with 1.
    If IBC is 2 or 4, then there is an unknown value of U
    at node 0, which will be unknown number 1.  Otherwise,
    unknown number 1 will be associated with node 1.
    If IBC is 1 or 4, then there is an unknown value of U
    at node NSUB, which will be unknown NSUB or NSUB+1,
    depending on whether there was an unknown at node 0.

    Input, int NL.
    The number of basis functions used in a single
    subinterval.  (NL-1) is the degree of the polynomials
    used.  For this code, NL is fixed at 2, meaning that
    piecewise linear functions are used as the basis.

    Output, int NODE[NL*NSUB].
    For each subinterval I:
    NODE[0+I*2] is the number of the left node, and
    NODE[1+I*2] is the number of the right node.

    Input, int NSUB.
    The number of subintervals into which the interval [XL,XR] is broken.

    Output, int *NU.
    NU is the number of unknowns in the linear system.
    Depending on the value of IBC, there will be NSUB-1,
    NSUB, or NSUB+1 unknown values, which are the coefficients
    of basis functions.

    Input, double XL.
    XL is the left endpoint of the interval over which the
    differential equation is being solved.

    Output, double XN(0:NSUB).
    XN(I) is the location of the I-th node.  XN(0) is XL,
    and XN(NSUB) is XR.

    Output, double XQUAD(NSUB)
    XQUAD(I) is the location of the single quadrature point
    in interval I.

    Input, double XR.
    XR is the right endpoint of the interval over which the
    differential equation is being solved.
*/
{
  int i;
/*
  Set the value of XN, the locations of the nodes.
*/
  printf ( "\n" );
  printf ( "  Node      Location\n" );
  printf ( "\n" );
  for ( i = 0; i <= nsub; i++ )
  {
    xn[i]  =  ( ( double ) ( nsub - i ) * xl 
              + ( double )          i   * xr ) 
              / ( double ) ( nsub );
    printf ( "  %8d  %14f \n", i, xn[i] );
  }
/*
  Set the lengths of each subinterval.
*/
  printf ( "\n" );
  printf ( "Subint    Length\n" );
  printf ( "\n" );
  for ( i = 0; i < nsub; i++ )
  {
    h[i] = xn[i+1] - xn[i];
    printf ( "  %8d  %14f\n", i+1, h[i] );
  }
/*
  Set the quadrature points, each of which is the midpoint
  of its subinterval.
*/
  printf ( "\n" );
  printf ( "Subint    Quadrature point\n" );
  printf ( "\n" );
  for ( i = 0; i < nsub; i++ )
  {
    xquad[i] = 0.5 * ( xn[i] + xn[i+1] );
    printf ( "  %8d  %14f\n", i+1, xquad[i] );
  }
/*
  Set the value of NODE, which records, for each interval,
  the node numbers at the left and right.
*/
  printf ( "\n" );
  printf ( "Subint  Left Node  Right Node\n" );
  printf ( "\n" );
  for ( i = 0; i < nsub; i++ )
  {
    node[0+i*2] = i;
    node[1+i*2] = i + 1;
    printf ( "  %8d  %8d  %8d\n", i+1, node[0+i*2], node[1+i*2] );
  }
/*
  Starting with node 0, see if an unknown is associated with
  the node.  If so, give it an index.
*/
  *nu = 0;
/*
  Handle first node.
*/
  i = 0;
  if ( ibc == 1 || ibc == 3 )
  {
    indx[i] = -1;
  }
  else
  {
    *nu = *nu + 1;
    indx[i] = *nu;
  }
/*
  Handle nodes 1 through nsub-1
*/
  for ( i = 1; i < nsub; i++ )
  {
    *nu = *nu + 1;
    indx[i] = *nu;
  }
/*
  Handle the last node.
/*/
  i = nsub;

  if ( ibc == 2 || ibc == 3 )
  {
    indx[i] = -1;
  }
  else
  {
    *nu = *nu + 1;
    indx[i] = *nu;
  }

  printf ( "\n" );
  printf ( "  Number of unknowns NU = %8d\n", *nu );
  printf ( "\n" );
  printf ( "  Node  Unknown\n" );
  printf ( "\n" );
  for ( i = 0; i <= nsub; i++ )
  {
    printf ( "  %8d  %8d\n", i, indx[i] );
  }

  return;
}
/******************************************************************************/

void init ( int *ibc, int *nquad, double *ul, double *ur, double *xl, 
  double *xr )

/******************************************************************************/
/*
  Purpose: 

    INIT assigns values to variables which define the problem.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    Output, int *IBC.
    IBC declares what the boundary conditions are.
    1, at the left endpoint, U has the value UL,
       at the right endpoint, U' has the value UR.
    2, at the left endpoint, U' has the value UL,
       at the right endpoint, U has the value UR.
    3, at the left endpoint, U has the value UL,
       and at the right endpoint, U has the value UR.
    4, at the left endpoint, U' has the value UL,
       at the right endpoint U' has the value UR.

    Output, int *NQUAD.
    The number of quadrature points used in a subinterval.
    This code uses NQUAD = 1.

    Output, double *UL.
    If IBC is 1 or 3, UL is the value that U is required
    to have at X = XL.
    If IBC is 2 or 4, UL is the value that U' is required
    to have at X = XL.

    Output, double *UR.
    If IBC is 2 or 3, UR is the value that U is required
    to have at X = XR.
    If IBC is 1 or 4, UR is the value that U' is required
    to have at X = XR.

    Output, double *XL.
    XL is the left endpoint of the interval over which the
    differential equation is being solved.

    Output, double *XR.
    XR is the right endpoint of the interval over which the
    differential equation is being solved.
*/
{
/*
  IBC declares what the boundary conditions are.
*/
  *ibc = 1;
/*
  NQUAD is the number of quadrature points per subinterval.
  The program as currently written cannot handle any value for
  NQUAD except 1.
*/
  *nquad = 1;
/*
  Set the values of U or U' at the endpoints.
*/
  *ul = 0.0;
  *ur = 1.0;
/*
  Define the location of the endpoints of the interval.
*/
  *xl = 0.0;
  *xr = 1.0;
/*
  Print out the values that have been set.
*/
  printf ( "\n" );
  printf ( "  The equation is to be solved for\n" );
  printf ( "  X greater than XL = %f\n", *xl );
  printf ( "  and less than XR = %f\n", *xr );
  printf ( "\n" );
  printf ( "  The boundary conditions are:\n" );
  printf ( "\n" );

  if ( *ibc == 1 || *ibc == 3 )
  {
    printf ( "  At X = XL, U = %f\n", *ul );
  }
  else
  {
    printf ( "  At X = XL, U' = %f\n", *ul );
  }

  if ( *ibc == 2 || *ibc == 3 )
  {
    printf ( "  At X = XR, U = %f\n", *ur );
  }
  else
  {
    printf ( "  At X = XR, U' = %f\n", *ur );
  }

  printf ( "\n" );
  printf ( "  Number of quadrature points per element is %d\n", *nquad );

  return;
}
/******************************************************************************/

void output ( double f[], int ibc, int indx[], int nsub, int nu, double ul, 
  double ur, double xn[] )

/******************************************************************************/
/*
  Purpose:

    OUTPUT prints out the computed solution.

  Discussion:

    We simply print out the solution vector F, except that, for
    certain boundary conditions, we are going to have to get the
    value of the solution at XL or XR by using the specified
    boundary value.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    Input, double F(NU).
    ASSEMBLE stores into F the right hand side of the linear
    equations.
    SOLVE replaces those values of F by the solution of the
    linear equations.

    Input, int IBC.
    IBC declares what the boundary conditions are.
    1, at the left endpoint, U has the value UL,
       at the right endpoint, U' has the value UR.
    2, at the left endpoint, U' has the value UL,
       at the right endpoint, U has the value UR.
    3, at the left endpoint, U has the value UL,
       and at the right endpoint, U has the value UR.
    4, at the left endpoint, U' has the value UL,
       at the right endpoint U' has the value UR.

    Input, int INDX[NSUB+1].
    For a node I, INDX(I) is the index of the unknown
    associated with node I.
    If INDX(I) is equal to -1, then no unknown is associated
    with the node, because a boundary condition fixing the
    value of U has been applied at the node instead.
    Unknowns are numbered beginning with 1.
    If IBC is 2 or 4, then there is an unknown value of U
    at node 0, which will be unknown number 1.  Otherwise,
    unknown number 1 will be associated with node 1.
    If IBC is 1 or 4, then there is an unknown value of U
    at node NSUB, which will be unknown NSUB or NSUB+1,
    depending on whether there was an unknown at node 0.

    Input, int NSUB.
    The number of subintervals into which the interval [XL,XR] is broken.

    Input, int NU.
    NU is the number of unknowns in the linear system.
    Depending on the value of IBC, there will be NSUB-1,
    NSUB, or NSUB+1 unknown values, which are the coefficients
    of basis functions.

    Input, double UL.
    If IBC is 1 or 3, UL is the value that U is required
    to have at X = XL.
    If IBC is 2 or 4, UL is the value that U' is required
    to have at X = XL.

    Input, double UR.
    If IBC is 2 or 3, UR is the value that U is required
    to have at X = XR.
    If IBC is 1 or 4, UR is the value that U' is required
    to have at X = XR.

    Input, double XN(0:NSUB).
    XN(I) is the location of the I-th node.  XN(0) is XL,
    and XN(NSUB) is XR.
*/
{
  int i;
  double u;

  printf ( "\n" );
  printf ( "  Computed solution coefficients:\n" );
  printf ( "\n" );
  printf ( "  Node    X(I)        U(X(I))\n" );
  printf ( "\n" );

  for ( i = 0; i <= nsub; i++ )
  {
/*
  If we're at the first node, check the boundary condition.
*/
    if ( i == 0 )
    {
      if ( ibc == 1 || ibc == 3 )
      {
        u = ul;
      }
      else
      {
        u = f[indx[i]-1];
      }
    }
/*
  If we're at the last node, check the boundary condition.
*/
    else if ( i == nsub )
    {
      if ( ibc == 2 || ibc == 3 )
      {
        u = ur;
      }
      else
      {
        u = f[indx[i]-1];
      }
    }
/*
  Any other node, we're sure the value is stored in F.
*/
    else
    {
      u = f[indx[i]-1];
    }

    printf ( "  %8d  %8f  %14f\n", i, xn[i], u );
  }

  return;
}
/******************************************************************************/

void phi ( int il, double x, double *phii, double *phiix, double xleft, 
  double xrite )

/******************************************************************************/
/*
  Purpose:

    PHI evaluates a linear basis function and its derivative.

  Discussion:

    The evaluation is done at a point X in an interval [XLEFT,XRITE].

    In this interval, there are just two nonzero basis functions.
    The first basis function is a line which is 1 at the left
    endpoint and 0 at the right.  The second basis function is 0 at
    the left endpoint and 1 at the right.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    Input, int IL, the index of the basis function.
    1, the function which is 1 at XLEFT and 0 at XRITE.
    2, the function which is 0 at XLEFT and 1 at XRITE.

    Input, double X, the evaluation point.

    Output, double *PHII, *PHIIX, the value of the
    basis function and its derivative at X.

    Input, double XLEFT, XRITE, the left and right
    endpoints of the interval.
*/
{
  if ( xleft <= x && x <= xrite )
  {
    if ( il == 1 )
    {
      *phii = ( xrite - x ) / ( xrite - xleft );
      *phiix =         -1.0 / ( xrite - xleft );
    }
    else
    {
      *phii = ( x - xleft ) / ( xrite - xleft );
      *phiix = 1.0          / ( xrite - xleft );
    }
  }
/*
  If X is outside of the interval, just set everything to 0.
*/
  else
  {
    *phii  = 0.0;
    *phiix = 0.0;
  }

  return;
}
/******************************************************************************/

double pp ( double x )

/******************************************************************************/
/*
  Purpose:

    PP evaluates the function P in the differential equation.

  Discussion:

    The function P appears in the differential equation as;

      - d/dx (p du/dx) + q u  =  f

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    John Burkardt

  Parameters:

    Input, double X, the argument of the function.

    Output, double PP, the value of the function.
*/
{
  double value;

  value = 1.0;

  return value;
}
/******************************************************************************/

void prsys ( double adiag[], double aleft[], double arite[], double f[], 
  int nu )

/******************************************************************************/
/*
  Purpose:

    PRSYS prints out the tridiagonal linear system.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameter:

    Input, double ADIAG(NU), the "diagonal" coefficients.  That is, 
    ADIAG(I) is the coefficient of the I-th unknown in the I-th equation.

    Input, double ALEFT(NU), the "left hand" coefficients.  That is, ALEFT(I) 
    is the coefficient of the (I-1)-th unknown in the I-th equation.
    There is no value in ALEFT(1), since the first equation
    does not refer to a "0-th" unknown.

    Input, double ARITE(NU).
    ARITE(I) is the "right hand" coefficient of the I-th
    equation in the linear system.  ARITE(I) is the coefficient
    of the (I+1)-th unknown in the I-th equation.  There is
    no value in ARITE(NU) because the NU-th equation does not
    refer to an "NU+1"-th unknown.

    Input, double F(NU).
    ASSEMBLE stores into F the right hand side of the linear
    equations.
    SOLVE replaces those values of F by the solution of the
    linear equations.

    Input, int NU.
    NU is the number of unknowns in the linear system.
    Depending on the value of IBC, there will be NSUB-1,
    NSUB, or NSUB+1 unknown values, which are the coefficients
    of basis functions.
*/
{
  int i;

  printf ( "\n" );
  printf ( "Printout of tridiagonal linear system:\n" );
  printf ( "\n" );
  printf ( "Equation  ALEFT  ADIAG  ARITE  RHS\n" );
  printf ( "\n" );

  for ( i = 0; i < nu; i++ )
  {
    printf ( "  %8d  %14f  %14f  %14f  %14f\n",
      i + 1, aleft[i], adiag[i], arite[i], f[i] );
  }

  return;
}
/******************************************************************************/

double qq ( double x )

/******************************************************************************/
/*
  Purpose: 

    QQ evaluates the function Q in the differential equation.

  Discussion:

    The function Q appears in the differential equation as:

      - d/dx (p du/dx) + q u  =  f

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    John Burkardt

  Parameters:

    Input, double X, the argument of the function.

    Output, double QQ, the value of the function.
*/
{
  double value;

  value = 0.0;

  return value;
}
/******************************************************************************/

void solve ( double adiag[], double aleft[], double arite[], double f[], 
  int nu )

/******************************************************************************/
/*
  Purpose: 

    SOLVE solves a tridiagonal matrix system of the form A*x = b.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    29 May 2009

  Author:

    C version by John Burkardt

  Parameters:

    Input/output, double ADIAG(NU), ALEFT(NU), ARITE(NU).
    On input, ADIAG, ALEFT, and ARITE contain the diagonal,
    left and right entries of the equations.
    On output, ADIAG and ARITE have been changed in order
    to compute the solution.
    Note that for the first equation, there is no ALEFT
    coefficient, and for the last, there is no ARITE.
    So there is no need to store a value in ALEFT(1), nor
    in ARITE(NU).

    Input/output, double F(NU).
    On input, F contains the right hand side of the linear
    system to be solved.
    On output, F contains the solution of the linear system.

    Input, int NU, the number of equations to be solved.
*/
{
  int i;
/*
  Carry out Gauss elimination on the matrix, saving information
  needed for the backsolve.
*/
  arite[0] = arite[0] / adiag[0];

  for ( i = 1; i < nu - 1; i++ )
  {
    adiag[i] = adiag[i] - aleft[i] * arite[i-1];
    arite[i] = arite[i] / adiag[i];
  }
  adiag[nu-1] = adiag[nu-1] - aleft[nu-1] * arite[nu-2];
/*
  Carry out the same elimination steps on F that were done to the
  matrix.
*/
  f[0] = f[0] / adiag[0];
  for ( i = 1; i < nu; i++ )
  {
    f[i] = ( f[i] - aleft[i] * f[i-1] ) / adiag[i];
  }
/*
  And now carry out the steps of "back substitution".
*/
  for ( i = nu - 2; 0 <= i; i-- )
  {
    f[i] = f[i] - arite[i] * f[i+1];
  }

  return;
}
/******************************************************************************/
