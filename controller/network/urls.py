from django.conf.urls import include, url

from beetle.regex import device, gateway
from gatt.regex import uuid

from . import regex

from . import views_user as user
from . import views_api as api

urlpatterns = [

	# User facing views

	url(r'^view/' + device("device") + r'$', 
		user.view_device, 
		name='view the services and characteristics of an device'),

	url(r'^find/gateway/' + gateway("gateway") + r'$', 
		user.find_gateway, name='find a gateway by name'),
	
	url(r'^discover/devices$', user.discover_devices, 
		name='discover devices'),
	url(r'^discover/service/' + uuid("uuid") + r'$', 
		user.discover_with_uuid, 
		{ "is_service" : True },
		name='discover devices with the service uuid'),
	url(r'^discover/char/' + uuid("uuid") + r'$', 
		user.discover_with_uuid,  
		{ "is_service" : False },
		name='discover devices with the characteristic uuid'),

	# Internal API views

	url(r'^connect/' + gateway("gateway") + r'$', 
		api.connect_gateway,
	 	name='connect a gateway'),

	url(r'^connect/' + gateway("gateway") + r'/' + device("device") + r'/' 
		+ regex.device_id("remote_id") + r'$', 
		api.connect_device, name='connect an device'),

	url(r'^update/' + gateway("gateway") + r'/' + regex.device_id("remote_id") 
		+ r'$', 
		api.update_device, name='update an device'),

	url(r'^map/' + gateway("from_gateway") + r'/' + regex.device_id("from_id") 
		+ r'/' + gateway("to_gateway") + r'/' + regex.device_id("to_id") 
		+ r'$',
		api.map_devices, name='update mappings on device'),

	url(r'^registerInterest/service/' + gateway("gateway") + r'/' 
		+ regex.device_id("remote_id") + r'$', 
		api.register_interest,
		{"is_service" : True},
		name='register interest in services or characteristic'),

	url(r'^registerInterest/char/' + gateway("gateway") + r'/' 
		+ regex.device_id("remote_id") + r'$', 
		api.register_interest,
		{"is_service" : False},
		name='register interest in services or characteristic'),
]