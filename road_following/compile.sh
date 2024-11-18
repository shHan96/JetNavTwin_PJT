#!/bin/bash

TORCH_PATH=/usr/local/lib/python3.6/dist-packages/torch

echo "Checking torch installation..."
python3 -c "import torch; print('Torch version:', torch.__version__)"

echo "Compiling..."
g++ -std=c++14 main.cpp \
    -I$TORCH_PATH/include \
    -I$TORCH_PATH/include/torch/csrc/api/include \
    -I/usr/include/opencv4 \
    $TORCH_PATH/lib/libtorch.so \
    $TORCH_PATH/lib/libc10.so \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_videoio \
    -Wl,-rpath,$TORCH_PATH/lib \
    -O3 \
    -o road_following

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
else
    echo "Compilation failed!"
fi