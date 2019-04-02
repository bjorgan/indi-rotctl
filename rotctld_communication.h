/**
 * Copied and modified from hamlib.c/.h in Flyby, 2019-02-17.
 */

#ifndef ROTCTLD_COMMUNICATION_H_DEFINED
#define ROTCTLD_COMMUNICATION_H_DEFINED

#include <cstdlib>

/**
 * Connect to rotctld.
 *
 * \param host Rotctld host
 * \param port Rotctld port
 * \return Socket to rotctld. Returns -1 on failure
 */
int rotctld_connect(const char *rotctld_host, const char *rotctld_port);

/**
 * Read available data from socket until newline or end of string.
 *
 * \param sockd Socket to rotctld
 * \param message Returned message
 * \param bufsize Size of message array
 */
int rotctld_sock_readline(int sockd, char *message, size_t bufsize);

/**
 * Set new position to rotctld.
 *
 * \param socket Socket to rotctld
 * \param azimuth Azimuth
 * \param elevation Elevation
 */
void rotctld_set_position(int socket, double azimuth, double elevation);

/**
 * Get current position from rotctld.
 *
 * \param socket Socket to rotctld
 * \param azimuth Returned azimuth
 * \param elevation Returned elevation
 */
void rotctld_get_position(int socket, float *azimuth, float *elevation);

/**
 * Stop rotor.
 *
 * \param socket Socket to rotctld
 */
void rotctld_stop(int socket);

#endif
