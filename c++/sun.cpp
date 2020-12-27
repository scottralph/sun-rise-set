/* Original source from Dr. Robert Scharein */

/*  
    Source:  http://broadband.gc.ca/maps/index.html

    Places to add:

    BEAUSEJOUR, MB      50.062244  96.50771
    DOMINION CITY, MB   49.132584  97.16368
    FORT SIMPSON, NT    61.860214 121.368576
    FORT GOOD HOPE, NT  66.26154  128.54745
    DETTAH, NT          62.40766  114.12293
    CAMBRIDGE BAY, NU   69.13749  105.17766
    RANKIN INLET, NU    62.81129   92.09558
    POND INLET, NU      72.69315   77.795715
    BIRCH RIVER, MB     52.565475 101.378624
    CHURCHILL, MB       58.76226   94.13347
    EMERSON, MB         49.04069   97.227905
    GIMLI, MB           50.63507   97.001945
    GRAND RAPIDS, MB    53.208206  99.2975
    PINE FALLS, MB      50.508434  96.20633
    MONTREAL, QC        45.537827  73.599884

    Krafla Caldera, IS  65.73      16.68
    Grimsvotn Caldera   64.42      17.33
    


*/

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
//#include "sun.h"
//#include "time_scales.h"
#define ABS(X)      ((X) < 0.0 ? -(X) : (X))

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

char comment [300];
double daylight_savings = 0.0;
bool altitudes_only = false;
bool zarvox = false;
#define DEG_TO_RAD   0.0174532925199432957692  /*  (PI / 180)  */
#define RAD_TO_DEG   57.2957795130823208768    /*  (180 / PI)  */
#define PI           3.14159265358979323846
#define TWOPI        6.28318530717958647693
#define PIBY2        1.57079632679489661923

#define DMS(D,M,S)   (((M) + (S) / 60.0) / 60.0 + D)
#define DMSsec(D,M,S)   ((S) + (M) * 60.0 + (D) * 3600.0)

