FROM ubuntu:20.04

# Устанавливаем переменные окружения
ENV TZ=Europe/Moscow
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y software-properties-common \
    && add-apt-repository -y ppa:deadsnakes/ppa \
    && apt-get update && apt-get install -y \
    cmake \
    g++ \
    libboost-system-dev \
    libboost-filesystem-dev \
    libpq-dev \
    libpqxx-dev \
    nlohmann-json3-dev

WORKDIR /app

COPY . .

# Собираем проект
RUN rm -rf build && mkdir build && cd build && cmake .. && make

