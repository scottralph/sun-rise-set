from datetime import date

import flask
from flask import request, jsonify

from EarthLocation import EarthLocation
from event import SunEvent
from sun import Sun

app = flask.Flask("sun")


def error_obj(mesg):
    return jsonify({'status': 'failed',
                    'message': mesg,
                    'help': 'Usage: event=[sunrise|rise|sunset|set],lat=float,lon=float,utc_offset=float,year=YYYY,month=M,day=d'})


@app.route('/', methods=['GET'])
def home():
    if 'event' not in request.args:
        return error_obj('No event found')
    event = request.args['event']
    if event.lower() not in ['set', 'rise', 'sunset', 'sunrise']:
        return error_obj('Event must be one of [set,sunset,rise,sunrise')
    if 'lon' not in request.args or 'lat' not in request.args:
        return error_obj('Must specify lat and lon coordinates')
    lat = float(request.args['lat'])
    lon = -float(request.args['lon'])
    if 'utc_offset' not in request.args:
        return error_obj('No uct_offset specified')
    utc_offset = float(request.args['utc_offset'])

    if not 'year' in request.args or not 'month' in request.args or not 'day' in request.args:
        return error_obj('Request must contain year, month, and day arguments.')
    year = int(request.args['year'])
    month = int(request.args['month'])
    day = int(request.args['day'])

    sun_event = SunEvent.create(event)
    earth_location = EarthLocation(lat, lon, utc_offset)
    event_decimal_hours = Sun.time_of_event_decimal_hours(sun_event, date(year, month, day), earth_location)
    time_str = Sun.from_decimal_to_hms(event_decimal_hours)


    return jsonify({
        'status': 'success',
        'date': {'year': year, 'month': month, 'day': day},
        'utc_offset': utc_offset,
        'event': sun_event.__str__(),
        'time_str': time_str})

app.run()

