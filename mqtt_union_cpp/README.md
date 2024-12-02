# Jetson Nano 실행
------

## Jetson Nano version

```
jetpack 4.3
python 3.6.9
torch 1.13.0
cuda 10.0
opencv 4.1.1
```
### MQTT Jetson Nano
```
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.13
cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON
sudo cmake --build build/ --target install
cd ..
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
git checkout 922e866f4ff4c02f8d488f1426f806cbed603e5d
cmake -Bbuild -H. -DPAHO_BUILD_EXAMPLES=ON
sudo cmake --build build/ --target install
```
./include/system_controller.h line 13 </br>
SystemController(const std::string& broker_address = "tcp://192.168.164.137:1883") </br>
IPADDRESS modify </br>
### CrossCompiler
```
sudo apt update
sudo apt install gcc-8-aarch64-linux-gnu g++-8-aarch64-linux-gnu
sudo apt install cmake
```


### Compile
```
mkdir build
mv ./best_steering_model_xy_test.pt ./build/best_steering_model_xy_test.pt
cd build
cmake ..
make
./road_following # RUN
```

### RUN
```
./exefile
# exefile과 best_steering_model_xy_test.pt같은 폴더 내에 있어야한다.
# torch와 cuda의 동적라이브러리의 위치가 맞아야한다.
```


