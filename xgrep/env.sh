TOP_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export TOP_DIR
export XGREP_DIR=$HOME/data/x/release_vit

rm -f ./regex_aws_hw.xclbin
ln -s $XGREP_DIR/regex_aws_hw.xclbin .
