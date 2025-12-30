#!/bin/bash

#Чтобы ваш компьютер мог выполнять инструкции внутри ARMv6 контейнера (запускать cmake, make и т.д.), выполните в терминале вашего ПК:
#sudo apt-get update
#sudo apt-get install -y qemu-user-static
# Активация эмулятора в Docker
#docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

docker build -t pizero-stable-builder .
sudo rm -rf build/
docker run --rm -v $(pwd):/app pizero-stable-builder
