class EarthLocation:
    """
    NOTE: The value for longitude is negated, making WEST positive.
    """
    def __init__(self, lat: float, lon: float, utc_offset, name=None):
        self.lat = lat
        self.lon = lon
        self.utc_offset = utc_offset
        self.name = name




