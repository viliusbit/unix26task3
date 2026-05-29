#!/bin/bash
if ! command -v zstd &> /dev/null; then
    sudo apt update && sudo apt install -y zstd
fi
sudo apt install -y build-essential
curl -fsSL https://ollama.com/install.sh | sh
ollama serve > /dev/null 2>&1 &
sleep 5
ollama pull tinyllama
