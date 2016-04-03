/*
 * Device.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: james
 */

#include "Device.h"

#include <assert.h>
#include <bluetooth/bluetooth.h>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <cstring>
#include <vector>

#include "../ble/att.h"
#include "../ble/gatt.h"
#include "../ble/helper.h"
#include "../Debug.h"
#include "../hat/HAT.h"
#include "../Router.h"
#include "../UUID.h"

Device::Device(Beetle &beetle_) : beetle(beetle_) {

}

Device::~Device() {
	for (auto &kv : handles) {
		delete kv.second;
	}
	handles.clear();
}

int Device::getHighestHandle() {
	std::lock_guard<std::recursive_mutex> lg(handlesMutex);
	if (handles.size() > 0) {
		return handles.rbegin()->first;
	} else {
		return -1;
	}
}

void Device::unsubscribeAll(device_t d) {
	std::lock_guard<std::recursive_mutex> lg(handlesMutex);
	for (auto &kv : handles) {
		Handle *handle = kv.second;
		if (handle->getUuid().isShort() &&
				handle->getUuid().getShort() == GATT_CLIENT_CHARAC_CFG_UUID &&
				handle->subscribers.find(d) != handle->subscribers.end()) {
			handle->subscribers.erase(d);
			if (handle->subscribers.size() == 0) {
				int reqLen = 5;
				uint8_t req[reqLen];
				memset(req, 0, reqLen);
				req[0] = ATT_OP_WRITE_REQ;
				*(uint16_t *)(req + 1) = htobs(handle->getHandle());
				writeTransaction(req, reqLen, [](uint8_t *resp, int respLen) -> void {});
			}
		}
	}
}
