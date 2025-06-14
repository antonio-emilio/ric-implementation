#!/bin/bash

set -e

# 1. Atualização e instalação de dependências
echo "[1/4] Instalando dependências..."
sudo apt-get update
sudo apt-get install -y git docker.io docker-compose
sudo apt-get install -y libmbedtls-dev
echo "[+] Instalando dependências para srsRAN e srsUE..."
sudo apt-get install -y \
  cmake \
  build-essential \
  git \
  libboost-all-dev \
  libmbedtls-dev \
  libsctp-dev \
  libfftw3-dev \
  libzmq3-dev \
  libczmq-dev \
  libconfig++-dev \
  doxygen \
  pkg-config \
  python3 \
  python3-pip


# Adiciona usuário atual ao grupo docker (se necessário)
if ! groups $USER | grep -q '\bdocker\b'; then
  echo "[1.1] Adicionando $USER ao grupo docker..."
  sudo usermod -aG docker $USER
  echo "Você precisa reiniciar sua sessão para que a alteração no grupo tenha efeito."
fi

# 2. Clonar o repositório ORAN SC RIC
echo "[2/4] Clonando o repositório ORAN SC RIC..."
if [ ! -d "oran-sc-ric" ]; then
  git clone https://github.com/srsran/oran-sc-ric.git
else
  echo "Repositório já existe. Pulando clonagem."
fi

cd oran-sc-ric

# 3. Construção e execução do ambiente RIC
echo "[3/4] Construindo e executando containers..."
docker compose up --build -d

# 4. Verificação dos containers
echo "[4/4] Verificando status dos serviços..."
docker compose ps

echo "✅ O SC RIC foi iniciado. Use 'docker compose logs -f' para acompanhar os logs."

echo ""
echo "[+] Começando a preparação do ambiente RAN (srsRAN + Open5GS)..."

# Clonar srsRAN_Project (com suporte a ZMQ)
if [ ! -d "srsRAN_Project" ]; then
  git clone https://github.com/srsran/srsRAN_Project.git
  cd srsRAN_Project
  mkdir build
  cd build
  cmake ../ -DENABLE_EXPORT=ON -DENABLE_ZEROMQ=ON
  make -j $(nproc)
  cd ../
else
  echo "[!] srsRAN_Project já clonado."
fi

cd  ./srsRAN_Project/docker/
docker compose up --build 5gc

cd  ./srsRAN_Project/build/apps/gnb/
sudo ./gnb -c ~/oran-sc-ric/e2-agents/srsRAN/gnb_zmq.yaml

sudo apt-get install -y g++-11 gcc-11
# Clonar srsRAN_4G (para srsUE com suporte a ZMQ)
if [ ! -d "srsRAN_4G" ]; then
  git clone https://github.com/srsran/srsRAN_4G.git
  cd srsRAN_4G
  mkdir build && cd build
  CC=gcc-11 CXX=g++-11 cmake ..
  make
  cd ../
else
  echo "[!] srsRAN_4G já clonado."
fi

# Subir Core (Open5GS via docker-compose)
echo "[+] Subindo core (Open5GS)..."
cd srsRAN_Project/docker/
docker compose up --build -d 5gc
cd ../../

# Executar gNB (com E2 Agent conectado ao RIC)
echo "[+] Iniciando gNB..."
cd srsRAN_Project/build/apps/gnb/
sudo ./gnb -c ~/cerise/oran-sc-ric/e2-agents/srsRAN/gnb_zmq.yaml &
cd -

# Esperar alguns segundos para o gNB se conectar
sleep 10

# Executar srsUE
echo "[+] Iniciando srsUE com namespace 'ue1'..."
sudo ip netns add ue1 || echo "Namespace já existe"
cd srsRAN_4G/build/srsue/src/
sudo ./srsue ~/cerise/oran-sc-ric/e2-agents/srsRAN/ue_zmq.conf &
cd -

# Aguarde para o UE se registrar
sleep 5

# Teste de ping
echo "[+] Testando conectividade com a rede 5G core..."
sudo ip netns exec ue1 ping -i 0.1 -c 10 10.45.1.1

echo "[✓] Ambiente RAN completo iniciado."

