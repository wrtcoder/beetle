/*
 * BeetleDevice.cpp
 *
 *  Created on: Apr 3, 2016
 *      Author: root
 */

#include "BeetleDevice.h"

#include <bluetooth/bluetooth.h>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <cstring>
#include <map>
#include <mutex>
#include <set>

#include "../ble/gatt.h"
#include "../ble/helper.h"
#include "../Debug.h"
#include "../Handle.h"
#include "../UUID.h"

BeetleDevice::BeetleDevice(Beetle &beetle, std::string name_) : Device(beetle, BEETLE_RESERVED_DEVICE) {
	name = name_;
	type = "Beetle";
	init();
}

BeetleDevice::~BeetleDevice() {

}

bool BeetleDevice::writeResponse(uint8_t *buf, int len) {
	throw DeviceException(getName() + " does not make requests");
}

bool BeetleDevice::writeCommand(uint8_t *buf, int len) {
	if (debug) {
		pdebug("Beetle received an unanticipated command");
	}
	return true;
}

bool BeetleDevice::writeTransaction(uint8_t *buf, int len, std::function<void(uint8_t*, int)> cb) {
	if (debug) {
		pdebug("Beetle received an unanticipated request");
	}
	uint8_t *resp;
	int respLen = writeTransactionBlocking(buf, len, resp);
	cb(resp, respLen);
	delete[] resp;
	return true;
}

/*
 * Should never get called. All reads and writes are serviced by the cache.
 */
int BeetleDevice::writeTransactionBlocking(uint8_t *buf, int len, uint8_t *&resp) {
	return pack_error_pdu(buf[0], 0, ATT_ECODE_UNLIKELY, resp); // TODO: probably not the right error code
}

void BeetleDevice::servicesChanged(handle_range_t range, device_t src) {
	if (debug) {
		pdebug("informing devices of service change");
	}

	boost::shared_lock<boost::shared_mutex> devicesLk(beetle.devicesMutex);
	std::lock_guard<std::recursive_mutex> handlesLg(handlesMutex);
	if (serviceChangedAttr->subscribers.size() == 0) {
		return;
	}
	uint8_t cmd[7];
	cmd[0] = ATT_OP_HANDLE_NOTIFY;
	*(uint16_t *)(cmd + 1) = htobs(serviceChangedAttr->getHandle());
	*(uint16_t *)(cmd + 3) = htobs(range.start);
	*(uint16_t *)(cmd + 5) = htobs(range.end);

	for (device_t dst : serviceChangedAttr->subscribers) {
		if (dst == src) {
			continue;
		}
		if (beetle.devices.find(dst) != beetle.devices.end()) {
			beetle.devices[dst]->writeCommand(cmd , sizeof(cmd));
		}
	}

}

