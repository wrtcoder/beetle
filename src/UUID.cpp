/*
 * UUID.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: james
 */

#include "UUID.h"

#include <algorithm>
#include <assert.h>
#include <boost/format.hpp>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "ble/gatt.h"

const int UUID_LEN = 16;
const int SHORT_UUID_LEN = 2;

UUID::UUID() {
	uuid = {0};
}

UUID::UUID(uint8_t *value, size_t len) {
	assert(len == SHORT_UUID_LEN || len == UUID_LEN);
	if (len == SHORT_UUID_LEN) {
		memset(uuid.value, 0, UUID_LEN);
		memcpy(uuid.value + 2, value, len);
		memcpy(uuid.value + 4, BLUETOOTH_BASE_UUID, 12);
	} else {
		memcpy(uuid.value, value, UUID_LEN);
	}
}

UUID::UUID(std::string s) {
	s.erase(std::remove(s.begin(), s.end(), '-'), s.end());
	std::istringstream hex_chars_stream(s);
	int i = 0;
	unsigned int c;
	if (s.length() == 4) {
		memset(uuid.value, 0, UUID_LEN);
		memcpy(uuid.value + 4, BLUETOOTH_BASE_UUID, 12);
		while (hex_chars_stream >> std::hex >> c) {
			uuid.value[3 - i] = c;
			i++;
		}
	} else {
		while (hex_chars_stream >> std::hex >> c) {
			uuid.value[i] = c;
			i++;
		}
		c = uuid.value[0];
		uuid.value[0] = uuid.value[1];
		uuid.value[1] = c;
		c = uuid.value[2];
		uuid.value[2] = uuid.value[3];
		uuid.value[3] = c;
	}
}

UUID::UUID(uuid_t uuid_) {
	uuid = uuid_;
}

UUID::UUID(uint16_t val) {
	memset(uuid.value, 0, UUID_LEN);
	memcpy(uuid.value + 2, &val, 2);
	memcpy(uuid.value + 4, BLUETOOTH_BASE_UUID, 12);
}

UUID::~UUID() {

}

uuid_t UUID::get() {
	return uuid;
}

uint16_t UUID::getShort() {
	return *(uint16_t *)(uuid.value + 2);
}

bool UUID::isShort() {
	return memcmp(uuid.value + 4, BLUETOOTH_BASE_UUID, 12) == 0;
}

std::string UUID::str() {
	std::stringstream ss;
	if (isShort()) {
		ss << boost::format("%02x") % static_cast<int>(uuid.value[3]);
		ss << boost::format("%02x") % static_cast<int>(uuid.value[2]);
	} else {
		// TODO not sure why the first 4 bytes are big-endian
		ss << boost::format("%02x") % static_cast<int>(uuid.value[1]);
		ss << boost::format("%02x") % static_cast<int>(uuid.value[0]);
		ss << boost::format("%02x") % static_cast<int>(uuid.value[3]);
		ss << boost::format("%02x") % static_cast<int>(uuid.value[2]);
		for (int i = 4; i < UUID_LEN; i++) {
			if (i == 4 || i == 6 || i == 8 || i == 10) {
				ss << '-';
			}
			ss << boost::format("%02x") % static_cast<int>(uuid.value[i]);
		}
	}
	return ss.str();
}

bool UUID::compareTo(UUID &other) {
	return memcmp(uuid.value, other.get().value, sizeof(uuid_t)) == 0;
}
