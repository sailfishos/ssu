/**
 * @file ssuconfperm.c
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 *
 * This is a dirty hack to make sure the main user can write to the
 * configuration file; this will be obsolete once we have some
 * security framework in place.
 */

#include <sys/stat.h>
#include <unistd.h>

#include "../constants.h"

int main(int argc, char **argv){
  struct stat sb;

  if (!stat(SSU_CONFIGURATION, &sb)){
    chown(SSU_CONFIGURATION, 0, SSU_GROUP_ID);
    chmod(SSU_CONFIGURATION, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  }
}
