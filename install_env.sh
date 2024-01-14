#!/bin/bash

# Aktualizacja listy pakietów
sudo apt-get update

# Instalacja narzędzi deweloperskich
sudo apt-get install -y build-essential

# Instalacja Clang i narzędzi związanych z Clang
sudo apt-get install -y clang clang-tidy clang-tools

echo "Instalacja zakończona pomyślnie"