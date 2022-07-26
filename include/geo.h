/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: geo.h 3364 2008-09-26 23:58:00Z kress $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2008/09/26 23:58:00  kress
 *     typo fix
 *
 *     Revision 1.2  2008/09/26 22:25:00  kress
 *     Fix numerous compile warnings and some tab-related fortran errors for linux compile
 *
 *     Revision 1.1  2004/11/01 19:11:15  davidk
 *     header file for geo_to_km*()
 *
 *
 ***************************************************************/
#ifndef GEO_H
# define GEO_H

/*****************************************************
 * This file contains functions useful for computing
 * the distance and azimuth between locations on the
 * earth's surface, and vice versa.
 * DK 102904
 *****************************************************/

/* value below fixed by Alex 6/6/1 */
#define PI 3.14159265358979323846

int geo_to_km(double lat1,double lon1,double lat2,double lon2,double* dist,double* azm);
/********************************************************************
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
********************************************************************/

int	geo_to_km_deg (double lat1, double lon1, double lat2, double lon2,
					double *dist, double *xdeg, double *azm);
/********************************************************************
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
********************************************************************/


void distaz_geo(double olat,double olon,double dist,double azimuth,
		double *lat,double *lon);
/********************************************************************
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
********************************************************************/


#endif
