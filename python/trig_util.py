class TrigUtil:
    DEG_TO_RAD = float(0.0174532925199432957692)  # (PI / 180)
    RAD_TO_DEG = float(57.2957795130823208768)  # (180/PI)

    @staticmethod
    def to_360(ang: float) -> float:
        if 0.0 <= ang < 360.0:
            return ang
        if ang >= 360.0:
            return TrigUtil.to_360(ang - 360.0)
        else:
            return TrigUtil.to_360(ang + 360.0)
