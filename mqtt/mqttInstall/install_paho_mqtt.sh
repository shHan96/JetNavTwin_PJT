#!/bin/bash

# 스크립트 실행 시 오류 발생 시 중지
set -e

echo "Paho MQTT 설치를 시작합니다."

# 1. 종속성 설치
echo "필요한 종속성을 설치 중입니다..."
sudo apt-get update
sudo apt-get install -y build-essential gcc make cmake libssl-dev
sudo apt-get install -y doxygen graphviz  # 문서화 도구 (선택 사항)

# 2. Paho C Library 설치
echo "Paho C 라이브러리를 설치 중입니다..."
if [ -d "paho.mqtt.c" ]; then
	echo "기존 paho.mqtt.c 디렉토리를 삭제합니다..."
	rm -rf paho.mqtt.c
fi
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.13
cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON
sudo env "PATH=$PATH" cmake --build build/ --target install
cd ..

# 3. Paho C++ Library 설치
echo "Paho C++ 라이브러리를 설치 중입니다..."
if [ -d "paho.mqtt.cpp" ]; then
	echo "기존 paho.mqtt.cpp 디렉토리를 삭제합니다..."
	rm -rf paho.mqtt.cpp
fi
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
git checkout v1.4.0
git submodule init
git submodule update
cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_EXAMPLES=ON
sudo env "PATH=$PATH" cmake --build build/ --target install
cd ..

echo "Paho MQTT 설치가 완료되었습니다!"
