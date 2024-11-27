SCRIPTPATH="$( cd "$(dirname "${BASH_SOURCE}")" >/dev/null 2>&1 ; pwd -P )"
echo ${SCRIPTPATH}
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/packages/hwloc/2.4.0-sever-gcc/lib/pkgconfig
source ${SCRIPTPATH}/sourceme-common.sh

