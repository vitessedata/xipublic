
set -e 
set -x


# /bin/sh was symlinked to /bin/dash in ubuntu, not good.
sudo rm /bin/sh
sudo ln -s /bin/bash /bin/sh

# setup machine
sudo apt install -y software-properties-common

sudo apt-get install -y build-essential
sudo apt-get install -y git vim tmux openssh-client openssh-server sudo
sudo apt-get install -y flex bison autoconf autopoint libtool bzip2 unzip
sudo apt-get install -y libyaml-dev libreadline-dev zlib1g-dev libssl-dev
sudo apt-get install -y python-dev libpython-dev
sudo apt-get install -y libcurl4-openssl-dev
sudo apt-get install -y libevent-dev libxml2 libxml2-dev libbz2-dev
sudo apt-get install -y libapr1-dev 
sudo apt-get install -y libkrb5-dev 
sudo apt-get install -y libsnmp-base libsnmp-dev
sudo apt-get install -y libpam-dev
sudo apt-get install -y libldap2-dev libperl-dev
sudo apt-get install -y cmake3 || sudo apt-get install -y cmake
sudo apt-get install -y pkg-config
sudo apt install -y iputils-ping
sudo apt install -y libre2-dev
