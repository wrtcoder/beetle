/*
 * Handle.cpp
 *
 *  Created on: Apr 2, 2016
 *      Author: James Hong
 */

#include "Handle.h"

#include <bluetooth/bluetooth.h>
#include <boost/smart_ptr/shared_array.hpp>
#include <sstream>

#include "ble/gatt.h"

CachedHandle::~CachedHandle() {

}

void CachedHandle::set(boost::shared_array<uint8_t> value_, int len_) {
	value = value_;
	len = len_;
	time = ::time(NULL);
}

void CachedHandle::clear() {
	value.reset();
	len = 0;
	time = ::time(NULL);
}

Handle::Handle(bool staticHandle_, bool cacheInfinite_) {
	staticHandle = staticHandle_;
	cacheInfinite = cacheInfinite_;
}

Handle::~Handle() {

}

bool Handle::isStaticHandle() const {
	return staticHandle;
}

bool Handle::isCacheInfinite() const {
	return cacheInfinite;
}

uint16_t Handle::getCharHandle() const {
	return charHandle;
}

void Handle::setCharHandle(uint16_t charHandle) {
	this->charHandle = charHandle;
}

uint16_t Handle::getEndGroupHandle() const {
	return endGroupHandle;
}

void Handle::setEndGroupHandle(uint16_t endGroupHandle) {
	this->endGroupHandle = endGroupHandle;
}

uint16_t Handle::getHandle() const {
	return handle;
}

void Handle::setHandle(uint16_t handle) {
	this->handle = handle;
}

uint16_t Handle::getServiceHandle() const {
	return serviceHandle;
}

void Handle::setServiceHandle(uint16_t serviceHandle) {
	this->serviceHandle = serviceHandle;
}

UUID Handle::getUuid() const {
	return uuid;
}

void Handle::setUuid(UUID uuid_) {
	uuid = uuid_;
}

std::string Handle::str() const {
	std::stringstream ss;
	ss << handle << "\t" << uuid.str() << "\tsH=" << serviceHandle << "\tcH=" << charHandle;
	if (cache.value != NULL) {
		ss << "\tcache: [";
		std::string sep = "";
		for (int i = 0; i < cache.len; i++) {
			ss << sep << (unsigned int) cache.value.get()[i];
			sep = ",";
		}
		ss << ']';
	}
	return ss.str();
}

CharacteristicValue::CharacteristicValue(bool staticHandle, bool cacheInfinite)
	: Handle(staticHandle, cacheInfinite) {

}

std::string CharacteristicValue::str() const {
	std::set<device_t> subscribers;
	subscribers.insert(subscribersNotify.cbegin(), subscribersNotify.cend());
	subscribers.insert(subscribersIndicate.cbegin(), subscribersIndicate.cend());

	std::stringstream ss;
	ss << handle << "\t" << uuid.str() << "\tsH=" << serviceHandle << "\tcH=" << charHandle << "\tnSub="
			<< subscribers.size();
	if (!subscribers.empty()) {
		ss << "\tsub=[";
		std::string sep = "";
		for (device_t d : subscribers) {
			ss << sep << d;
			sep = ",";
		}
		ss << "]";
	}
	if (cache.value != NULL) {
		ss << "\tcache: [";
		std::string sep = "";
		for (int i = 0; i < cache.len; i++) {
			ss << sep << (unsigned int) cache.value.get()[i];
			sep = ",";
		}
		ss << ']';
	}
	return ss.str();
}

PrimaryService::PrimaryService() : Handle(true, true) {
	uuid = UUID(GATT_PRIM_SVC_UUID);
}

UUID PrimaryService::getServiceUuid() const {
	return UUID(cache.value.get(), cache.len);
}

std::string PrimaryService::str() const {
	std::stringstream ss;
	ss << handle << "\t" << "[PrimaryService]" << "\tuuid=" << UUID(cache.value.get(), cache.len).str() << "\tend="
			<< endGroupHandle;
	return ss.str();
}

Characteristic::Characteristic() : Handle(true, true) {
	uuid = UUID(GATT_CHARAC_UUID);
}

uint16_t Characteristic::getAttrHandle() const {
	return *(uint16_t *) (cache.value.get() + 1);
}

UUID Characteristic::getCharUuid() const {
	return UUID(cache.value.get() + 3, cache.len - 3);
}

uint8_t Characteristic::getProperties() const {
	return cache.value.get()[0];
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

std::string Characteristic::str() const {
	std::stringstream ss;
	ss << handle << "\t" << "[Characteristic]" << "\tsH=" << serviceHandle << "\t";
	if (cache.value != NULL && cache.len >= 5) {
		uint8_t properties = cache.value.get()[0];
		ss << "uuid=" << UUID(cache.value.get() + 3, cache.len - 3).str() << "\tvH="
				<< btohs(*(uint16_t * )(cache.value.get() + 1))
				<< "\t" << getPropertiesString(properties) << "\tend=" << endGroupHandle;
	} else {
		ss << "unknown or malformed";
	}
	return ss.str();
}

ClientCharCfg::ClientCharCfg() : Handle(false, false) {
	uuid = UUID(GATT_CLIENT_CHARAC_CFG_UUID);
}

std::string ClientCharCfg::str() const {
	std::stringstream ss;
	ss << handle << "\t" << "[ClientCharCfg]" << "\tsH=" << serviceHandle << "\tcH=" << charHandle;
	return ss.str();
}

