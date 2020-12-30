# Conversion of the C++ code into Python

from datetime import date
from math import sin, cos, atan2, asin

from EarthLocation import EarthLocation
from JulianDate import JulianDay
from event import SunEvent
from trig_util import TrigUtil


class Sun:

    @staticmethod
    def dms_sec(d, m, s) -> float:
        return s + m * 60.0 + d * 3600.0

    @staticmethod
    def delta_t(jd: float) -> float:
        """
        :param jd:
        :return: TD - UT the difference between Terrestrial Dynamical Time and Universal Time (in seconds).
        """
        JD_1jan2000 = float(2451544.5)
        delta_year = float((jd - JD_1jan2000) / 365.25)

        return 65.0 + delta_year  # from 1998 to 2003, assume a second per year

    @staticmethod
    def true_altitude_centre_of_disk_of_sun(universal_time_t1_sec_since_0_ut: JulianDay,
                                            location: EarthLocation) -> float:
        jd = universal_time_t1_sec_since_0_ut.as_float()  # Julian Day (based on Universal Time)
        ut = universal_time_t1_sec_since_0_ut.seconds
        td = ut + Sun.delta_t(
            jd)  # Terrestrial Dynamical Time (A uniform time scale, TD = TAI + 32.184 sec., where TAI is \
        # Terrestrial Atomic Time)

        # Julian Ephemeris Day (based on Terrestrial Dynamical Time)
        jde = JulianDay(td,
                        universal_time_t1_sec_since_0_ut.month,
                        universal_time_t1_sec_since_0_ut.day,
                        universal_time_t1_sec_since_0_ut.year).as_float()

        t = float((jde - 2451545.0) / 36525.0)  # Time, in Julian centuries of 36525 ephemeris days from  epoch J2000.0

        # Same as T, but using JD instead of JDE
        tut = float((jd - 2451545.0) / 36525.0)

        # Geometric mean longitude of the Sun, referred to the mean equinox of the date (degrees)
        l0 = float(280.46646 + 36000.76983 * t + 0.0003032 * t * t)

        # Mean anomaly of the Sun
        m = float(357.52911 + 35999.05029 * t - 0.0001537 * t * t)

        # Eccentricity of the Earth's orbit
        e = float(0.016708634 - 0.000042037 * t - 0.0000001267 * t * t)

        # Equation of the centre of Sun
        c = float((1.914602 - 0.004817 * t - 0.000014 * t * t) * sin(TrigUtil.DEG_TO_RAD * m) + (
                0.019993 - 0.000101 * t) * sin(TrigUtil.DEG_TO_RAD * 2.0 * m) + 0.000289 * sin(
            TrigUtil.DEG_TO_RAD * 3.0 * m))

        #  Sun's true longitude
        circle_dot = TrigUtil.to_360(l0 + c)

        nu = m + c  # Sun's true anomaly

        # Sun's radius vector
        r = 1.000001018 * (1.0 - e * e) / (1.0 + e * cos(TrigUtil.DEG_TO_RAD * nu))

        omega = 125.04 - 1934.136 * t

        #  Sun's apparent longitude
        sun_lambda = circle_dot - 0.00569 - 0.00478 * sin(TrigUtil.DEG_TO_RAD * omega)

        #  Mean obliquity of the ecliptic  (degrees)
        epsilon0 = (Sun.dms_sec(23.0, 26.0, 21.448) - 46.8150 * t - 0.00059 * t * t + 0.001813 * t * t * t) / 3600.0

        # True obliquity of the ecliptic  (degrees)
        epsilon = epsilon0 + 0.00256 * cos(TrigUtil.DEG_TO_RAD * omega)

        alpha_app = TrigUtil.RAD_TO_DEG * atan2(
            cos(TrigUtil.DEG_TO_RAD * epsilon) * sin(TrigUtil.DEG_TO_RAD * sun_lambda),
            cos(TrigUtil.DEG_TO_RAD * sun_lambda))
        delta_app = TrigUtil.RAD_TO_DEG * asin(
            sin(TrigUtil.DEG_TO_RAD * epsilon) * sin(TrigUtil.DEG_TO_RAD * sun_lambda))

        #  Longitude of the ascending node of the Moon's mean orbit on the ecliptic,
        #  measured from the mean equinox of the date
        omega_moon = 125.04452 - 1934.136261 * t + 0.0020708 * t * t + t * t * t / 450000.0

        # Mean longitude of the moon
        l_moon = 218.3165 + 481267.8813 * t  # ????

        # Nutation in longitude (seconds of arc)
        nut_long = -17.20 * sin(TrigUtil.DEG_TO_RAD * omega_moon) - 1.32 * sin(
            TrigUtil.DEG_TO_RAD * 2.0 * l0) - 0.23 * sin(TrigUtil.DEG_TO_RAD * 2.0 * l_moon) + 0.21 * sin(
            TrigUtil.DEG_TO_RAD * 2.0 * omega_moon)  # ????

        # Mean sidereal time at Greenwich (degrees)
        theta0 = 280.46061837 + 360.98564736629 * (
                jd - 2451545.0) + 0.000387933 * tut * tut - tut * tut * tut / 38710000.0  # Should use UT, not TD

        # Apparent sidereal time at Greenwich (degrees)
        theta = theta0 + nut_long * cos(TrigUtil.DEG_TO_RAD * epsilon) / 3600.0

        # local hour angle
        local_hour_angle = theta0 - location.lon - alpha_app

        # apparent altitude
        h = TrigUtil.RAD_TO_DEG * asin(sin(TrigUtil.DEG_TO_RAD * location.lat) * sin(TrigUtil.DEG_TO_RAD * delta_app) +
                                       cos(TrigUtil.DEG_TO_RAD * location.lat) * cos(
            TrigUtil.DEG_TO_RAD * delta_app) * cos(TrigUtil.DEG_TO_RAD * local_hour_angle))
        return h

    @staticmethod
    def time_of_event_decimal_hours(sun_event: SunEvent,
                                    event_date: date,
                                    earth_location: EarthLocation) -> float:
        target_altitude = sun_event.target_altitude
        ut_low = ((12.0 if sun_event.setting else 0.0) - earth_location.utc_offset) * 3600.0
        ut_high = ((24.0 if sun_event.setting else 12.0) - earth_location.utc_offset) * 3600.0
        ut = (ut_low, ut_high)

        def take_step(pr):
            center = (pr[0] + pr[1]) / 2.0
            jd = JulianDay(center, event_date.month, event_date.day, event_date.year)
            alt = Sun.true_altitude_centre_of_disk_of_sun(jd, earth_location)
            replace_left = (alt > target_altitude and sun_event.setting) or (
                        alt <= target_altitude and not sun_event.setting)
            new_pr = (center, pr[1]) if replace_left else (pr[0], center)
            return new_pr

        while (ut[1] - ut[0]) > 0.01:
            ut = take_step(ut)
        return (ut[0] + ut[1]) / 7200.0 + earth_location.utc_offset
