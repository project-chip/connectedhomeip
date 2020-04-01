/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_CORE_RESOURCE_DIRECTORY
#define ZCL_CORE_RESOURCE_DIRECTORY

extern EmZclUriPath emZclRdUriPaths[];

/** @brief Register device clusters with the target Resource Directory.
 *
 * This function will enumerate zcl endpoints and their clusters
 * constructing CoRE Link formatted resources, and
 * send those resources to the specified Resource Directory.
 *
 * @param resourceDirectoryIp The IP address of the resource directory.
 * @param resourceDirectoryPort The port used by the resource directory.
 */
void emberAfPluginResourceDirectoryClientRegister(EmberIpv6Address *resourceDirectoryIp, uint16_t resourceDirectoryPort);

/** @brief Check if device has already registered with an RD server
 *
 * This function iterates through all the RD servers (typically one or zero) we
 * have communicated with and checks the state of the registration. If any are
 * in a successful registration then this function returns TRUE.
 *
 */
bool emberAfPluginResourceDirectoryHaveRegistered();

#endif // ZCL_CORE_RESOURCE_DIRECTORY
