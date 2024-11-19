#!/bin/bash

# 스크립트 실행 시 오류 발생 시 중지
set -e

echo "Paho C 및 C++ 라이브러리 제거를 시작합니다..."

# Paho C uninstall
if [ -f "paho.mqtt.c/build/install_manifest.txt" ]; then
    echo "Paho C 라이브러리 제거 중..."
    xargs -a paho.mqtt.c/build/install_manifest.txt sudo rm -v
    echo "Paho C 라이브러리가 제거되었습니다."
else
    echo "Paho C의 install_manifest.txt 파일을 찾을 수 없습니다. 수동으로 제거해야 할 수 있습니다."
fi

# Paho C++ uninstall
if [ -f "paho.mqtt.cpp/build/install_manifest.txt" ]; then
    echo "Paho C++ 라이브러리 제거 중..."
    xargs -a paho.mqtt.cpp/build/install_manifest.txt sudo rm -v
    echo "Paho C++ 라이브러리가 제거되었습니다."
else
    echo "Paho C++의 install_manifest.txt 파일을 찾을 수 없습니다. 수동으로 제거해야 할 수 있습니다."
fi

echo "Paho C 및 C++ 라이브러리 제거가 완료되었습니다!"