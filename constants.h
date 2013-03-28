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
/// Path to config.d for board mappings
#define SSU_BOARD_MAPPING_CONFIGURATION_DIR "/usr/share/ssu/board-mappings.d"
/// The SSU protocol version used by the ssu client libraries
#define SSU_PROTOCOL_VERSION "1"
/// Maximum recursion level for resolving variables
#define SSU_MAX_RECURSION 1024
/// Path to zypper repo configuration
#define ZYPP_REPO_PATH "/etc/zypp/repos.d"
#endif
