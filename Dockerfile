FROM python:3.9-slim

ENV APP_HOME /app
WORKDIR $APP_HOME

COPY python/flask_app.py /app
COPY python/julian_date.py /app
COPY python/earth_location.py /app
COPY python/sun.py /app
COPY python/event.py /app
COPY python/trig_util.py /app


RUN pip install flask

CMD python3 /app/flask_app.py
