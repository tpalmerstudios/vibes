/**
 * @file network.h
 * @brief Socket utilities for the Bourne daemon.
 */

#ifndef DEERBOURNE_BOURNE_NETWORK_H
#define DEERBOURNE_BOURNE_NETWORK_H

#include <stdbool.h>
#include <stddef.h>

#include "bourne/config.h"

/**
 * @brief Create a listening socket for the provided configuration.
 *
 * @param config Socket configuration parameters.
 * @return File descriptor for the listening socket; -1 on failure.
 */
int bourne_network_listen(const BourneServerConfig *config);

/**
 * @brief Accept a pending client connection.
 *
 * Emits verbose logging when a connection attempt fails.
 *
 * @param listener_fd Active listening socket.
 * @return Client file descriptor or -1 on failure.
 */
int bourne_network_accept(int listener_fd);

/**
 * @brief Close a Bourne network file descriptor.
 *
 * @param fd File descriptor to close; ignored when negative.
 */
void bourne_network_close(int fd);

#endif // DEERBOURNE_BOURNE_NETWORK_H
