class JulianDay:

    def __init__(self, seconds: float, month: int, day: int, year: int):
        self.seconds = seconds
        self.month = month
        self.day = day
        self.year = year

    def as_float(self):
        month = self.month + 12 if self.month < 3 else self.month
        year = self.year - 1 if self.month < 3 else self.year
        a = int(year/100)
        b = int(2 - a + a / 4)
        return float(
            int(365.25 * float(year + 4716.0)) +
            int(30.6001 * (float(month + 1.0)) + self.day + b) -
            1524.5 + self.seconds / (24.0 * 3600.0)
        )

class AstroTime:
    pass
