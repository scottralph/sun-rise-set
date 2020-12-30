from unittest import TestCase

from EarthLocation import EarthLocation
from JulianDate import JulianDay
from event import SunSetEvent, SunRiseEvent
from sun import Sun
from datetime import date


class TestSun(TestCase):
    def test_true_altitude_centre_of_disk_of_sun(self):
        jd = JulianDay(87641.463661, 12, 27, 2020)
        vancouver = EarthLocation(49.257431, 123.146353, -8.0)

        apparent_horizon = Sun.true_altitude_centre_of_disk_of_sun(jd, vancouver)
        expected = -0.833335
        self.assertTrue(abs(apparent_horizon - expected) < 0.0001)

    def test_event_time_sunset(self):
        sun_event = SunSetEvent()

        setting_time = Sun.time_of_event_decimal_hours(sun_event,
                                                       date(2020, 12, 30),
                                                       EarthLocation(49.257431, 123.146353, -8.0))
        self.assertLess(abs(setting_time - 16.388608),  0.001)

    def test_event_time_sunrise(self):
        sun_event = SunRiseEvent()

        setting_time = Sun.time_of_event_decimal_hours(sun_event,
                                                       date(2020, 12, 30),
                                                       EarthLocation(49.257431, 123.146353, -8.0))
        self.assertLess(abs(setting_time - 8.129552), 0.001)

    def test_hms(self):
        hms = Sun.from_decimal_to_hms(8.129552)
        self.assertEqual(hms, '08h 07m 46s')

