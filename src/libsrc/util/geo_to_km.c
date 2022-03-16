#include <stdio.h>
#include <math.h>

#include "geo.h"

/*
Calculations are based upon the reference spheroid of 1968 and
are defined by the major radius (RAD) and the flattening (FL).
*/
#define semi_major 6378.160
#define semi_minor 6356.775   

int geo_to_km(double lat1,double lon1,double lat2,double lon2,double* dist,double* azm) 
{

/*
=====================================================================
 PURPOSE:  To compute the distance and azimuth between locations.
=====================================================================
 INPUT ARGUMENTS:
    lat1:     Event latitude in decimal degrees, North positive. [r]
    lon1:     Event longitude, East positive. [r]
    lat2:     station latitude. [r]
    lon2:     station longitude. [r]
=====================================================================
 OUTPUT ARGUMENTS:
    DIST:    epicentral distance in km. [r]
    AZM:      azimuth in degrees. [r]
=====================================================================
*/
/*
Calculations are based upon the reference spheroid of 1968 and
are defined by the major radius (RAD) and the flattening (FL).
*/

      const double a = semi_major;
      const double b = semi_minor;   
      double torad, todeg;
      double aa, bb, cc, dd, top, bottom, lambda12, az, temp;
      double v1, v2;
      double fl, e2, eps, eps0;
      double b0, x2, y2, z2, z1, u1p, u2p, xdist;
      double lat1rad, lat2rad, lon1rad, lon2rad;
      double coslon1, sinlon1, coslon2, sinlon2;
      double coslat1, sinlat1, coslat2, sinlat2;
      double tanlat1, tanlat2, cosazm, sinazm;

      double c0, c2, c4, c6;

      double c00=1.0, c01=0.25, c02=-0.046875, c03=0.01953125;
      double c21=-0.125, c22=0.03125, c23=-0.014648438;
      double c42=-0.00390625, c43=0.0029296875;
      double c63=-0.0003255208;

/*  Check for special conditions */
     if( lat1 == lat2 && lon1 == lon2 ) {
         *azm = 0.0;
         *dist= 0.0;
         return(1);
     }
/* - Initialize.             */

      torad = PI / 180.0;
      todeg = 1.0 / torad;
      fl = ( a - b ) / a;
      e2 = 2.0*fl - fl*fl;
      eps = e2 / ( 1.0 - e2);
/*
* - Convert event location to radians.
*   (Equations are unstable for latidudes of exactly 0 degrees.)
*/
      temp=lat1;
      if(temp == 0.) temp=1.0e-08;
      lat1rad=torad*temp;
      lon1rad=torad*lon1;

      temp=lat2;
      if(temp == 0.) temp=1.0e-08;
      lat2rad=torad*temp;
      lon2rad=torad*lon2;

/*
      Compute some of the easier and often used terms.
*/
      coslon1 = cos(lon1rad);
      sinlon1 = sin(lon1rad);
      coslon2 = cos(lon2rad);
      sinlon2 = sin(lon2rad);
      tanlat1 = tan(lat1rad);
      tanlat2 = tan(lat2rad);
      sinlat1 = sin(lat1rad);
      coslat1 = cos(lat1rad);
      sinlat2 = sin(lat2rad);
      coslat2 = cos(lat2rad);
/*
    The radii of curvature are compute from an equation defined in
    GEODESY by Bomford, Appendix A (page 647).
    v = semi_major/sqrt(1-e*e*sin(lat)*sin(lat))
*/
      v1 = a / sqrt( 1.0 - e2*sinlat1*sinlat1 );  /* radii of curvature */
      v2 = a / sqrt( 1.0 - e2*sinlat2*sinlat2 );  /* radii of curvature */
      aa = tanlat2 / ((1.0+eps)*tanlat1);
      bb = e2*(v1*coslat1)/(v2*coslat2);
      lambda12 = aa + bb;
      top = sinlon2*coslon1 - coslon2*sinlon1;
      bottom = lambda12*sinlat1-coslon2*coslon1*sinlat1-sinlon2*sinlon1*sinlat1;
      az = atan2(top,bottom)*todeg;
      if( az < 0.0 ) az = 360 + az;
      *azm = az;
      az = az * torad;
      cosazm = cos(az);
      sinazm = sin(az);

/*
   Now compute the distance using the equations on page 121 in GEODESY by
   Bomford (2.15 Reverse formulae).  There is some numerical problem with 
   the following formulae.
   If the station is in the southern hemisphere and the event is in the
   northern, these equations give the longer, not the shorter distance between
   the two locations.  Since the equations are messy, the simplist solution
   is to reverse the order of the lat,lon pairs.  This means that the azimuth
   must also be recomputed to get the correct distance.
*/
      if( lat2rad < 0.0 ) {
          temp = lat1rad;
          lat1rad = lat2rad;
          lat2rad = temp;
          temp = lon1rad;
          lon1rad = lon2rad;
          lon2rad = temp;

          coslon1 = cos(lon1rad);
          sinlon1 = sin(lon1rad);
          coslon2 = cos(lon2rad);
          sinlon2 = sin(lon2rad);
          tanlat1 = tan(lat1rad);
          tanlat2 = tan(lat2rad);
          sinlat1 = sin(lat1rad);
          coslat1 = cos(lat1rad);
          sinlat2 = sin(lat2rad);
          coslat2 = cos(lat2rad);

          v1 = a / sqrt( 1.0 - e2*sinlat1*sinlat1 );  
          v2 = a / sqrt( 1.0 - e2*sinlat2*sinlat2 );  

          aa = tanlat2 / ((1.0+eps)*tanlat1);
          bb = e2*(v1*coslat1)/(v2*coslat2);
          lambda12 = aa + bb;

          top = sinlon2*coslon1 - coslon2*sinlon1;
          bottom =lambda12*sinlat1-coslon2*coslon1*sinlat1-
                  sinlon2*sinlon1*sinlat1;
          az = atan2(top,bottom);
          cosazm = cos(az);
          sinazm = sin(az);
            
       }

       eps0 = eps * ( coslat1*coslat1*cosazm*cosazm + sinlat1*sinlat1 );
       b0 = (v1/(1.0+eps0)) * sqrt(1.0+eps*coslat1*coslat1*cosazm*cosazm);
     
       x2 = v2*coslat2*(coslon2*coslon1+sinlon2*sinlon1);
       y2 = v2*coslat2*(sinlon2*coslon1-coslon2*sinlon1);
       z2 = v2*(1.0-e2)*sinlat2;
       z1 = v1*(1.0-e2)*sinlat1;

       c0 = c00 + c01*eps0 + c02*eps0*eps0 + c03*eps0*eps0*eps0;
       c2 =       c21*eps0 + c22*eps0*eps0 + c23*eps0*eps0*eps0;
       c4 =                  c42*eps0*eps0 + c43*eps0*eps0*eps0;
       c6 =                                  c63*eps0*eps0*eps0;

       bottom = cosazm*sqrt(1.0+eps0);
       u1p = atan2(tanlat1,bottom);
          
       top = v1*sinlat1+(1.0+eps0)*(z2-z1);
       bottom = (x2*cosazm-y2*sinlat1*sinazm)*sqrt(1.0+eps0);
       u2p = atan2(top,bottom);

       aa = c0*(u2p-u1p);
       bb = c2*(sin(2.0*u2p)-sin(2.0*u1p));
       cc = c4*(sin(4.0*u2p)-sin(4.0*u1p));
       dd = c6*(sin(6.0*u2p)-sin(6.0*u1p));

       xdist = fabs(b0*(aa+bb+cc+dd));
       *dist = xdist;
	   return(1);
}



