from django.shortcuts import render, render_to_response
from django.http import JsonResponse, HttpResponse
from django.db.models import Q
from django.core import serializers
from django.utils import timezone
from django.views.decorators.gzip import gzip_page
from django.views.decorators.http import require_GET

from .shared import query_can_map_static
from .models import Rule, RuleException, DynamicAuth, AdminAuth, UserAuth, \
	PasscodeAuth, NetworkAuth, Exclusive

from beetle.models import Principal, Gateway, Contact
from gatt.models import Service, Characteristic
from network.models import ConnectedGateway, ConnectedPrincipal, \
	ServiceInstance, CharInstance
from network.shared import get_gateway_helper, get_gateway_and_principal_helper

import dateutil.parser
import cronex
import json
import base64

def _get_dynamic_auth(rule, principal):
	result = []
	for auth in DynamicAuth.objects.filter(rule=rule).order_by("priority"):
		auth_obj = {
			"when" : auth.require_when,
		}
		if isinstance(auth, NetworkAuth):
			auth_obj["type"] = "network"
			auth_obj["ip"] = auth.ip_address
			auth_obj["priv"] = auth.is_private
		elif isinstance(auth, PasscodeAuth):
			auth_obj["type"] = "passcode"
		elif isinstance(auth, UserAuth):
			auth_obj["type"] = "user"
			if principal.owner.id == Contact.NULL:
				# Rule is unsatisfiable: there is user to 
				# authenticate
				return False, []
		elif isinstance(auth, AdminAuth):
			auth_obj["type"] = "admin"
			if auth.admin.id == Contact.NULL:
				# Rule is unsatisfiable: there is no admin
				return False, []
		else:
			continue
		result.append(auth_obj)
	
	return True, result

def _properties_equal(lhs, rhs):
	return set(lhs.properties) == set(rhs.properties)

def _get_minimal_rules(rules, cached_relations, ignore_properties=True):
	"""
	Returns the rules that are 'minimal'.
	"""
	if rules.count() == 0:
		return rules

	not_minimal_ids = set()
	# TODO: this is not particularly efficient... 
	# filling in upper triangular matrix of strict partial order.
	for lhs in rules.order_by("id"):
		for rhs in rules.filter(id__gte=lhs.id):
			key = (lhs.id, rhs.id)
			if key not in cached_relations:
				if ignore_properties or _properties_equal(lhs, rhs):
					lhs_lte_rhs = lhs.domain_lte(rhs)
					rhs_lte_lhs = rhs.domain_lte(lhs)
					if lhs_lte_rhs and rhs_lte_lhs:
						cached_relations[key] = 0
					elif lhs_lte_rhs:
						cached_relations[key] = -1
					elif rhs_lte_lhs:
						cached_relations[key] = 1
					else:
						cached_relations[key] = 0
				else:
					cached_relations[key] = 0
			cached_val = cached_relations[key]
			if cached_val == 0:
				pass
			elif cached_val > 0:
				not_minimal_ids.add(lhs.id)
			else:
				not_minimal_ids.add(rhs.id)

	for rule_id in not_minimal_ids:
		rules = rules.exclude(id=rule_id)

	return rules

@gzip_page
@require_GET
def query_can_map(request, from_gateway, from_id, to_gateway, to_id):
	"""
	Return whether fromId at fromGateway can connect to toId at toGateway
	"""

	if "timestamp" in request.GET:
		timestamp = dateutil.parser.parse(request.GET["timestamp"])
	else:
		timestamp = timezone.now()

	from_id = int(from_id)
	from_gateway, from_principal, conn_from_gateway, conn_from_principal = \
		get_gateway_and_principal_helper(from_gateway, from_id)

	to_id = int(to_id)
	to_gateway, to_principal, conn_to_gateway, conn_to_principal = \
		get_gateway_and_principal_helper(to_gateway, to_id)

	can_map, applicable_rules = query_can_map_static(from_gateway, 
		from_principal, to_gateway, to_principal, timestamp)
	
	response = {}
	if not can_map:
		response["result"] = False
		return JsonResponse(response)

	# Response format:
	# ================
	# {
	# 	"result" : True,
	# 	"access" : {
	# 		"rules" : {
	# 			1 : {					# Spec of the rule
	# 				"prop" : "rwni",
	# 				"int" : True,
	# 				"enc" : False,
	# 				"lease" : 1000,		# Expiration time
	#				"excl" : False,
	#				"dauth" : [
	#					... 			# Additional auth	
	#				]
	# 			}, 
	# 		},
	# 		"services": {
	# 			"2A00" : {				# Service
	# 				"2A01" : [1]		# Char to applicable rules
	#			},
	# 		},
	# 	}
	# }

	# Cached already computed relations for a strict partial order
	cached_relations = {}

	services = {}
	rules = {}
	for service_instance in ServiceInstance.objects.filter(
		principal=conn_from_principal):
		service_rules = applicable_rules.filter(
			Q(service=service_instance.service) | Q(service__name="*"))

		service = service_instance.service

		for char_instance in CharInstance.objects.filter(
			service=service_instance):
			
			characteristic = char_instance.char

			char_rules = service_rules.filter(Q(characteristic=characteristic) | 
				Q(characteristic__name="*"))

			for char_rule in _get_minimal_rules(char_rules, cached_relations):

				#####################################
				# Compute access per characteristic #
				#####################################

				# Access allowed
				if service.uuid not in services:
					services[service.uuid] = {}
				if characteristic.uuid not in services[service.uuid]:
					services[service.uuid][characteristic.uuid] = []
				if char_rule.id not in rules:
					satisfiable, dauth = _get_dynamic_auth(char_rule, 
						to_principal)
					if not satisfiable:
						continue

					if char_rule.exclusive:
 						exclusive_id = char_rule.exclusive.id
 					else:
 						exclusive_id = Exclusive.NULL

					# Put the rule in the result
					rules[char_rule.id] = {
						"prop" : char_rule.properties,
						"excl" : exclusive_id,
						"int" : char_rule.integrity,
						"enc" : char_rule.encryption,
						"lease" : (timestamp + char_rule.lease_duration
							).strftime("%s"),
						"dauth" : dauth,
					}

				services[service.uuid][characteristic.uuid].append(
					char_rule.id)
				
	if not rules:
		response["result"] = False
	else:
		response["result"] = True
		response["access"] = {
			"rules" : rules,
			"services" : services,
		}
	return JsonResponse(response)

@gzip_page
@require_GET
def view_rule_exceptions(request, rule):
	"""
	View in admin UI.
	"""
	response = []
	for exception in RuleException.objects.filter(rule__name=rule):
		response.append({
			"from_principal" : exception.from_principal.name,
			"from_gateway" : exception.from_gateway.name,
			"to_principal" : exception.to_principal.name,
			"to_gateway" : exception.to_gateway.name,
			"service" : exception.service.name,
			"characteristic" : exception.characteristic.name,
		})
	return JsonResponse(response, safe=False)

