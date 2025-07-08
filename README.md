
# RIC Implementation Repository

This repository contains scripts for deploying and integrating various RIC (Radio Interface Controller) implementations with existing CORE/gNB infrastructure.

## 🚀 Interactive Deployment Script (NEW)

**[`deploy_ric_interactive.py`](./deploy_ric_interactive.py)** - Interactive Python script that guides users through RIC deployment configuration and execution.

### Features:
- **Interactive Configuration**: Asks users for all necessary deployment parameters
- **Multiple RIC Support**: ORAN SC RIC, FlexRIC, OAI E2 Agent, and Custom implementations
- **Existing Infrastructure**: Connects to existing CORE/gNB or deploys new ones
- **Input Validation**: Validates IP addresses, ports, and directory paths
- **Configuration Management**: Saves and loads deployment configurations
- **Automated Script Generation**: Creates deployment scripts based on user input
- **Real-time Execution**: Executes deployment with live output

### Usage:
```bash
# Make script executable
chmod +x deploy_ric_interactive.py

# Show help
python3 deploy_ric_interactive.py --help

# Run interactive deployment
python3 deploy_ric_interactive.py
```

### Supported RIC Types:
1. **ORAN SC RIC** - Standard ORAN SC implementation with Docker support
2. **FlexRIC** - FlexRIC with OAI integration and custom E2AP/KMP versions
3. **OAI E2 Agent** - OAI with E2 Agent integration and RF simulator support
4. **Custom RIC** - Template for custom RIC implementations

---

## 📜 Legacy Scripts (Individual Components)

1. [`setup_core.sh`](./setup_core.sh)
2. [`setup_flexric.sh`](./setup_flexric.sh)
3. [`integrar_oai_flexric.sh`](./integrar_oai_flexric.sh)
4. [`deploy_flexric_oai_e2.sh`](./deploy_flexric_oai_e2.sh)
5. [`install_and_run_oran_sc_ric.sh`](./install_and_run_oran_sc_ric.sh)

---

## 📦 Legacy Script Details

### 1. `setup_core.sh`

Este script prepara o ambiente para o Core 5G da OAI.

### Funções:
- Instala Docker e dependências.
- Baixa os arquivos de configuração do Core.
- Executa os containers com `docker compose`.

> Após a execução, reinicie sua sessão para que o grupo `docker` funcione corretamente.

---

### 2. `setup_flexric.sh`

Este script compila e instala o FlexRIC com os service models.

### Funções:
- Instala compiladores e dependências (GCC 13, CMake etc.).
- Clona o repositório FlexRIC.
- Compila os binários.
- Instala os artefatos necessários em `/usr/local/lib/flexric/`.
- Executa testes unitários para validação.
- Inicia o `nearRT-RIC`, o agente emulado e os xApps de monitoramento.

---

### 3. `integrar_oai_flexric.sh`

Este script realiza a integração entre o gNB do OAI e o `nearRT-RIC`.

### Funções:
- Clona o repositório OAI RAN com suporte ao E2 Agent.
- Compila o gNB e o UE com o `--build-e2`.
- Modifica o arquivo de configuração do gNB para ativar o E2 Agent.
- Inicia:
  - `nearRT-RIC`
  - `gNB` com E2 Agent
  - `UE` simulado
  - xApps de monitoramento

---


### 4. `deploy_flexric_oai_e2.sh`

Este script executa todo o processo completo de forma automatizada:

### Funções:
- Instala todas as dependências para OAI RAN e FlexRIC
- Compila o `gNB` e o `nrUE` com suporte ao E2 Agent
- Compila o `FlexRIC` com os mesmos parâmetros de E2AP/KPM
- Inicia:
  - `nearRT-RIC`
  - `gNB` com `--rfsim`
  - `nrUE` simulado com `rfsim`
  - xApps de monitoramento (KPM, RC, MAC/RLC/PDCP/GTP)

### Comando para executar:
```bash
chmod +x deploy_flexric_oai_e2.sh
./deploy_flexric_oai_e2.sh
```

> Este script pressupõe que o Core 5G da OAI já está rodando conforme descrito no `setup_core.sh`.


Este script realiza a integração entre o gNB do OAI e o `nearRT-RIC`.

### Funções:
- Clona o repositório OAI RAN com suporte ao E2 Agent.
- Compila o gNB e o UE com o `--build-e2`.
- Modifica o arquivo de configuração do gNB para ativar o E2 Agent.
- Inicia:
  - `nearRT-RIC`
  - `gNB` com E2 Agent
  - `UE` simulado
  - xApps de monitoramento

---

### 5. `install_and_run_oran_sc_ric.sh`

This script installs and runs the ORAN SC RIC implementation with srsRAN integration.

### Functions:
- Installs dependencies for srsRAN and srsUE
- Clones and builds srsRAN Project and srsRAN 4G
- Sets up ORAN SC RIC with Docker
- Configures E2 Agent integration
- Starts gNB and UE with ZMQ support

---

## 📖 Documentation

- **[Configuration Examples](./CONFIG_EXAMPLES.md)** - Example configurations for different deployment scenarios
- **[Legacy Scripts Guide](./README.md#legacy-script-details)** - Details about individual component scripts

---

## 📌 Requirements

## 📌 Requirements

### For Interactive Deployment Script:
- **Ubuntu 20.04 or 22.04**
- **Python 3.8+**
- **sudo privileges**
- **Internet connection**

### For Legacy Scripts:
- Ubuntu 22.04
- sudo access
- Internet connection
- Session restart after `setup_core.sh` (docker group)

---

## ✅ Verification

- For interactive deployment: Follow the script prompts and check generated logs
- For legacy scripts: Use `docker ps`, `htop` and terminal logs
- Ensure `/usr/local/lib/flexric/` directory exists for FlexRIC deployments