int	geo_to_km_deg (double lat1, double lon1, double lat2, double lon2,
					double *dist, double *xdeg, double *azm)
{

/*
=====================================================================
 PURPOSE:  To compute the distance and azimuth between locations.
=====================================================================
 INPUT ARGUMENTS:
    lat1:     Event latitude in decimal degrees, North positive. [r]
    lon1:     Event longitude, East positive. [r]
    lat2:     Array of station latitudes. [r]
    lon2:     Array of station longitudes. [r]
=====================================================================
 OUTPUT ARGUMENTS:
    DIST:    epicentral distance in km. [r]
    XDEG:    epicentral distance in degrees. [r]
    AXM:     azimuth in degrees. [r]
=====================================================================
*/
/*
Calculations are based upon the reference spheroid of 1968 and
are defined by the major radius (RAD) and the flattening (FL).
*/

      const double a = semi_major;
      const double b = semi_minor;
      double torad, todeg;
      double aa, bb, cc, dd, top, bottom, lambda12, az, temp;
      double v1, v2;
      double fl, e2, eps, eps0;
      double b0, x2, y2, z2, z1, u1p, u2p, xdist;
      double lat1rad, lat2rad, lon1rad, lon2rad;
      double coslon1, sinlon1, coslon2, sinlon2;
      double coslat1, sinlat1, coslat2, sinlat2;
      double tanlat1, tanlat2, cosazm, sinazm;
	  double onemec2;
	  double aa2, bb2, cc2, dd2, ee2, ff2, aa3, bb3, cc3, dd3, ee3, ff3;
	  double aminus, bminus, cminus, aplus, bplus, cplus;
	  double thg, sd, sc, deg;

      double c0, c2, c4, c6;

      double c00=1.0, c01=0.25, c02=-0.046875, c03=0.01953125;
      double c21=-0.125, c22=0.03125, c23=-0.014648438;
      double c42=-0.00390625, c43=0.0029296875;
      double c63=-0.0003255208;

/*  Check for special conditions                                  */

     if( lat1 == lat2 && lon1 == lon2 ) {
         *azm = 0.0;
         *dist= 0.0;
         return(0);
     }
/* - Initialize.             */

      torad = PI / 180.0;
      todeg = 1.0 / torad;
      fl = ( a - b ) / a;
      e2 = 2.0*fl - fl*fl;
      eps = e2 / ( 1.0 - e2);
      onemec2 = 1.0 - e2;
/*
* - Convert event location to radians.
*   (Equations are unstable for latidudes of exactly 0 degrees.)
*/

      temp=lat1;
      if(temp == 0.) temp=1.0e-08;
      lat1rad=torad*temp;
      lon1rad=torad*lon1;

      temp=lat2;
      if(temp == 0.) temp=1.0e-08;
      lat2rad=torad*temp;
      lon2rad=torad*lon2;

/*
      Compute some of the easier and often used terms.
*/
      coslon1 = cos(lon1rad);
      sinlon1 = sin(lon1rad);
      coslon2 = cos(lon2rad);
      sinlon2 = sin(lon2rad);
      tanlat1 = tan(lat1rad);
      tanlat2 = tan(lat2rad);
      sinlat1 = sin(lat1rad);
      coslat1 = cos(lat1rad);
      sinlat2 = sin(lat2rad);
      coslat2 = cos(lat2rad);

/*
    The radii of curvature are compute from an equation defined in
    GEODESY by Bomford, Appendix A (page 647).
    v = semi_major/sqrt(1-e*e*sin(lat)*sin(lat))
*/
      v1 = a / sqrt( 1.0 - e2*sinlat1*sinlat1 );  /* radii of curvature */
      v2 = a / sqrt( 1.0 - e2*sinlat2*sinlat2 );  /* radii of curvature */

      aa = tanlat2 / ((1.0+eps)*tanlat1);
      bb = e2*(v1*coslat1)/(v2*coslat2);
      lambda12 = aa + bb;

      top = sinlon2*coslon1 - coslon2*sinlon1;
      bottom = lambda12*sinlat1-coslon2*coslon1*sinlat1-sinlon2*sinlon1*sinlat1;
      az = atan2(top,bottom)*todeg;
      if( az < 0.0 ) az = 360 + az;
      *azm = az;
      az = az * torad;
      cosazm = cos(az);
      sinazm = sin(az);

/*
   Now compute the distance using the equations on page 121 in GEODESY by
   Bomford (2.15 Reverse formulae).  There is some numerical problem with
   the following formulae.
   If the station is in the southern hemisphere and the event is in the
   northern, these equations give the longer, not the shorter distance between
   the two locations.  Since the equations are messy, the simplist solution
   is to reverse the order of the lat,lon pairs.  This means that the azimuth
   must also be recomputed to get the correct distance.
*/

      if( lat2rad < 0.0 ) {
          temp = lat1rad;
          lat1rad = lat2rad;
          lat2rad = temp;
          temp = lon1rad;
          lon1rad = lon2rad;
          lon2rad = temp;

          coslon1 = cos(lon1rad);
          sinlon1 = sin(lon1rad);
          coslon2 = cos(lon2rad);
          sinlon2 = sin(lon2rad);
          tanlat1 = tan(lat1rad);
          tanlat2 = tan(lat2rad);
          sinlat1 = sin(lat1rad);
          coslat1 = cos(lat1rad);
          sinlat2 = sin(lat2rad);
          coslat2 = cos(lat2rad);

          v1 = a / sqrt( 1.0 - e2*sinlat1*sinlat1 );
          v2 = a / sqrt( 1.0 - e2*sinlat2*sinlat2 );

          aa = tanlat2 / ((1.0+eps)*tanlat1);
          bb = e2*(v1*coslat1)/(v2*coslat2);
          lambda12 = aa + bb;

          top = sinlon2*coslon1 - coslon2*sinlon1;
          bottom =lambda12*sinlat1-coslon2*coslon1*sinlat1-
                  sinlon2*sinlon1*sinlat1;
          az = atan2(top,bottom);
          cosazm = cos(az);
          sinazm = sin(az);

       }

       eps0 = eps * ( coslat1*coslat1*cosazm*cosazm + sinlat1*sinlat1 );
       b0 = (v1/(1.0+eps0)) * sqrt(1.0+eps*coslat1*coslat1*cosazm*cosazm);

       x2 = v2*coslat2*(coslon2*coslon1+sinlon2*sinlon1);
       y2 = v2*coslat2*(sinlon2*coslon1-coslon2*sinlon1);
       z2 = v2*(1.0-e2)*sinlat2;
       z1 = v1*(1.0-e2)*sinlat1;

       c0 = c00 + c01*eps0 + c02*eps0*eps0 + c03*eps0*eps0*eps0;
       c2 =       c21*eps0 + c22*eps0*eps0 + c23*eps0*eps0*eps0;
       c4 =                  c42*eps0*eps0 + c43*eps0*eps0*eps0;
       c6 =                                  c63*eps0*eps0*eps0;

       bottom = cosazm*sqrt(1.0+eps0);
       u1p = atan2(tanlat1,bottom);

       top = v1*sinlat1+(1.0+eps0)*(z2-z1);
       bottom = (x2*cosazm-y2*sinlat1*sinazm)*sqrt(1.0+eps0);
       u2p = atan2(top,bottom);

       aa = c0*(u2p-u1p);
       bb = c2*(sin(2.0*u2p)-sin(2.0*u1p));
       cc = c4*(sin(4.0*u2p)-sin(4.0*u1p));
       dd = c6*(sin(6.0*u2p)-sin(6.0*u1p));

       xdist = fabs(b0*(aa+bb+cc+dd));
       *dist = xdist;

/* compute the distance in degrees                                */
		thg=atan(onemec2*tan(lat1rad));
		dd2=sin(lon1rad);
		ee2=-cos(lon1rad);
		ff2=-cos(thg);
		cc2=sin(thg);
		aa2= ff2*ee2;
		bb2=-ff2*dd2;

/* -- Calculate some trig constants.          */

		thg=atan(onemec2*tan(lat2rad));
		dd3=sin(lon2rad);
		ee3=-cos(lon2rad);
		ff3=-cos(thg);
		cc3=sin(thg);
		aa3=ff3*ee3;
		bb3=-ff3*dd3;
		sc=aa2*aa3+bb2*bb3+cc2*cc3;

/* - Spherical trig relationships used to compute angles.       */

        aminus = aa2 - aa3;
		bminus = bb2 - bb3;
		cminus = cc2 - cc3;
		aplus  = aa2 + aa3;
		bplus  = bb2 + bb3;
		cplus  = cc2 + cc3;

		sd=0.5*sqrt((aminus*aminus + bminus*bminus+cminus*cminus)*(aplus*aplus
			+ bplus*bplus + cplus*cplus));
		deg=atan2(sd,sc)*todeg;
		*xdeg = deg;

	   return(1);
}

