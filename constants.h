/**
 * @file constants.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

/// The group ID ssu expects to run as. This is usually the GID of the main phone user
#define SSU_GROUP_ID 1000
/// Path to the main ssu configuration file
#define SSU_REPO_CONFIGURATION "/usr/share/ssu/repos.ini"
/// Path to board / device family mappings file
#define SSU_BOARD_MAPPING_CONFIGURATION "/var/cache/ssu/board-mappings.ini"
/// Path to config.d for board mappings
#define SSU_BOARD_MAPPING_CONFIGURATION_DIR "/usr/share/ssu/board-mappings.d"
/// Directory where all the non-user modifiable data sits
#define SSU_DATA_DIR "/usr/share/ssu/"
/// The ssu protocol version used by the ssu client libraries
#define SSU_PROTOCOL_VERSION "1"
/// Maximum recursion level for resolving variables
#define SSU_MAX_RECURSION 1024
/// Path to zypper repo configuration
#define ZYPP_REPO_PATH "/etc/zypp/repos.d"

/// Path to the main ssu configuration file
#define SSU_FEATURE_CONFIGURATION "/var/cache/ssu/features.ini"

/// Path to the main ssu configuration file
#define SSU_FEATURE_CONFIGURATION_DIR "/usr/share/ssu/features.d"

#endif
