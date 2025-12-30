# Используем bullseye для лучшей совместимости со старыми дистрибутивами RPi и по компиляторам
# FROM debian:bullseye - будут проблемы с компиляцией. Компилятор ставит инструкции от armv7, а надо v6 
# Используем balenalib образ с поддержкой ARMv6
FROM --platform=linux/arm/v6 balenalib/raspberry-pi-debian:bullseye-build



RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app


# Настройки для ARMv6 (Pi Zero / Pi 1)
ENV CXX_FLAGS="-marm -march=armv6 -mfpu=vfp -mfloat-abi=hard"


CMD mkdir -p build && cd build && \
    cmake .. \
        -DWEBUI_BUILD_EXAMPLES=OFF \
        -DWEBUI_BUILD_STATIC=ON && \
    make
