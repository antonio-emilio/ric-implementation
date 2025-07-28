#!/bin/bash

set -e  # Encerra em caso de erro

echo "========================= UPDATE & DEPENDÊNCIAS ========================="
apt-get update
apt-get install -y \
    git \
    build-essential \
    cmake \
    libfftw3-dev \
    libmbedtls-dev \
    libsctp-dev \
    libconfig++-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libzmq3-dev \
    python3 \
    python3-pip \
    docker.io \
    docker-compose \
    iproute2 \
    gcc-11 \
    g++-11 \
    sudo \
    net-tools \
    iputils-ping \
    pkg-config \
    libusb-1.0-0-dev \
    libuhd-dev \
    uhd-host

# Configurar Docker para ser usado sem sudo (opcional, exige logout/login)
if ! groups $SUDO_USER | grep -q '\bdocker\b'; then
    echo "Adicionando usuário $SUDO_USER ao grupo docker..."
    usermod -aG docker $SUDO_USER
fi

# Diretórios base
RIC_DIR=~/oran-sc-ric
PROJECT_DIR=~/srsRAN_Project
UE_DIR=~/srsRAN_4G

echo "========================= DEPLOY RIC ========================="
cd ~
git clone https://github.com/srsran/oran-sc-ric || echo "oran-sc-ric já clonado"
cd $RIC_DIR
docker compose up -d

echo "========================= DEPLOY CORE ========================"
cd ~
git clone https://github.com/srsran/srsRAN_Project.git || echo "srsRAN_Project já clonado"
cd $PROJECT_DIR/docker
docker compose up -d --build 5gc

echo "========================= BUILD & DEPLOY GNB ================="
cd $PROJECT_DIR
mkdir -p build && cd build
cmake ../ -DENABLE_EXPORT=ON -DENABLE_ZEROMQ=ON
make -j $(nproc)
cd apps/gnb
./gnb -c ~/oran-sc-ric/e2-agents/srsRAN/gnb_zmq.yaml

echo "========================= DEPLOY UE =========================="
ip netns add ue1 || echo "Namespace ue1 já existe"

cd ~
git clone https://github.com/srsran/srsRAN_4G || echo "srsRAN_4G já clonado"
cd $UE_DIR
mkdir -p build && cd build
CC=gcc-11 CXX=g++-11 cmake ..
make -j $(nproc)
make install

cd $UE_DIR/build/srsue/src
./srsue ~/oran-sc-ric/e2-agents/srsRAN/ue_zmq.conf
