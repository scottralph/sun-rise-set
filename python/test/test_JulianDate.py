from unittest import TestCase

from JulianDate import JulianDay


class TestJulianDay(TestCase):
    def test_as_float(self):
        jd = JulianDay(55912.0, 12, 27, 2020)
        computed = jd.as_float()
        self.assertLess(abs(computed-2459211.147130), 0.0001)