void distaz_geo(olat,olon,dist,azimuth,lat,lon)
double olat, olon, azimuth, dist, *lat, *lon;
{

/*
=====================================================================
 PURPOSE:  To compute latitude and longitude given a distance and
            azimuth from a geographic point (olat,olon).
=====================================================================
 INPUT ARGUMENTS:
    olat:    latitude in decimal degrees, North positive.
    olon:    longitude in decimal degrees, East positive.
    dist:    distance from point (olat,olon) in kms.
    azimuth: azimuath between point (olat,olon) and (lat,lon) in degrees.
=====================================================================
 OUTPUT ARGUMENTS:
    lat:    latitude in decimal degrees, North positive.
    lon:    longitude in decimal degrees, East positive.
=====================================================================
*/

      double C, C_sqr;
      double torad, todeg;
      double aa, bb, a0, b0;
      double g0, g2, g4, g6;
      double latrad, lonrad, azmrad;
      double ec2, eps, eps0,       fl;
      double tanmu;
      double tanlat, coslat, sinlat, tanazm, sinazm, cosazm;
      double v1, u1p, u2p, u1pbot, x2, y2, z2, ztop, zbot;
      double temp,        mu, sd, sig1      ;
      /* double sig2, eps2, delta; */

/*
Calculations are based upon the reference spheroid of 1967.
See GEODESY by Bomford for definitions and reference values.
Reference spheriod is found in GEODESY by Bomford (page 426).  Definitions
for flattening, eccentricity and second eccentricity are found in
Appendix A (page 646).
*/

      const double a = semi_major;
      const double b = semi_minor;
      double a1=0.25, b1=-0.125, c1=-0.0078125;
      double g00=1.0, g01=-0.25, g02=0.109375, g03=-0.058139535;
      double g21=0.125, g22=-0.06250, g23=0.034667969;
      double g42=0.01953125, g43=-0.01953125;
      double g63=0.004720052;

/* - Initialize.             */

      fl = ( a - b ) / a;         /* earth flattening                 */
      ec2 = 2.0*fl-fl*fl;         /* square of eccentricity           */
      eps = ec2/(1.0-ec2);        /* second eccentricity e'*e' = eps  */
      torad = PI / 180.0;
      todeg = 1.0 / torad;

/* - Convert event location to radians.                               */

      temp=olat;
      if(temp == 0.) temp=1.0e-08;
      latrad=torad*temp;
      lonrad=torad*olon;
      azmrad=azimuth*torad;

/*  Compute some of the easier terms               */

      coslat = cos(latrad);
      sinlat = sin(latrad);
      cosazm = cos(azmrad);
      sinazm = sin(azmrad);
      tanazm = tan(azmrad);
      tanlat = tan(latrad);

      C_sqr = coslat*coslat*cosazm*cosazm+ sinlat*sinlat;
      C = sqrt(C_sqr);
      eps0 = C_sqr * eps;
      v1 = a / sqrt( 1.0 - ec2*sinlat*sinlat ); /* Radii of curvature */
      b0 = v1*sqrt(1.0+eps*coslat*coslat*cosazm*cosazm)/(1.0+eps0);

      g0 = g00 + g01*eps0 + g02*eps0*eps0 + g03*eps0*eps0*eps0;
      g2 =       g21*eps0 + g22*eps0*eps0 + g23*eps0*eps0*eps0;
      g4 =                  g42*eps0*eps0 + g43*eps0*eps0*eps0;
      g6 =                                  g63*eps0*eps0*eps0;


      u1pbot = cosazm * sqrt(1.0+eps0);
      u1p=atan2(tanlat,u1pbot);
      sig1 = 0.5*( 2.0*u1p-(a1*eps0+b1*eps0*eps0)*sin(2.0*u1p)+
                 c1*eps0*eps0*sin(4.0*u1p));
/*
      aa = ( sig2 - sig1 ) page 117, GEODESY
      bb = ( sig1 + sig2 ) page 117, GEODESY
*/
      aa =  (dist*g0)/b0;
      bb = 2.0 * sig1 + aa;
      u2p=u1p+aa+2.0*g2*sin(aa)*cos(bb)+2.0*g4*sin(2.*aa)*cos(2.*bb)+
          2.*g6*sin(3.*aa)*cos(3.*bb);

/*  This calculation of latitude is based on Rudoe's formulation, which
    has not been tested  */

/*    sinu1=tanlat/sqrt(1.0+eps+tanlat*tanlat);                       */
/*    sinu2 = ((b0*C)/b)*sin(u2p)-((eps-eps0)/(1+eps0))*sinu1;        */
/*    u2 = asin(sinu2);                                               */
/*    arg = sinu2/sqrt(1.0-eps*eps*cos(u2)*cos(u2));                  */
/*    *lat = asin(arg)*todeg;                                         */

      a0 = b0*sqrt(1.0+eps0);
      tanmu = sinlat*tanazm;
      mu = atan(tanmu);

/*  This calculation of longitude is based on Ruloe's formulation, which
    has not been tested  */

/*    arg = (a0*cos(u2p))/(a*cos(u2));                                */
/*    delta = acos(arg)-mu;                                           */
/*    *lon = (lonrad + delta)*todeg;                                  */

/*
   This calculation of latitude and longitude is an alternative to Rudoe's
   formulation.  See GEODESY by Bomford (page 118).
*/
      sd=(ec2*v1*sinlat*coslat*sinazm)/(1.-ec2*coslat*coslat*sinazm*sinazm);
      x2=a0*cos(u2p)*cos(mu)+b0*sin(u2p)*sin(mu)*coslat*sinazm+
         sd*sinlat*sinazm;
      y2=-a0*cos(u2p)*sin(mu)+b0*sin(u2p)*cos(mu)*coslat*sinazm+
         sd*cosazm;
      z2=b0*C*sin(u2p)-(sd*coslat*sinazm)/(1.0+eps);
      ztop = (1.0+eps)*z2;
      zbot = sqrt(x2*x2+y2*y2);
      *lat = atan2(ztop,zbot)*todeg;
      *lon = (atan(y2/x2)+lonrad)*todeg;
      if((atan(y2/x2)+lonrad)<0.0) *lon=((atan(y2/x2)+lonrad)*todeg);
}
