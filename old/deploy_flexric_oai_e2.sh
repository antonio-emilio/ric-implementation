#!/bin/bash

set -e

echo "🔧 Instalando dependências base..."
sudo apt update && sudo apt install -y \
  git build-essential cmake-curses-gui libsctp-dev libpcre2-dev \
  gcc-13 g++-13 cpp-13 unzip python3.10-dev net-tools ninja-build

echo "⚙️ Configurando gcc-13 como padrão..."
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-13 --slave /usr/bin/gcov gcov /usr/bin/gcov-13
sudo update-alternatives --set gcc /usr/bin/gcc-13

echo "📥 Clonando repositório OAI com E2 Agent..."
git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git
cd openairinterface5g

echo "📁 Inicializando submódulo FlexRIC embutido..."
cd openair2/E2AP/flexric
git submodule init && git submodule update
cd ../../../

echo "🛠️ Compilando OAI com suporte a gNB, UE e E2 Agent..."
cd cmake_targets
./build_oai -I
./build_oai --gNB --nrUE --build-e2 --cmake-opt -DE2AP_VERSION=E2AP_V2 --cmake-opt -DKPM_VERSION=KPM_V2_03 --ninja
cd ../

echo "🏗️ Compilando FlexRIC (submódulo) com mesmas versões do OAI..."
cd openair2/E2AP/flexric
mkdir -p build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DE2AP_VERSION=E2AP_V2 -DKPM_VERSION=KPM_V2_03 ..
ninja
sudo make install

echo "🚀 Iniciando nearRT-RIC..."
./examples/ric/nearRT-RIC &

sleep 2

echo "📡 Iniciando gNB com E2 Agent..."
cd ../../../build
sudo ./nr-softmodem -O ../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106PRB.usrpb210.conf --gNBs.[0].min_rxtxtime 6 --rfsim &

sleep 3

echo "📶 Iniciando UE simulado (nrUE)..."
sudo ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000 --rfsim --uicc0.imsi 001010000000001 --rfsimulator.serveraddr 127.0.0.1 &

sleep 5

echo "📈 Iniciando xApp KPM monitor..."
cd ../openair2/E2AP/flexric
XAPP_DURATION=30 ./build/examples/xApp/c/monitor/xapp_kpm_moni &

sleep 1

echo "📡 Iniciando xApp RC monitor..."
XAPP_DURATION=30 ./build/examples/xApp/c/monitor/xapp_rc_moni &

sleep 1

echo "📊 Iniciando xApp MAC+RLC+PDCP+GTP monitor..."
XAPP_DURATION=30 ./build/examples/xApp/c/monitor/xapp_gtp_mac_rlc_pdcp_moni &

echo "✅ Tudo rodando: FlexRIC + OAI gNB com E2 + UE + xApps"
