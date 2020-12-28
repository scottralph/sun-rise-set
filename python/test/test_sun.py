from unittest import TestCase

from EarthLocation import EarthLocation
from JulianDate import JulianDay
from sun import Sun


class TestSun(TestCase):
    def test_true_altitude_centre_of_disk_of_sun(self):
        jd = JulianDay(87641.463661, 12, 27, 2020)
        vancouver = EarthLocation(49.257431, 123.146353)

        apparent_horizon = Sun.true_altitude_centre_of_disk_of_sun(jd, vancouver)
        expected = -0.833335
        self.assertTrue(abs(apparent_horizon - expected) < 0.0001)
