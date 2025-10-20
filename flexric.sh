#!/usr/bin/env bash
set -euo pipefail

# =========================
# FlexRIC minimal installer
# Tested on Ubuntu (apt)
# =========================

# --- Configs (altere se quiser) ---
FLEXRIC_DIR="${FLEXRIC_DIR:-$HOME/flexric}"
BUILD_TYPE="${BUILD_TYPE:-Release}"   # Debug|Release
FLEXRIC_REPO="${FLEXRIC_REPO:-https://gitlab.eurecom.fr/mosaic5g/flexric}"
RUN_MINI_TESTBED="${RUN_MINI_TESTBED:-false}"  # true para subir nearRT-RIC + emulador E2
E2AP_PORT="${E2AP_PORT:-36421}"       # Porta usada pelo E2AP nesta implementação
# -----------------------------------

need_cmd() { command -v "$1" &>/dev/null; }
die() { echo "ERROR: $*" >&2; exit 1; }
log() { echo -e "\033[1;32m$*\033[0m"; }

if ! need_cmd sudo; then die "Este script requer sudo na máquina."; fi

if ! need_cmd apt; then
  die "Distribuição sem apt detectado. Adapte os comandos de pacote (apt) para sua distro."
fi

# Detectar IP local para nearRT-RIC (primeira interface não-loopback)
detect_local_ip() {
  # tenta hostname -I, pega primeiro IP IPv4
  local ip
  ip=$(hostname -I 2>/dev/null | awk '{for(i=1;i<=NF;i++) if ($i ~ /^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$/) {print $i; exit}}' || true)
  if [[ -z "${ip:-}" ]]; then
    # fallback via ip route
    ip=$(ip route get 1.1.1.1 2>/dev/null | awk '/src/ {for(i=1;i<=NF;i++) if ($i=="src") {print $(i+1); exit}}' || true)
  fi
  echo "${ip:-127.0.0.1}"
}

LOCAL_IP=$(detect_local_ip)
log "[1/6] Atualizando pacotes e instalando dependências mínimas…"
sudo apt update -y
sudo apt upgrade -y
sudo apt install -y build-essential
# GCC-13 e toolchain (gcc-11 não suportado)
sudo apt install -y gcc-13 g++-13 cpp-13
# libs obrigatórias
sudo apt install -y libsctp-dev cmake-curses-gui libpcre2-dev

# Ajustar alternatives para usar gcc-13 como padrão
log "[2/6] Configurando gcc-13 como padrão…"
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-13 \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-13
# força seleção sem prompt
sudo update-alternatives --set gcc /usr/bin/gcc-13 || true

# Clonar repositório
log "[3/6] Clonando FlexRIC em ${FLEXRIC_DIR}…"
if [[ -d "$FLEXRIC_DIR/.git" ]]; then
  log "Repositório já existe. Atualizando…"
  git -C "$FLEXRIC_DIR" fetch --all --prune
  git -C "$FLEXRIC_DIR" pull --ff-only
else
  git clone "$FLEXRIC_REPO" "$FLEXRIC_DIR"
fi

# Build (Release por padrão) - mais rápido e adequado para perfil/uso real
log "[4/6] Compilando (CMAKE_BUILD_TYPE=${BUILD_TYPE})…"
mkdir -p "$FLEXRIC_DIR/build"
cd "$FLEXRIC_DIR/build"
cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..
make -j"$(nproc)"

# Instalar Service Models globalmente (xApps e RAN compartilham SMs)
log "[5/6] Instalando Service Models (sudo make install)…"
sudo make install

# Ajustar configuração do nearRT-RIC para usar o IP local
# Local: /usr/local/etc/flexric/flexric.conf (padrão de instalação)
CONF="/usr/local/etc/flexric/flexric.conf"
if [[ -f "$CONF" ]]; then
  log "[6/6] Ajustando NEAR_RIC_IP=${LOCAL_IP} e porta E2AP=${E2AP_PORT} em ${CONF}…"
  sudo sed -i "s/^\(\s*near_ric_ip\s*=\s*\).*/\1\"${LOCAL_IP}\";/g" "$CONF" || true
  sudo sed -i "s/^\(\s*e2ap_server_port\s*=\s*\).*/\1${E2AP_PORT};/g" "$CONF" || true
else
  log "Arquivo ${CONF} não encontrado. O binário aceitará -c/-p caso use prefixo customizado."
fi

# Funções para rodar mini testbed local (opcional)
start_near_rt_ric() {
  log "Iniciando nearRT-RIC em background…"
  # Usa conf e libs no caminho padrão de instalação
  nohup "$FLEXRIC_DIR/build/examples/ric/nearRT-RIC" \
    -c /usr/local/etc/flexric/flexric.conf \
    -p /usr/local/lib/flexric/ \
    >/tmp/flexric_near_rt_ric.log 2>&1 &
  echo $! > /tmp/flexric_near_rt_ric.pid
  log "nearRT-RIC PID $(cat /tmp/flexric_near_rt_ric.pid). Logs: /tmp/flexric_near_rt_ric.log"
}

start_gnb_emulator() {
  log "Iniciando E2 agent emulator (gNB mono) em background…"
  nohup "$FLEXRIC_DIR/build/examples/emulator/agent/emu_agent_gnb" \
    >/tmp/flexric_emu_gnb.log 2>&1 &
  echo $! > /tmp/flexric_emu_gnb.pid
  log "emu_agent_gnb PID $(cat /tmp/flexric_emu_gnb.pid). Logs: /tmp/flexric_emu_gnb.log"
}

stop_minitestbed() {
  for f in /tmp/flexric_near_rt_ric.pid /tmp/flexric_emu_gnb.pid; do
    [[ -f "$f" ]] && { kill "$(cat "$f")" 2>/dev/null || true; rm -f "$f"; }
  done
  log "Mini testbed parado."
}

if [[ "${RUN_MINI_TESTBED}" == "true" ]]; then
  start_near_rt_ric
  # esperar RIC subir alguns segundos antes do agente
  sleep 2
  start_gnb_emulator
  log "Mini testbed ativo. Você pode inspecionar mensagens E2 (porta ${E2AP_PORT})."
  log "Para parar, rode: kill \$(cat /tmp/flexric_near_rt_ric.pid /tmp/flexric_emu_gnb.pid)"
fi