void BeetleDevice::init() {
	std::lock_guard<std::recursive_mutex> lg(handlesMutex);
	uint16_t handleAlloc = 1; // 0 is special

	/*
	 * Setup GAP service with name characteristic
	 */
	Handle *gapServiceHandle = new PrimaryService();
	gapServiceHandle->setHandle(handleAlloc++);
	uint16_t *gapHandleUUID = new uint16_t;
	*gapHandleUUID = btohs(GATT_GAP_SERVICE_UUID);
	gapServiceHandle->cache.set((uint8_t *)gapHandleUUID, sizeof(uint16_t));
	gapServiceHandle->setCacheInfinite(true);
	handles[gapServiceHandle->getHandle()] = gapServiceHandle;

	Handle *gapDeviceNameCharHandle = new Characteristic();
	gapDeviceNameCharHandle->setHandle(handleAlloc++);
	gapDeviceNameCharHandle->setServiceHandle(gapServiceHandle->getHandle());
	uint8_t *gapDeviceNameCharHandleValue = new uint8_t[5];
	gapDeviceNameCharHandleValue[0] = GATT_CHARAC_PROP_READ;
	*(uint16_t *)(gapDeviceNameCharHandleValue + 3) = htobs(GATT_GAP_CHARAC_DEVICE_NAME_UUID);
	gapDeviceNameCharHandle->cache.set(gapDeviceNameCharHandleValue, 5);
	gapDeviceNameCharHandle->setCacheInfinite(true);
	handles[gapDeviceNameCharHandle->getHandle()] = gapDeviceNameCharHandle;

	Handle *gapDeviceNameAttrHandle = new Handle();
	gapDeviceNameAttrHandle->setHandle(handleAlloc++);
	gapDeviceNameAttrHandle->setUuid(UUID(GATT_GAP_CHARAC_DEVICE_NAME_UUID));
	gapDeviceNameAttrHandle->setServiceHandle(gapServiceHandle->getHandle());
	gapDeviceNameAttrHandle->setCharHandle(gapDeviceNameCharHandle->getHandle());
	uint8_t *gapDeviceNameAttrHandleValue = new uint8_t[name.length()];
	memcpy(gapDeviceNameAttrHandleValue, name.c_str(), name.length());
	gapDeviceNameAttrHandle->cache.set(gapDeviceNameAttrHandleValue, name.length());
	gapDeviceNameAttrHandle->setCacheInfinite(true);
	handles[gapDeviceNameAttrHandle->getHandle()] = gapDeviceNameAttrHandle;

	// fill in attr handle for characteristic
	gapDeviceNameCharHandle->setCharHandle(gapDeviceNameAttrHandle->getHandle());
	*(uint16_t *)(gapDeviceNameCharHandleValue + 1) = htobs(gapDeviceNameAttrHandle->getHandle());

	// end the characteristic
	gapDeviceNameCharHandle->setEndGroupHandle(gapDeviceNameAttrHandle->getHandle());

	// end the service
	gapServiceHandle->setEndGroupHandle(gapDeviceNameAttrHandle->getHandle());

	/*
	 * Setup GATT service with service changed characteristic
	 */
	Handle *gattServiceHandle = new PrimaryService();
	gattServiceHandle->setHandle(handleAlloc++);
	uint16_t *gattHandleUUID = new uint16_t;
	*gattHandleUUID = btohs(GATT_GATT_SERVICE_UUID);
	gattServiceHandle->cache.set((uint8_t *)gattHandleUUID, sizeof(uint16_t));
	gattServiceHandle->setCacheInfinite(true);
	handles[gattServiceHandle->getHandle()] = gattServiceHandle;

	Handle *gattServiceChangedCharHandle = new Characteristic();
	gattServiceChangedCharHandle->setHandle(handleAlloc++);
	gattServiceChangedCharHandle->setServiceHandle(gattServiceHandle->getHandle());
	uint8_t *gattServiceChangedCharHandleValue = new uint8_t[5];
	gattServiceChangedCharHandleValue[0] = GATT_CHARAC_PROP_NOTIFY;
	*(uint16_t *)(gattServiceChangedCharHandleValue + 3) = htobs(GATT_GATT_CHARAC_SERVICE_CHANGED_UUID);
	gattServiceChangedCharHandle->cache.set(gattServiceChangedCharHandleValue, 5);
	gattServiceChangedCharHandle->setCacheInfinite(true);
	handles[gattServiceChangedCharHandle->getHandle()] = gattServiceChangedCharHandle;

	Handle *gattServiceChangedAttrHandle = new Handle();
	gattServiceChangedAttrHandle->setHandle(handleAlloc++);
	gattServiceChangedAttrHandle->setUuid(UUID(GATT_GATT_CHARAC_SERVICE_CHANGED_UUID));
	gattServiceChangedAttrHandle->setServiceHandle(gattServiceHandle->getHandle());
	gattServiceChangedAttrHandle->setCharHandle(gattServiceChangedCharHandle->getHandle());
	gattServiceChangedAttrHandle->cache.set(NULL, 0);
	gattServiceChangedAttrHandle->setCacheInfinite(false);
	handles[gattServiceChangedAttrHandle->getHandle()] = gattServiceChangedAttrHandle;

	// fill in attr handle for characteristic
	gattServiceChangedCharHandle->setCharHandle(gattServiceChangedAttrHandle->getHandle());
	*(uint16_t *)(gattServiceChangedCharHandleValue + 1) = htobs(gattServiceChangedAttrHandle->getHandle());

	Handle *gattServiceChangedCfgHandle = new ClientCharCfg();
	gattServiceChangedCfgHandle->setHandle(handleAlloc++);
	gattServiceChangedCfgHandle->setServiceHandle(gattServiceHandle->getHandle());
	gattServiceChangedCfgHandle->setCharHandle(gattServiceChangedCharHandle->getHandle());
	gattServiceChangedCfgHandle->cache.set(NULL, 0);
	gattServiceChangedCfgHandle->setCacheInfinite(false);
	handles[gattServiceChangedCfgHandle->getHandle()] = gattServiceChangedCfgHandle;

	// end the characteristic
	gattServiceChangedCharHandle->setEndGroupHandle(gattServiceChangedCfgHandle->getHandle());
	// end the service
	gattServiceHandle->setEndGroupHandle(gattServiceChangedCfgHandle->getHandle());

	// save the service changed attr handle
	serviceChangedAttr = gattServiceChangedAttrHandle;
}

