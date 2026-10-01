# 🔵 AgroTech — Estação de Monitorização Agrícola Inteligente

<p align="center">
  <img src="https://img.shields.io/badge/Placa-ESP32-007ACC?style=for-the-badge&logo=espressif&logoColor=white" />
  <img src="https://img.shields.io/badge/Linguagem-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/Simulador-Wokwi-1877F2?style=for-the-badge&logo=wokwi&logoColor=white" />
  <img src="https://img.shields.io/badge/Status-Ativo-0088CC?style=for-the-badge" />
</p>

---

## 🔹 Visão Geral do Sistema

A **AgroTech** é uma solução de IoT concebida para **estufas inteligentes**, permitindo o acompanhamento em tempo real de parâmetros ambientais críticos para maximizar o rendimento das culturas e mitigar riscos agrícolas.

```mermaid
graph TD
    A[🔹 Sensores: DHT22 & LDR] -->|Leitura contínua| B(⚡ Microcontrolador ESP32)
    B -->|Avalia Limites| C{Anomalia Detetada?}
    C -->|Não| D[🟢 LED Verde: Estado Normal]
    C -->|Sim| E[🔴 LED Vermelho + 🔊 Buzzer: Alerta]
    B -->|Sempre Ativo| F[🔵 LED Azul: Alimentação do Sistema]
    B -->|Telemetria| G[🌐 Monitor Serial / Nuvem]
