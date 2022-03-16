/*
**  c_geo_to_km.h -- header file used with geo_to_km.obj
*/
//---------------------------------------------------------------------------
#ifndef _C_GEO_TO_KM_H
#define _C_GEO_TO_KM_H
//---------------------------------------------------------------------------

extern "C" {
   
   int geo_to_km( double   lat1
                , double   lon1
                , double   lat2
                , double   lon2
                , double * dist
                , double * azm
                );

   int geo_to_km_deg( double   lat1
                    , double   lon1
                    , double   lat2
                    , double   lon2
                    , double * dist
                    , double * xdeg
                    , double * azm
                    );

   void distaz_geo( double   olat
                  , double   olon
                  , double   dist
                  , double   azimuth
                  , double * lat
                  , double * lon
                  );
}
#endif

 