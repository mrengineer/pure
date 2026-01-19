#!/bin/bash

set -e


# Инициализация и обновление подмодулей git
git submodule update --init --recursive


# --- 1. СБОРКА ПОД UBUNTU (x86_64) ---
echo "Сборка под x86_64..."
sudo rm -rf build_x86
mkdir -p build_x86
cd build_x86
/usr/bin/cmake ..
make -j$(nproc)
cd ..



#Чтобы ваш компьютер мог выполнять инструкции внутри ARMv6 контейнера (запускать cmake, make и т.д.), выполните в терминале вашего ПК:
#sudo apt-get update
#sudo apt-get install -y qemu-user-static
# Активация эмулятора в Docker
#docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

#docker build -t pizero-stable-builder .
sudo rm -rf build/
docker run --rm -v $(pwd):/app pizero-stable-builder

echo "x86 бинарник: build_x86/pure"
echo "ARM бинарник: build_arm/pure"