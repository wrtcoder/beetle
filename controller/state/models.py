from __future__ import unicode_literals

from django.db import models
from django.utils import timezone

# Create your models here.

class AdminAuthInstance(models.Model):
	"""Instance of authorization from admin."""

	class Meta:
		verbose_name = "Admin Authorization (Record)"
		verbose_name_plural = verbose_name
		unique_together = ("rule", "principal")

	rule = models.ForeignKey("access.Rule")
	principal = models.ForeignKey("beetle.VirtualDevice")
	allow = models.BooleanField(default=True)

	timestamp = models.DateTimeField(auto_now_add=True)
	expire = models.DateTimeField(default=timezone.now)

	def __unicode__(self):
		return "%d %s" % (self.rule.id, self.principal.name)

class UserAuthInstance(models.Model):
	"""Instance of authentication of user."""

	class Meta:
		verbose_name = "User Authentication (Record)"
		verbose_name_plural = verbose_name
		unique_together = ("rule", "principal")

	rule = models.ForeignKey("access.Rule")
	principal = models.ForeignKey("beetle.VirtualDevice")
	allow = models.BooleanField(default=True)

	timestamp = models.DateTimeField(auto_now_add=True)
	expire = models.DateTimeField(default=timezone.now)

	def __unicode__(self):
		return "%d %s" % (self.rule.id, self.principal.name)

class PasscodeAuthInstance(models.Model):
	"""Instance of password authentication."""

	class Meta:
		verbose_name = "Passcode Authentication (Record)"
		verbose_name_plural = verbose_name
		unique_together = ("rule", "principal")

	rule = models.ForeignKey("access.Rule")
	principal = models.ForeignKey("beetle.VirtualDevice")
	timestamp = models.DateTimeField(auto_now_add=True)
	expire = models.DateTimeField(default=timezone.now)

	def __unicode__(self):
		return "%d %s" % (self.rule.id, self.principal.name)

class ExclusiveLease(models.Model):
	"""A lease of exclusivity"""

	class Meta:
		verbose_name = "Exclusive (Lease)"
		verbose_name_plural = verbose_name

	group = models.OneToOneField("access.Exclusive")
	device_instance = models.ForeignKey("network.ConnectedDevice")
	timestamp = models.DateTimeField(auto_now_add=True)
	expire = models.DateTimeField(default=timezone.now)

	def __unicode__(self):
		return "%d %s" % (self.group.id, self.device_instance.name)
