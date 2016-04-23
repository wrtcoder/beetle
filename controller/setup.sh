#!/bin/bash

./manage.py migrate
./manage.py makemigrations beetle
./manage.py makemigrations gatt
./manage.py makemigrations access
./manage.py makemigrations network
./manage.py migrate

./manage.py loaddata initial_data
./manage.py loaddata default_services
./manage.py loaddata default_characteristics

./manage.py createsuperuser