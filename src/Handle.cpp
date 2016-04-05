/*
 * Handle.cpp
 *
 *  Created on: Apr 2, 2016
 *      Author: root
 */

#include "Handle.h"

#include <bluetooth/bluetooth.h>
#include <sstream>

#include "ble/gatt.h"

void CachedHandle::set(uint8_t *value, int len) {
	if (this->value != NULL) {
		delete[] this->value;
	}
	this->value = value;
	this->len = len;
	::time(&(this->time));
}

Handle::Handle() {

}

Handle::~Handle() {

}

bool Handle::isCacheInfinite() {
	return cacheInfinite;
}

void Handle::setCacheInfinite(bool cacheInfinite) {
	this->cacheInfinite = cacheInfinite;
}

uint16_t Handle::getCharHandle() {
	return charHandle;
}

void Handle::setCharHandle(uint16_t charHandle) {
	this->charHandle = charHandle;
}

uint16_t Handle::getEndGroupHandle() {
	return endGroupHandle;
}

void Handle::setEndGroupHandle(uint16_t endGroupHandle) {
	this->endGroupHandle = endGroupHandle;
}

uint16_t Handle::getHandle() {
	return handle;
}

void Handle::setHandle(uint16_t handle) {
	this->handle = handle;
}

uint16_t Handle::getServiceHandle() {
	return serviceHandle;
}

void Handle::setServiceHandle(uint16_t serviceHandle) {
	this->serviceHandle = serviceHandle;
}

UUID& Handle::getUuid() {
	return uuid;
}

void Handle::setUuid(UUID uuid_) {
	uuid = uuid_;
}

std::string Handle::str() {
	std::stringstream ss;
	ss << handle << "\t" << uuid.str() << "\tsH" << serviceHandle
			<< "\tcH" << charHandle << "\tcache: [";
	std::string sep = "";
	for (int i = 0; i < cache.len; i++) {
		ss << sep << (unsigned int) cache.value[i];
		sep = ",";
	}
	ss << ']';
	return ss.str();
}

PrimaryService::PrimaryService() {
	uuid = UUID(GATT_PRIM_SVC_UUID);
}

std::string PrimaryService::str() {
	std::stringstream ss;
	ss << handle << "\t" << "[PrimaryService]" << "\tuuid=";
	ss << UUID(cache.value, cache.len).str();
	return ss.str();
}

Characteristic::Characteristic() {
	uuid = UUID(GATT_CHARAC_UUID);
}

static std::string getPropertiesString(uint8_t properties) {
	std::stringstream ss;
	ss << ((properties & GATT_CHARAC_PROP_BCAST) ? 'b' : '-');
	ss << ((properties & GATT_CHARAC_PROP_READ) ? 'r' : '-');
	ss << ((properties & GATT_CHARAC_PROP_WRITE_NR) ? 'W' : '-');
	ss << ((properties & GATT_CHARAC_PROP_WRITE) ? 'w' : '-');
	ss << ((properties & GATT_CHARAC_PROP_NOTIFY) ? 'n' : '-');
	ss << ((properties & GATT_CHARAC_PROP_IND) ? 'i' : '-');
	ss << ((properties & GATT_CHARAC_PROP_AUTH_SIGN_WRITE) ? 's' : '-');
	ss << ((properties & GATT_CHARAC_PROP_EXT) ? 'e' : '-');
	return ss.str();
}

std::string Characteristic::str() {
	std::stringstream ss;
	ss << handle << "\t" << "[Characteristic]" << "\tsH=" << serviceHandle << "\t";
	if (cache.value != NULL && cache.len >= 5) {
		uint8_t properties = cache.value[0];
		ss << "vH=" << btohs(*(uint16_t *)(cache.value + 1)) << "\t"
				<< getPropertiesString(properties);
	} else {
		ss << "unknown or malformed";
	}
	return ss.str();
}

ClientCharCfg::ClientCharCfg() {
	uuid = UUID(GATT_CLIENT_CHARAC_CFG_UUID);
}

std::string ClientCharCfg::str() {
	std::stringstream ss;
	std::string sep = "";
	ss << handle << "\t" << "[CliCharCfg]" << "\tsH" << serviceHandle
			<< "\tcH" << charHandle << "\tn=" << subscribers.size()
			<< "\tsub=[";
	for (device_t d : subscribers) {
		ss << sep << d;
		sep = ",";
	}
	ss << "]";
	return ss.str();
}


