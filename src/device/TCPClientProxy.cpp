/*
 * TCPClientProxy.cpp
 *
 *  Created on: Apr 10, 2016
 *      Author: james
 */

#include "../../include/device/TCPClientProxy.h"

#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <netinet/in.h>
#include <map>

#include "../../include/Beetle.h"
#include "../../include/Device.h"
#include "../../include/hat/SingleAllocator.h"

TCPClientProxy::TCPClientProxy(Beetle &beetle, int sockfd, std::string clientGateway_,
		struct sockaddr_in clientGatewaySockAddr_, device_t localProxyFor_)
: TCPConnection(beetle, sockfd, "", clientGatewaySockAddr_) {
	/*
	 * Make sure the device exists locally.
	 */
	boost::shared_lock<boost::shared_mutex> devicesLk(beetle.devicesMutex);
	if (beetle.devices.find(localProxyFor_) == beetle.devices.end()) {
		throw new DeviceException("no device for " + std::to_string(localProxyFor_));
	}

	name = "Proxy for " +  std::to_string(localProxyFor_) + " to " + clientGateway_;
	type = TCP_CLIENT_PROXY;
	clientGateway = clientGateway_;

	/*
	 * TODO: Not happy about this. Base class makes a hat...
	 */
	delete hat;
	hat = new SingleAllocator(localProxyFor_);

	localProxyFor = localProxyFor_;
}

TCPClientProxy::~TCPClientProxy() {
	// TODO Auto-generated destructor stub
}

device_t TCPClientProxy::getLocalDeviceId() {
	return localProxyFor;
};

std::string TCPClientProxy::getClientGateway() {
	return clientGateway;
};
