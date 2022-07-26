
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: brent.c 10 2000-02-14 18:56:41Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

#include <math.h>

#define ITMAX 100
#define CGOLD 0.3819660
#define ZEPS 1.0e-10
#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

float brent(float ax, float bx, float cx, float (*f)(float), float tol, float *xmin)
{
        int iter;
        float a,b,etemp,fu,fv,fw,fx,p,q,r,tol1,tol2,u,v,w,x,xm;
        float d=0.0;
        float e=0.0;

        a=((ax < cx) ? ax : cx);
        b=((ax > cx) ? ax : cx);
        x=w=v=bx;
        fw=fv=fx=(*f)(x);

        for (iter=1;iter<=ITMAX;iter++)
        {
           xm=(float)0.5*(a+b);


           tol2=(float)2.0*(tol1=(float)tol*(float)fabs(x)+(float)ZEPS);

           if (fabs(x-xm) <= (tol2-0.5*(b-a)))
           {
              *xmin=x;
              return fx;
           }
           if (fabs(e) > tol1)
           {
              r=(x-w)*(fx-fv);
              q=(x-v)*(fx-fw);
              p=(x-v)*q-(x-w)*r;
              q=(float)2.0*(q-r);
              if (q > 0.0) p = -p;
              q=(float)fabs(q);
              etemp=e;
              e=d;
              if (fabs(p) >= fabs(0.5*q*etemp) || p <= q*(a-x) || p >= q*(b-x))
                 d=(float)CGOLD*(e=(x >= xm ? a-x : b-x));
              else
              {
                 d=p/q;
                 u=x+d;
                 if (u-a < tol2 || b-u < tol2)
                    d=(float)SIGN(tol1,xm-x);
              }
           }
           else
              d=(float)CGOLD*(e=(x >= xm ? a-x : b-x));

           u=(float)(fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
           fu=(*f)(u);
           if (fu <= fx)
           {
              if (u >= x) a=x; else b=x;
              SHFT(v,w,x,u)
              SHFT(fv,fw,fx,fu)
           }
           else
           {
              if (u < x) a=u; else b=u;
              if (fu <= fw || w == x)
              {
                 v=w;
                 w=u;
                 fv=fw;
                 fw=fu;
              }
              else if (fu <= fv || v == x || v == w)
              {
                 v=u;
                 fv=fu;
              }
           }
        }
        *xmin=x;
        return fx;
}

#undef ITMAX
#undef CGOLD
#undef ZEPS
#undef SIGN
