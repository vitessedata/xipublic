TOP_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export TOP_DIR
export XGREP_DIR=$HOME/xilinx/xre 
export XXXX=$HOME/p/xilinx/xxx

rm -f ./xre_hw.xclbin
ln -s $XGREP_DIR/xre_hw.xclbin .

. /opt/xilinx/xrt/setup.sh
. ~/xilinx/Vivado/2019.1/settings64.sh

export LD_LIBRARY_PATH=$HOME/xilinx/x/installed/lib:$LD_LIBRARY_PATH

export PATH=${XILINX_VIVADO}/tps/lnx64/gcc-6.2.0/bin:$PATH
