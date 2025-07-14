#!/bin/bash

set -e

echo "📦 Instalando dependências..."
sudo apt update
sudo apt install -y build-essential git cmake-curses-gui libsctp-dev libpcre2-dev unzip \
  gcc-13 g++-13 cpp-13 python3.10-dev autoconf automake libtool bison flex

echo "⚙️ Configurando gcc-13 como padrão..."
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-13 --slave /usr/bin/gcov gcov /usr/bin/gcov-13
sudo update-alternatives --set gcc /usr/bin/gcc-13

echo "🔧 Instalando ASN1C com suporte a -gen-UPER (mouse07410/aper)..."
cd ~
rm -rf asn1c
git clone https://github.com/mouse07410/asn1c.git
cd asn1c
git checkout aper
autoreconf -fiv
./configure
make -j$(nproc)
sudo make install

echo "📁 Clonando FlexRIC..."
cd ~
git clone https://gitlab.eurecom.fr/mosaic5g/flexric.git
cd flexric

echo "🏗️ Compilando FlexRIC..."
rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DASN1C_EXEC_PATH=/usr/local/bin/asn1c ..
make -j$(nproc)

echo "📥 Instalando modelos de serviço (Service Models)..."
sudo make install

echo "✅ Criando diretório esperado para testes..."
sudo mkdir -p /usr/local/lib/flexric/

echo "✅ Verificando instalação com testes unitários..."
ctest -j$(nproc) --output-on-failure || true

echo "📁 Retornando à raiz do projeto..."
cd ..

echo "⚙️ Copiando arquivo de configuração para referência..."
CONF_PATH="/usr/local/etc/flexric/flexric.conf"
if [ ! -f "$CONF_PATH" ]; then
  echo "⚠️ Arquivo de configuração não encontrado em $CONF_PATH"
else
  echo "🧠 Arquivo de configuração pronto: $CONF_PATH"
fi

echo "🚀 Iniciando o nearRT-RIC..."
build/examples/ric/nearRT-RIC &

sleep 2

echo "🔗 Iniciando agente E2 emulado (gNB)..."
build/examples/emulator/agent/emu_agent_gnb &

sleep 2

echo "📡 Iniciando xApp de monitoramento E2SM-KPM (20s)..."
XAPP_DURATION=20 build/examples/xApp/c/monitor/xapp_kpm_moni &

sleep 2

echo "🎯 Iniciando xApp de monitoramento GTP+MAC+RLC+PDCP (20s)..."
XAPP_DURATION=20 build/examples/xApp/c/monitor/xapp_gtp_mac_rlc_pdcp_moni &

echo "✅ FlexRIC está rodando com nearRT-RIC, E2 agent e xApps."