char month_name [13][4] = {"abc",
                           "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

#define WEST_EAST   0
#define NORTH_SOUTH 1

void offset_location (char *location, double *lat, double *lon, char *how_much, int dir) {
  // Offset the location at (lat, lon) by how_much kilometres in the direction dir (either N/S or W/E).
  // Assumption is that this is a small distance (less than several dozen kilometres), so that
  // time zone doesn't change. 
  // Also assuming (only here) that Earth is as close to a sphere of radius 6378.137km
  // as "damm it!" is to swearing. 
  double Earth_radius = 6378.137;     // Equatorial adius of Earth (kilometres)
  double kilometres_offset = atof (how_much);

  switch (dir) {
  case NORTH_SOUTH:
    *lat += 360.0 * kilometres_offset / (TWOPI * Earth_radius);
    if (kilometres_offset > 0.0) 
      sprintf (comment, "%.2f km N of %s", kilometres_offset, location);
    else 
      sprintf (comment, "%.2f km S of %s", -kilometres_offset, location);
    strcpy (location, comment);
    break;
  case WEST_EAST:
    *lon += 360.0 * kilometres_offset / (TWOPI * Earth_radius) / cos (DEG_TO_RAD * (*lat));
    if (kilometres_offset > 0.0) 
      sprintf (comment, "%.2f km W of %s", kilometres_offset, location);
    else 
      sprintf (comment, "%.2f km E of %s", -kilometres_offset, location);
    strcpy (location, comment);
    break;
  }
}

double to360 (double angle) {
  // forces an angle to be in range 0.0 to 360.0
  int N = (int) (angle / 360.0);
  if (angle < 0.0) N--;
  return angle - (double) N * 360.0;
}

void error (char *what) {
  fprintf (stdout, "\nERROR: %s\n", what);
  exit (636352);
}

double DeltaT (double JD) {
  // Returns 
  //     DeltaT = TD - UT
  // the difference between Terrestrial Dynamical Time and Universal Time (in seconds).

  double JD_1jan2000 = 2451544.5;
  double delta_year = (JD - JD_1jan2000) / 365.25;

  //if (delta_year < -10.0 || delta_year > 6.0) error ("can't do it!");
  //  if (delta_year < -3.0 || delta_year > 3.0) fprintf (stdout, "\n *** Warning: may be inaccurate!\n");

  
  return 65.0 + delta_year;  // from 1998 to 2003, assume a second per year
}

double JulianDay (double seconds, int month, int day, int year) {
  if (month < 3) {
    year--;
    month += 12;
  }
  int A = year / 100;
  int B = 2 - A + (A / 4);

  return (double) ((int) (365.25 * ((double) year + 4716.0)) + 
                   (int) (30.6001 * ((double) month + 1.0)) + day + B) - 1524.5 + seconds / (24.0 * 3600.0);
}

double JulianDay2 (int month, double day, int year) {
  int D;
  D = (int) day;
  double seconds;
  seconds = (day - (double) D) * 3600.0 * 24.0;
  return JulianDay (seconds, month, D, year);
}

double true_altitude_centre_of_disk_of_Sun (double UT,         // Universal Time UT1 (in seconds since 0 UT)
                                            int month,         // Month (1 = Jan, 2 = Feb, etc.)
                                            int day,           // Day of month
                                            int year,          // Calendar year
                                            double lat,        // Geographical Latitude (degrees)
                                            double lon,        // Geographical Longitude (degrees, west is positive)
                                            double *azimuth) { // Azimuth of Sun at event
    
  double JDE;   // Julian Ephemeris Day (based on Terrestrial Dynamical Time)
  double JD;    // Julian Day (based on Universal Time)
  double TD;    // Terrestrial Dynamical Time (A uniform time scale, TD = TAI + 32.184 sec., where TAI is Terrestrial Atomic Time)


  JD  = JulianDay (UT, month, day, year);
  TD = UT + DeltaT (JD);

  JDE = JulianDay (TD, month, day, year);

  double T;    // Time, in Julian centuries of 36525 ephemeris days from the epoch J2000.0
  T = (JDE - 2451545.0) / 36525.0;

  double TUT;  // Same as T, but using JD instead of JDE
  TUT = (JD - 2451545.0) / 36525.0;

  double L0;   // Geometric mean longitude of the Sun, referred to the mean equinox of the date (degrees)
  L0 = 280.46646 + 36000.76983 * T + 0.0003032 * T * T;

  double M;    // Mean anomaly of the Sun
  M = 357.52911 + 35999.05029 * T - 0.0001537 * T * T;

  double e;    // Eccentricity of the Earth's orbit
  e = 0.016708634 - 0.000042037 * T - 0.0000001267 * T * T;

  double C;    // Equation of the centre of SUn
  C = (1.914602 - 0.004817 * T - 0.000014 * T * T) * sin (DEG_TO_RAD * M) +
    (0.019993 -0.000101 * T) * sin (DEG_TO_RAD * 2.0 * M) +
    0.000289 * sin (DEG_TO_RAD * 3.0 * M);

  double circle_dot;  // Sun's true longitude
  circle_dot = to360 (L0 + C);

  double nu;          // Sun's true anomaly
  nu = M + C;

  double R;           // Sun's radius vector (A.U.)
  R = 1.000001018 * (1.0 - e * e) / (1.0 + e * cos (DEG_TO_RAD * nu));

  double Omega;
  Omega = 125.04 - 1934.136 * T;

  double lambda;      // Sun's apparent longitude
  lambda = circle_dot - 0.00569 - 0.00478 * sin (DEG_TO_RAD * Omega);

  double epsilon0;     // Mean obliquity of the ecliptic  (degrees)
  epsilon0 = (DMSsec (23.0, 26.0, 21.448) - 46.8150 * T - 0.00059 * T * T + 0.001813 * T * T * T) / 3600.0;

  double epsilon;    // True obliquity of the ecliptic  (degrees)
  epsilon = epsilon0 + 0.00256 * cos (DEG_TO_RAD * Omega);

  double alpha_app;   // Apparent right ascension
  double delta_app;   // Apparent declination

  alpha_app = RAD_TO_DEG * atan2 (cos (DEG_TO_RAD * epsilon) * sin (DEG_TO_RAD * lambda), cos (DEG_TO_RAD * lambda));
  delta_app = RAD_TO_DEG * asin (sin (DEG_TO_RAD * epsilon) * sin (DEG_TO_RAD * lambda));

  /*
    printf ("T = %.9f\n", T);
    printf ("L0 = %f\n", L0);
    printf ("M = %f\n", M);
    printf ("e = %.9f\n", e);
    printf ("C = %f\n", C);
    printf ("circle_dot = %f\n", circle_dot);
    printf ("R = %f\n", R);
    printf ("Omega = %f\n", Omega);
    printf ("lambda = %f\n", lambda);
    printf ("epsilon0 = %f\n", epsilon0);
    printf ("epsilon = %f\n", epsilon);
    printf ("alpha_app = %f\n", alpha_app);
    printf ("delta_app = %f\n", delta_app);
  */
  double Omega_moon;  // Longitude of the ascending node of the Moon's mean orbit on the ecliptic,
  // measured from the mean equinox of the date
  Omega_moon = 125.04452 - 1934.136261 * T + 0.0020708 * T * T + T * T * T / 450000.0;  


  double L_moon;     // Mean longitude of the Moon
  L_moon = 218.3165 + 481267.8813 * T;        // ????

  double nut_long;  // Nutation in longitude (seconds of arc)
  nut_long = -17.20 * sin (DEG_TO_RAD * Omega_moon) - 1.32 * sin (DEG_TO_RAD * 2.0 * L0) -
    0.23 * sin (DEG_TO_RAD * 2.0 * L_moon) + 0.21 * sin (DEG_TO_RAD * 2.0 * Omega_moon);  // ????

  //printf ("Omega = %f\nnut_long = %f\n", Omega_moon, nut_long);

  double theta0;   // Mean sidereal time at Greenwich (degrees)
  theta0 = 280.46061837 + 360.98564736629 * (JD - 2451545.0) + 
    0.000387933 * TUT * TUT - TUT * TUT * TUT / 38710000.0; // Should use UT, not TD

  double theta;    // Apparent sidereal time at Greenwich (degrees)
  theta = theta0 + nut_long * cos (DEG_TO_RAD * epsilon) / 3600.0;

  double H;        // Local hour angle
  H = theta0 - lon - alpha_app;

  double h;        // Apparent altitude
  h = RAD_TO_DEG * asin (sin (DEG_TO_RAD * lat) * sin (DEG_TO_RAD * delta_app) +
                         cos (DEG_TO_RAD * lat) * cos (DEG_TO_RAD * delta_app) * cos (DEG_TO_RAD * H));

  if (azimuth) 
    *azimuth = 0.0;
  //printf ("H = %.9f\nh = %.9f\n", H / 15.0, h);
  return h;
}

#define test(Y,M,D)  printf ("%d %s %f = %f\n", Y, month_name [M], D, JulianDay2 (M, D, Y))

double time_of_event (double target_altitude, 
                      int month, int day, int year,   // Local time
                      int setting,    // TRUE for a setting phenomena, FALSE for rising
                      double lat, double lon, double time_zone) {  // Latitude, longitude, and time zone 
  double UT_low, UT_high, UT, altitude;
  double altitude_max = 0.0;
  double UT_noon = 0.0;

  if (target_altitude > 90.0) {
    UT_low  = (10.0 - time_zone) * 3600.0;
    UT_high = (14.0 - time_zone) * 3600.0;
    for (UT = UT_low; UT <= UT_high; UT += 1.0) {
      altitude = true_altitude_centre_of_disk_of_Sun (UT, month, day, year, lat, lon, NULL);
      if (altitude > altitude_max) {
        altitude_max = altitude;
        UT_noon = UT;
      }
    }
    return UT_noon / 3600.0 + time_zone;
  }

  if (altitudes_only) {
    UT_low  = (0.0  - time_zone) * 3600.0;
    UT_high = (24.1 - time_zone) * 3600.0;
    for (UT = UT_low; UT < UT_high; UT += 900.0) {
      altitude = true_altitude_centre_of_disk_of_Sun (UT, month, day, year, lat, lon, NULL);
      printf ("%.2f %.1f\n", UT / 3600.0 + time_zone, altitude);
    }
    exit (102);
  }

  if (setting) {
    UT_low  = (12.0 - time_zone) * 3600.0;
    UT_high = (24.0 - time_zone) * 3600.0;
  }
  else {
    UT_low  = (0.0  - time_zone) * 3600.0;
    UT_high = (12.0 - time_zone) * 3600.0;
  }

  do {
    UT = (UT_low + UT_high) / 2.0;
    altitude = true_altitude_centre_of_disk_of_Sun (UT, month, day, year, lat, lon, NULL);
    //printf ("%d: altitude = %f, UT = %f %f %f\n", N, altitude, UT_low, UT, UT_high);

    if (altitude > target_altitude) {
      if (setting) 
        UT_low = UT;
      else
        UT_high = UT;
    }
    else {
      if (setting) 
        UT_high = UT;
      else
        UT_low  = UT;
    }
 
  } while (UT_high - UT_low > 0.01);

  if (ABS (altitude - target_altitude) > 0.01) {
    printf ("does not occur\n");
    exit (1022);
  }

  return UT / 3600.0 + time_zone;
}

void house_of_paul_kry (int month, int day, int year) {
  double lat = DMS (49, 16, 17), lon = DMS (123, 10, 55);  // Paul Kry's house

  double LST; 

  // Modify so that prints days from current day - 10 to current day + 10
  
  for (day = 1; day <= 31; day++) {
    LST = time_of_event (-50.0 / 60.0, month, day, year, TRUE, lat, lon, -8.0);  // assuming standard sun and normal atmospheric refraction
    int hour = (int) LST;
    double dminutes = (LST - (double) hour) * 60.0;
    int minutes = (int) dminutes;
    double seconds = (dminutes - (double) minutes) * 60.0;
    printf ("On %s %d sunset is at PST = %dh %dm %.2fs as seen from Paul Kry's house\n", 
            month_name [month], day, hour, minutes, seconds);
  }
}

int scom (char *s, char *t) {
  /*
    "s" is a pointer to a string which is supplied from the command line.
    "t" is a pointer to a string which is supplied from the testing routines.
    If strlen (s) > strlen (t) then the strings are considered not to match.
  */
  int i, same, length;
  if (s == NULL || t == NULL) return FALSE;
  if ((length = (int) strlen (s)) > (int) strlen (t)) return FALSE;
  if (length == 0) return FALSE;
  same = TRUE;
  i = 0;
  while (same && i < length) {
    if (s [i] != t [i]) same = FALSE;
    else i++;
  }
  return same;
}

bool allow_DST = true;

void get_event (char *event, double *talt, int *setting, char *what) {
  if (scom (what, "set") || scom (what, "sunset")) {
    sprintf (event, "sunset");
    *talt = -50.0 / 60.0;
    *setting = TRUE;
  }
  else if (scom (what, "rise") || scom (what, "sunrise")) {
    sprintf (event, "sunrise");
    *talt = -50.0 / 60.0;
    *setting = FALSE;
  }
  else if (scom (what, "irise")) {
    sprintf (event, "ideal sunrise");
    *talt = 0.0;
    *setting = FALSE;
  }
  else if (scom (what, "iset")) {
    sprintf (event, "ideal sunset");
    *talt = 0.0;
    *setting = TRUE;
  }
  else if (scom (what, "bcivil")) {
    sprintf (event, "beginning of civil twilight");
    *talt = -6.0;
    *setting = FALSE;
  }
  else if (scom (what, "ecivil") || scom (what, "civil")) {
    sprintf (event, "end of civil twilight");
    *talt = -6.0;
    *setting = TRUE;
  }
  else if (scom (what, "bdave")) {
    sprintf (event, "beginning of Dave twilight");
    *talt = -8.5;
    *setting = FALSE;
  }
  else if (scom (what, "bnautical")) {
    sprintf (event, "beginning of nautical twilight");
    *talt = -12.0;
    *setting = FALSE;
  }
  else if (scom (what, "enautical")) {
    sprintf (event, "end of nautical twilight");
    *talt = -12.0;
    *setting = TRUE;
  }
  else if (scom (what, "bgolden")) {
    sprintf (event, "beginning of golden light");
    *talt = 8.0;
    *setting = TRUE;
  }
  else if (scom (what, "brob")) {
    sprintf (event, "beginning of `rob' light");
    *talt = 4.0;
    *setting = TRUE;
  }
  else if (scom (what, "SPF102.2")) {
    sprintf (event, "Sun protection factor of 102.2 relative to Sun at zenith");
    *talt = 32.8955;
    *setting = TRUE;
  }
  else if (scom (what, "eburn")) {
    sprintf (event, "end of sunburning time");
    *talt = 32.8955;
    *setting = TRUE;
  }
  else if (scom (what, "bburn")) {
    sprintf (event, "beginning of sunburning time");
    *talt = 32.8955;
    *setting = FALSE;
  }
  else if (scom (what, "buv")) {
    sprintf (event, "beginning of high UVB");
    *talt = 54.0;
    *setting = FALSE;
  }
  else if (scom (what, "euv")) {
    sprintf (event, "end of high UVB");
    *talt = 54.0;
    *setting = TRUE;
  }
  else if (scom (what, "egolden")) {
    sprintf (event, "end of golden light");
    *talt = 8.0;
    *setting = FALSE;
  }
  else if (scom (what, "edave")) {
    sprintf (event, "end of Dave twilight");
    *talt = -8.5;
    *setting = TRUE;
  }
  else if (scom (what, "bastronomical")) {
    sprintf (event, "beginning of astronomical twilight");
    *talt = -18.0;
    *setting = FALSE;
  }
  else if (scom (what, "eastronomical")) {
    sprintf (event, "end of astronomical twilight");
    *talt = -18.0;
    *setting = TRUE;
  }
  else if (scom (what, "noon")) {
    sprintf (event, "high noon (maximum altitude of Sun)");
    *talt = 102.2;
    *setting = FALSE;
  }
  else if (scom (what, "altitudes"))
    altitudes_only = true;
  else 
    error ("unknown event type");
}

void get_location (char *location, double *lat, double *lon, double *tzone, char *what) {
  if (scom (what, "vancouver")) {
    //*lat = DMS (49, 16, 17), *lon = DMS (123, 10, 55);  // Paul Kry's house
    *lat = DMS (49, 15, 26.75), *lon = DMS (123, 8, 46.87);  // Canadian Memorial Centre for Peace
    sprintf (location, "Vancouver");
    *tzone = -8.0;
  }
  else if (scom (what, "sanfrancisco")) {
    *lat = DMS (37.0, 47.0, 4.3);  // Chew/Bullock residence
    *lon = DMS (122.0, 28.0, 7.3);
    sprintf (location, "San Francisco");
    *tzone = -8.0;
  }
  else if (scom (what, "davis")) {
    *lat = DMS (38.0, 33.0, 14);
    *lon = DMS (121.0, 44, 17);
    sprintf (location, "Davis, California");
    *tzone = -8.0;
  }
  else if (scom (what, "deathvalley")) {
    *lat = DMS (36.0, 39.0, 45.0);  // moving rocks at the Devil's Racetrack
    *lon = DMS (117.0, 33.0, 20.0);
    sprintf (location, "Death Valley, California");
    *tzone = -8.0;
  }

  // 49Â°35'43.23"N
  // 119Â°36'1.41"W
  else if (scom (what, "powell")) {
    *lon = 124.55090996;
    *lat = 49.874614965;
    sprintf (location, "Powell River");
    *tzone = -8.0;
  }

  else if (scom (what, "cumberland")) {
    *lon = DMS (125, 1, 36);
    *lat = DMS (49, 37, 16);
    sprintf (location, "Cumberland");
    *tzone = -8.0;
  }


  else if (scom (what, "winnipeg")) {
    *lat = DMS (49.0, 53.0, 4.0);   // the forks??
    *lon = DMS (97.0, 8.0, 47.0);
    sprintf (location, "Winnipeg");
    *tzone = -6.0;
  }
  else if (scom (what, "chicago")) {  // 41.90   87.65  
    *lat = 41.9;  
    *lon = 87.65;
    sprintf (location, "Chicago");
    *tzone = -6.0;
  }
  else if (scom (what, "tom")) {  // 72.27515541220987,45.25271205225604
    *lat = 45.25271205225604;  
    *lon = 72.2751554122098;
    sprintf (location, "Tom & Anne Marie's");
    *tzone = -5.0;
  }
  else if (scom (what, "scott")) {  // 79.69672758936413,43.45878674527088
    *lat = 43.45878674527088;  
    *lon = 79.69672758936413;
    sprintf (location, "Scott, Valerie and Victoria's home");
    *tzone = -5.0;
  }
  else if (scom (what, "toronto")) { 
    *lat = 43.658030;  
    *lon = 79.395508;
    sprintf (location, "Toronto");
    *tzone = -5.0;
  }
  else if (scom (what, "iowacity")) { // U. of Iowa
    *lat = DMS (41.0, 39.0, 44.4);
    *lon = DMS (91.0, 32.0, 0.55); 
    sprintf (location, "Iowa City");
    *tzone = -6.0;
  }
  else if (scom (what, "ima")) { // IMA in MN
    *lat = DMS (44.0, 58.0, 28.14);
    *lon = DMS (93.0, 14.0, 7.72); 
    sprintf (location, "IMA");
    *tzone = -6.0;
  }
  else if (scom (what, "wendy")) {
    *lat = DMS (49.0, 25.0, 0.0);  // Wendy's
    *lon = DMS (123.0, 19.0, 0.0);
    sprintf (location, "Wendy's place");
    *tzone = -8.0;
  }
  else if (scom (what, "calgary")) {
    *lat = 50.944183;
    *lon = 114.091945;
    sprintf (location, "Calgary");
    *tzone = -7.0;
  }
  else if (scom (what, "banff")) {
    *lat =  51.1721;
    *lon =  115.5615;
    sprintf (location, "Banff Centre");
    *tzone = -7.0;
  }
  else if (scom (what, "saskatoon")) {
    // McLean Hall, University of Saskatchewan
    *lon = 106.6376963315182;
    *lat = 52.12981763843212;
    sprintf (location, "Saskatoon");
    *tzone = -6.0;  // Note: Saskatchewan uses CST all year long
  }
  else if (scom (what, "edmonton")) {
    *lat = DMS (53.0, 34.0, 0.0);   // accurate only to nearest minute
    *lon = DMS (113.0, 31.0, 0.0);
    sprintf (location, "Edmonton");
    *tzone = -7.0;
  }
  else if (scom (what, "yellowknife")) {
    *lat = DMS (62, 27, 20);  
    *lon = DMS (114, 21, 0);
    sprintf (location, "Yellowknife");
    *tzone = -7.0;
  }

  // 53.694369,-6.475031
  else if (scom (what, "newgrange")) {
    *lat = 53.694369;
    *lon = 6.475031;
    sprintf (location, "Newgrange");
    *tzone = 0.0;
  }
  else if (scom (what, "stonehenge")) {
    *lat = 51.178381;
    *lon = 1.824018;
    sprintf (location, "Stonehenge");
    *tzone = 0.0;
  }
  else if (scom (what, "castlerigg")) {
    *lat = 54.603;
    *lon = 3.098;
    sprintf (location, "Castlerigg");
    *tzone = 0.0;
  }
  else if (scom (what, "wales")) {
    *lat = 53.155778;
    *lon = 4.042647;
    sprintf (location, "Nant Ffrancon, Wales");
    *tzone = 0.0;
  }
  else if (scom (what, "glenys")) {
    *lat = DMS (50, 59, 31.60);
    *lon = -DMS (0, 33, 22.90);
    sprintf (location, "Glenys's house");
    *tzone = 0.0;
  }
  else if (scom (what, "rye")) {
    *lat = 50.950096;
    *lon = -0.721559;
    sprintf (location, "Rye");
    *tzone = 0.0;
  }
  else if (scom (what, "stleonards")) {
    *lat = 50.858104;
    *lon = -0.560004;
    sprintf (location, "St. Leonard's");
    *tzone = 0.0;
  }
  else if (scom (what, "avebury")) {
    *lat = 51.428420;
    *lon = 1.851830;
    sprintf (location, "Avebury");
    *tzone = 0.0;
  }
  else if (scom (what, "reykjavik")) {
    *lat = DMS (64.0, 4.0, 0.0);
    *lon = DMS (21.0, 58.0, 0.0);
    sprintf (location, "Reykjavik");
    *tzone = 0.0;
    daylight_savings = 0.0;
  }
  // 59 17 N 18 3 E 6:00 p.m.
  else if (scom (what, "stockholm")) {
    *lat = DMS (59.0, 17.0, 0.0);
    *lon = -DMS (18.0, 3.0, 0.0);
    sprintf (location, "Stockholm");
    *tzone = 1.0;
  }
  else if (scom (what, "callanish")) {
    *lat = 58.197790;
    *lon = 6.744047;
    sprintf (location, "Callanish");
    *tzone = 0.0;
  }
  else if (scom (what, "boston")) {
    *lat = 42.34779;  // actually Brighton MA (Scott & Valerie's)
    *lon = 71.156599;
    sprintf (location, "Boston");
    *tzone = -5.0;
  }
  else if (scom (what, "sherbrooke")) {
    *lat = DMS (45.0, 24.0, 0.0);
    *lon = DMS (71.0, 54.0, 0.0);
    sprintf (location, "Sherbrooke");
    *tzone = -5.0;
  }
  else if (scom (what, "pisa")) {
    *lat = DMS (43.0, 42.0, 43.0);
    *lon = -DMS (10.0, 24.0, 54.0);
    sprintf (location, "Pisa");
    *tzone = 1.0;
  }
  else if (scom (what, "amsterdam")) {
    *lat = DMS (52.0, 29.0, 15.0);
    *lon = -DMS (4.0, 48.0, 25.0);
    sprintf (location, "Amsterdam");
    *tzone = 1.0;
  }
  else if (scom (what, "svalbard")) {
    *lat = DMS (78.0, 13.0, 12.0);
    *lon = -DMS (15.0, 39.0, 1.0);
    sprintf (location, "Longyearbyen, Svalbard, Norway");
    *tzone = 1.0;
  }
  else if (scom (what, "honolulu")) {
    *lat = 21.2918;
    *lon = 157.8333;
    sprintf (location, "Honolulu");
    *tzone = -10.0;
    allow_DST = false;
  }
  else if (scom (what, "hawaii")) {
    allow_DST = false;
    *lat = 19.577969;
    *lon = 154.956085;
    sprintf (location, "LL & R's place");
    *tzone = -10.0;
  }
  else if (scom (what, "tampa")) {
    *lat = 28.0532;
    *lon = 82.4179;
    sprintf (location, "Tampa");
    *tzone = -5.0;
  }
  else if (scom (what, "albuquerque")) {
    *lat = 35.090274;
    *lon = 106.633386;
    sprintf (location, "Albuquerque");
    *tzone = -7.0;
  }
  else if (scom (what, "phoenix")) {
    *lat = 33.438;
    *lon = 111.999193;
    sprintf (location, "Phoenix");
    *tzone = -7.0;
  }
  else if (scom (what, "florence")) {
    *lat = DMS (43.0, 46.0, 8.0);
    *lon = -DMS (11.0, 14.0, 38.0);
    sprintf (location, "Florence");
    *tzone = 1.0;
  }
  else if (scom (what, "easterisland")) {
    *lat = -27.09;
    *lon = 109.26;
    sprintf (location, "Easter Island");
    *tzone = -6.0;  // ???
  }
  else if (scom (what, "stjohns")) {
    *lat = 47.6186;
    *lon = 52.7519;
    sprintf (location, "St. John's");
    *tzone = -8.0 + 4.5;
  }
  else if (scom (what, "farlain")) {
    // 44.8088472,-79.9711752
    *lat = 44.8088472;
    *lon = 79.9711752;
    sprintf (location, "Farlain Lake");
    *tzone = -5.0;
  }
  else if (scom (what, "bowlinggreen")) {
    *lat = DMS (37.0, 00.0, 0.0);   // accurate only to nearest minute
    *lon = DMS (86.0, 26.0, 0.0);    // Latitude 37° 00' N,   Longitude 86° 26' W
    sprintf (location, "Bowling Green");
    *tzone = -6.0;
  }
  else if (scom (what, "anchorage")) {
    *lat = DMS (61.0, 13.0, 0.0);   // accurate only to nearest minute
    *lon = DMS (149.0, 52.0, 0.0);
    sprintf (location, "Anchorage");
    *tzone = -9.0;
  }
  else if (scom (what, "augustine")) {
    *lat = DMS (59.0, 21.0, 45.31);   
    *lon = DMS (153.0, 26.0, 2.86);
    sprintf (location, "Augustine");
    *tzone = -9.0;
  }
  else if (scom (what, "churchpoint")) {
    *lat = 44.386635;  // from http://broadband.gc.ca/demographic_servlet/community_demographics/843
    *lon = 66.02554;
    sprintf (location, "Church Point, NS");
    *tzone = -4.0;
  }
  else if (scom (what, "sydney")) {
    *lat = -DMS (33.0, 55.0, 0.0);   // accurate only to nearest minute
    *lon = -DMS (151.0, 17.0, 0.0);
    sprintf (location, "Sydney, Australia");
    *tzone = 10.0;
  }
  else if (scom (what, "beijing")) {   // BJUT 39.87591, 116.48207
    *lat = 39.87591;
    *lon = -116.48207;
    sprintf (location, "BJUT, Beijing, China");
    *tzone = 8.0;
  }
  else if (scom (what, "xian")) {   // 34.265, 108.954
    *lat = 34.265;
    *lon = -108.954;
    sprintf (location, "Xi'an, China");
    *tzone = 8.0;
  }
  else if (scom (what, "nadi")) {
    // 18 08 S, 178 25 E (Suva, capital of Fiji)
    *lat = -DMS (17.0, 47.0, 0.0);   // accurate only to nearest minute
    *lon = -DMS (177.0, 29.0, 0.0);
    sprintf (location, "Nadi, Fiji");
    *tzone = 12.0;
  }
  else 
    error ("unknown location");
}

int main (int argc, char **argv) {
  time_t the_time;
  struct tm *current_time;
  int index, month, day, year;
  int month_local, day_local, year_local;
  int month_utc, day_utc, year_utc;
  double hour_local, hour_utc;
  double lat = DMS (49, 16, 17), lon = DMS (123, 10, 55);  // Paul Kry's house
  double time_zone = -8.0;   // PST
  double target_altitude;
  int setting = TRUE;
  char event [102], location [102];
  char date_not_set = TRUE;
  int long_output = TRUE;
  int hms_output = TRUE;
  target_altitude = -50.0 / 60.0;
  sprintf (event, "sunset");
  sprintf (location, "Vancouver");
  target_altitude = -50.0 / 60.0;
  
  time (&the_time);
  current_time = localtime (&the_time);
  month_local = current_time->tm_mon + 1;
  day_local = current_time->tm_mday;
  year_local = current_time->tm_year + 1900;
  hour_local = (double) current_time->tm_hour + ((double) current_time->tm_min + (double) current_time->tm_sec / 60.0) / 60.0;
  //printf ("LST is %f, %d %s %d\n", hour_local, day_local, month_name [month_local], year_local);

  double current_JD_tzoffset = JulianDay (hour_local * 3600.0, month_local, day_local, year_local);

  current_time = gmtime (&the_time);
  month_utc = current_time->tm_mon + 1;
  day_utc = current_time->tm_mday;
  year_utc = current_time->tm_year + 1900;
  hour_utc = (double) current_time->tm_hour + ((double) current_time->tm_min + (double) current_time->tm_sec / 60.0) / 60.0;
  //printf ("UTC is %f, %d %s %d\n", hour_utc, day_utc, month_name [month_utc], year_utc);

  double current_JD = JulianDay (hour_utc * 3600.0, month_utc, day_utc, year_utc);

  //printf ("Timezone offset is %f hours\n", (current_JD_tzoffset - current_JD) * 24.0);


  double paranoia_check = ((current_JD - JulianDay2 (1, 1.0, 1970)) * 24.0 * 3600.0 - (double) the_time) / 3600.0;

  if (ABS (paranoia_check) > 0.1) error ("failed paranoia check, please inform software vendor");


  for (index = 1; index < argc; index++) {
    if (scom (argv [index], "event")) {
      if (index > argc - 2) error ("too few arguments");
      get_event (event, &target_altitude, &setting, argv [index + 1]);
      index++;
    }
    else if (scom (argv [index], "location")) {
      if (index > argc - 2) error ("too few arguments");
      get_location (location, &lat, &lon, &time_zone, argv [index + 1]);
      index++;
    }
    else if (scom (argv [index], "kry")) {
      house_of_paul_kry (month_local, day_local, year_local);
      exit (5326526);
    }
    else if (scom (argv [index], "today")) {
      // set date to today
      month = month_local;
      day = day_local;
      year = year_local;
      date_not_set = FALSE;
    }
    else if (scom (argv [index], "tomorrow")) {
      // set date to today
      month = month_local;
      day = day_local + 1.0;
      year = year_local;
      date_not_set = FALSE;
    }
    else if (scom (argv [index], "savings")) {
      daylight_savings = 1.0;
    }
    else if (scom (argv [index], "west")) {
      if (index > argc - 2) error ("too few arguments");
      offset_location (location, &lat, &lon, argv [index + 1], WEST_EAST);
      index++;
    }
    else if (scom (argv [index], "north")) {
      if (index > argc - 2) error ("too few arguments");
      offset_location (location, &lat, &lon, argv [index + 1], NORTH_SOUTH);
      index++;
    }
    else if (scom (argv [index], "short")) 
      long_output = FALSE;
    else if (scom (argv [index], "zarvox")) {
      zarvox = true;
      long_output = FALSE;
    }
    else if (scom (argv [index], "decimal"))
      hms_output = FALSE;
    else 
      break;
  }
  if (argc == 3 && scom (argv [1], "location")) {
    printf ("%s\n", location);
    exit (2);
  }

  if (!allow_DST && daylight_savings > 0.0) {
    fprintf (stdout, "*** no DST for location!\n");
    exit (1);
  }
    
  if (index > argc - 2 && date_not_set) {
    fprintf (stdout, "Usage: %s  [options] day month [year]\n", argv [0]);
    fprintf (stdout, "   or  %s  [options] today\n", argv [0]);
    fprintf (stdout, "  where options can be\n");
    fprintf (stdout, "    event [set|rise|bcivil|ecivil|bnautical|enautical|bastronomical|eastronomical]\n");
    fprintf (stdout, "    location [vancouver|sanfrancisco|deathvalley|boston|winnipeg|calgary|edmonton|stonehenge|avebury|easterisland]\n");
    fprintf (stdout, "    savings\n");
    fprintf (stdout, " \n");
    fprintf (stdout, "Examples: \n");
    fprintf (stdout, "     %s event set location deathvalley today\n", argv [0]);
    fprintf (stdout, "     %s event noon location winnipeg 22 10 1959\n", argv [0]);
    fprintf (stdout, "    \n");
    fprintf (stdout, "  commands can be abbreviated:\n");
    fprintf (stdout, "     %s event set loc vanc today\n", argv [0]);
    fprintf (stdout, "     %s event set loc vanc savings today\n", argv [0]);
    fprintf (stdout, "        (use command `savings' for daylight savings time)\n");
    exit (1022);
  }
  if (date_not_set) {
    // Parse remaining arguments to find date
    day = atoi (argv [index]);
    if (day > 1000)
      error ("bad value for day");
    month = atoi (argv [index + 1]);
    if (month < 1 || month > 12) error ("bad value for month");
    year = 2020;
    if (index + 2 < argc) year = atoi (argv [index + 2]);
  }

  double LST;  

  LST = time_of_event (target_altitude, month, day, year, setting, lat, lon, time_zone);  // assuming standard sun and normal atmospheric refraction

  //printf ("ksldjfsdlk \n");
  LST += daylight_savings;

  if (long_output) {
    printf ("Time of %s from\n%s (lat %f, long %f), (tz %.1f)\non %d %s %d is ", 
	    event, location, lat, lon, time_zone, 
            day, month_name [month], year);
  }
  if (zarvox) 
    printf ("%s ", event);

  if (hms_output) {
    int nearest_minute = FALSE;  // MOVE THIS 
    int hour = (int) LST;
    double dminutes = (LST - (double) hour) * 60.0;
    if (nearest_minute) {
      printf ("%02dh %02dm\n",
	      hour, (int) (dminutes + 0.5));
    }
    else {
      int minutes = (int) dminutes;
      double seconds = (dminutes - (double) minutes) * 60.0;
      printf ("%02dh %02dm %.0fs\n", 
	      hour, minutes, seconds);
    }
  }
  else {
    if (long_output)
      printf ("%f\n", LST);
    else
      printf ("%.4f\n", LST);
  }
  
  return 0;
}



/*

azimuth / altitude  for Vancouver noon PDT
 * month  azimuth  altitude
	 * 4      153.5    44.5       
	 * 5      150.7    54.5
	 * 6      145.7    60.0
	 * 7      143.2    59.3
	 * 8      147.1    53.7
	 * 9      154.9    44.5
	
	 */
