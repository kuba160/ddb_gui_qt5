#!/bin/sh

temp_conf=`mktemp -d`

echo "new config dir: $temp_conf"
XDG_RUNTIME_DIR=$temp_conf XDG_CONFIG_HOME=$temp_conf deadbeef --gui q_test
echo rm -r "$temp_conf"
