
# FlexRIC + OAI CN5G Integration

Este repositório contém três scripts para configurar, iniciar e integrar o [FlexRIC](https://gitlab.eurecom.fr/mosaic5g/flexric) com o [OpenAirInterface 5G Core (OAI CN5G)](https://gitlab.eurecom.fr/oai/openairinterface5g).

## 📜 Ordem de Execução dos Scripts

1. [`setup_core.sh`](./setup_core.sh)
2. [`setup_flexric.sh`](./setup_flexric.sh)
3. [`integrar_oai_flexric.sh`](./integrar_oai_flexric.sh)

---

## 📦 1. `setup_core.sh`

Este script prepara o ambiente para o Core 5G da OAI.

### Funções:
- Instala Docker e dependências.
- Baixa os arquivos de configuração do Core.
- Executa os containers com `docker compose`.

> Após a execução, reinicie sua sessão para que o grupo `docker` funcione corretamente.

---

## 🛠️ 2. `setup_flexric.sh`

Este script compila e instala o FlexRIC com os service models.

### Funções:
- Instala compiladores e dependências (GCC 13, CMake etc.).
- Clona o repositório FlexRIC.
- Compila os binários.
- Instala os artefatos necessários em `/usr/local/lib/flexric/`.
- Executa testes unitários para validação.
- Inicia o `nearRT-RIC`, o agente emulado e os xApps de monitoramento.

---

## 🔗 3. `integrar_oai_flexric.sh`

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

## 📌 Requisitos

- Ubuntu 22.04
- Acesso `sudo`
- Conexão à internet
- Reinício da sessão após `setup_core.sh` (grupo docker)

---

## ✅ Verificação

- Para verificar o funcionamento, use `docker ps`, `htop` e logs de terminal.
- Certifique-se de que o diretório `/usr/local/lib/flexric/` está presente.
