#!/bin/bash

set -e

echo "🚀 Iniciando integração entre OAI RAN e FlexRIC..."

echo "📁 Entrando no diretório do OAI..."
git clone https://gitlab.eurecom.fr/oai/openairinterface5g oai
cd ~/oai || { echo "❌ Diretório oai não encontrado!"; exit 1; }

echo "🛠️ Verificando submódulos (E2 Agent)..."
git submodule init
git submodule update

echo "📦 Compilando OAI com suporte ao E2 Agent..."
cd cmake_targets
./build_oai -I -w SIMU --gNB --nrUE --build-e2 --ninja

echo "⚙️ Configurando o E2 Agent no arquivo de configuração do gNB..."
CONF_PATH="targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106PRB.usrpb210.conf"
E2_AGENT_BLOCK=$(cat <<EOF
e2_agent = {
  near_ric_ip_addr = "127.0.0.1";
  sm_dir = "/usr/local/lib/flexric/"
}
EOF
)

if grep -q "e2_agent" "$CONF_PATH"; then
  echo "🔁 Bloco e2_agent já está presente no arquivo de configuração."
else
  echo "➕ Adicionando bloco e2_agent ao final do arquivo..."
  echo -e "\n$E2_AGENT_BLOCK" | sudo tee -a "$CONF_PATH" > /dev/null
fi

echo "🛰️ Iniciando o nearRT-RIC..."
cd ~/flexric || { echo "❌ Diretório flexric não encontrado!"; exit 1; }
build/examples/ric/nearRT-RIC &

sleep 3

echo "📡 Iniciando o agente gNB (E2 Node)..."
cd ~/oai/cmake_targets/ran_build/build || { echo "❌ gNB não encontrado!"; exit 1; }
sudo ./nr-softmodem -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106PRB.usrpb210.conf --rfsim -E &

sleep 3

echo "📶 Iniciando o UE..."
sudo ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000 --rfsim --uicc0.imsi 001010000000001 --rfsimulator.serveraddr 127.0.0.1 &

sleep 3

echo "📊 Iniciando xApps de monitoramento..."
cd ~/flexric

build/examples/xApp/c/monitor/xapp_kpm_moni &
build/examples/xApp/c/monitor/xapp_rc_moni &
build/examples/xApp/c/monitor/xapp_gtp_mac_rlc_pdcp_moni &

echo "✅ Integração completa entre OAI e FlexRIC iniciada com sucesso!"
