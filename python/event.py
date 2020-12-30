class SunEvent:
    def __init__(self, target_altitude, setting):
        self.target_altitude = target_altitude
        self.setting = setting

    @staticmethod
    def create(desc):
        if desc.lower() in ["sunrise", "rise"]:
            return SunRiseEvent()
        elif desc.lower() in ["sunset", "set"]:
            return SunSetEvent()
        else:
            raise RuntimeError(f"Unknown event type  {desc}")


class SunSetEvent(SunEvent):

    def __init__(self):
        super().__init__(-50.0/60.0, True)

    def __str__(self):
        return "Sunset"


class SunRiseEvent(SunEvent):

    def __init__(self):
        super().__init__(-50.0/60.0, False)

    def __str__(self):
        return "Sunrise"



