/**
 * @file constants.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

/// The group ID SSU expects to run as. This is usually the GID of the main phone user
#define SSU_GROUP_ID 1000
/// Path to the main SSU configuration file
#define SSU_CONFIGURATION "/etc/ssu/ssu.ini"
/// Path to the main SSU configuration file
#define SSU_REPO_CONFIGURATION "/usr/share/ssu/repos.ini"
/// Path to the main SSU configuration file
#define SSU_DEFAULT_CONFIGURATION "/usr/share/ssu/ssu-defaults.ini"
/// Path to board / device family mappings file
#define SSU_BOARD_MAPPING_CONFIGURATION "/usr/share/ssu/board-mappings.ini"
/// The SSU protocol version used by the ssu client libraries
#define SSU_PROTOCOL_VERSION "1"
#endif
