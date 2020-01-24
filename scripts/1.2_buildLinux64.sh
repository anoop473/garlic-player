#!/bin/bash

set -e
SCRIPTDIR=$(dirname "$0")
source $SCRIPTDIR/envLinux64.sh

#check if called from jenkins
if [ -z "$BUILD_NUMBER" ]; then
	source ../$SCRIPTDIR/writeVersionFromGithub.sh
else
	source $SCRIPTDIR/writeVersionFromGithub.sh
fi

echo 
echo ========== prepare build 
echo 
$QT_PATH_RUNTIME/bin/qmake $GARLIC_DIR/src/complete.pro "CONFIG+=$CONFIG_DEBUG_RELEASE"

echo 
echo ========== build 
echo 
make -j $DEV_JOBS --silent

# check if binary was created 
if [ ! -f bin/garlic-player ]; then
	echo Binary was not created!
    exit 1;
fi

exit 0;
