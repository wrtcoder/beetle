/*
 * NetworkReporter.h
 *
 *  Created on: Apr 16, 2016
 *      Author: james
 */

#ifndef INCLUDE_CONTROLLER_NETWORKREPORTER_H_
#define INCLUDE_CONTROLLER_NETWORKREPORTER_H_

#include "Beetle.h"
#include "ControllerClient.h"

/*
 * Reports devices connected and disconnected.
 */
class NetworkStateClient {
public:
	NetworkStateClient(Beetle &beetle, ControllerClient &client, int tcpPort);
	virtual ~NetworkStateClient();

	AddDeviceHandler getAddDeviceHandler();
	RemoveDeviceHandler getRemoveDeviceHandler();
	UpdateDeviceHandler getUpdateDeviceHandler();
private:
	Beetle &beetle;

	ControllerClient &client;

	void addDeviceHelper(Device *device);
	void updateDeviceHelper(Device *device);
	void removeDeviceHelper(device_t d);
};

#endif /* INCLUDE_CONTROLLER_NETWORKREPORTER_H_ */