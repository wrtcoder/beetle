/*
 * ConnParams.h
 *
 *  Created on: Apr 10, 2016
 *      Author: James Hong
 */

#ifndef INCLUDE_TCP_CONNPARAMS_H_
#define INCLUDE_TCP_CONNPARAMS_H_

#include <map>
#include <string>
#include <openssl/ossl_typ.h>

/*
 * Indicate that the client is a gateway. Value is the gateway's name.
 */
const std::string TCP_PARAM_GATEWAY = "gateway";

/*
 * Indicate the device the gateway is connecting to. Value is the device id.
 */
const std::string TCP_PARAM_DEVICE = "device";

/*
 * Declare a client-> Value is the name.
 */
const std::string TCP_PARAM_CLIENT = "client";

/*
 * Indicate that the client is a server. Value is a (true/false).
 * Default is false. If true, then Beetle will try to discover handles.
 */
const std::string TCP_PARAM_SERVER = "server";

/*
 * Read paramsLen bytes of plaintext parameters from fd into params. Socket
 * should be nonblocking, if timeout seconds passes, then failure is returned.
 * Returns true on success.
 */
bool readParamsHelper(SSL *ssl, int fd, std::map<std::string, std::string> &params, double timeout = 10.0);

#endif /* INCLUDE_TCP_CONNPARAMS_H_ */
