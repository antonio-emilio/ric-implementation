#!/bin/bash

set -e

echo "🔧 Instalando pré-requisitos básicos..."
sudo apt update
sudo apt install -y git net-tools putty ca-certificates curl unzip

echo "🐳 Instalando Docker..."
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "${UBUNTU_CODENAME:-$VERSION_CODENAME}") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

echo "👥 Adicionando usuário ao grupo docker (requer logout/login para efeito)..."
sudo usermod -aG docker "$USER"

echo "📦 Baixando arquivos de configuração do OAI CN5G..."
wget -O ~/oai-cn5g.zip "https://gitlab.eurecom.fr/oai/openairinterface5g/-/archive/develop/openairinterface5g-develop.zip?path=doc/tutorial_resources/oai-cn5g"
unzip -q ~/oai-cn5g.zip
mv openairinterface5g-develop-doc-tutorial_resources-oai-cn5g/doc/tutorial_resources/oai-cn5g ~/oai-cn5g
rm -rf openairinterface5g-develop-doc-tutorial_resources-oai-cn5g ~/oai-cn5g.zip

echo "🐳 Baixando imagens docker do Core 5G..."
cd ~/oai-cn5g
docker compose pull

echo "🚀 Iniciando o OAI CN5G com docker compose..."
docker compose up -d

echo ""
echo "✅ Core 5G da OAI iniciado com sucesso!"
echo "👉 Para verificar os containers: docker ps"
echo "👉 Para ver os logs: docker logs -f <nome_do_container>"
echo "👉 Para parar: cd ~/oai-cn5g && docker compose down"
echo ""
echo "⚠️ Saia e entre novamente na sessão do terminal para que o grupo docker tenha efeito sem sudo."

# https://gitlab.eurecom.fr/oai/openairinterface5g/-/blob/develop/doc/NR_SA_Tutorial_OAI_CN5G.md?plain=1
