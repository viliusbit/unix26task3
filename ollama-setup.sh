#!/bin/bash

curl -fsSL https://ollama.com/install.sh | sh
ollama serve > /dev/null 2>&1 &
sleep 5
ollama pull tinyllama
