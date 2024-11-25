#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <cmath>
#include <deque>
#include <numeric>

class BatteryMonitor {
private:
    int file_i2c;
    const char *filename = "/dev/i2c-1";
    const int I2C_ADDR = 0x41;
    
    // INA219 Registers
    const uint8_t INA219_REG_CONFIG        = 0x00;
    const uint8_t INA219_REG_SHUNTVOLTAGE  = 0x01;
    const uint8_t INA219_REG_BUSVOLTAGE    = 0x02;
    const uint8_t INA219_REG_POWER         = 0x03;
    const uint8_t INA219_REG_CURRENT       = 0x04;
    const uint8_t INA219_REG_CALIBRATION   = 0x05;
    
    // Battery specifications
    const float BATTERY_MAX_VOLTAGE = 12.6f;
    const float BATTERY_MIN_VOLTAGE = 9.0f;
    
    // Moving average window sizes
    const size_t VOLTAGE_WINDOW_SIZE = 10;
    const size_t CURRENT_WINDOW_SIZE = 5;
    
    // Moving average buffers
    std::deque<float> voltageBuffer;
    std::deque<float> currentBuffer;
    
    // Last valid readings
    float lastValidVoltage = 0.0f;
    float lastValidCurrent = 0.0f;
    
    bool debugMode = true;

    void debugPrint(const char* message) {
        if (debugMode) {
            std::cerr << "Debug: " << message << " (errno: " << errno << " - " << strerror(errno) << ")" << std::endl;
        }
    }

    float getMovingAverage(std::deque<float>& buffer) {
        if (buffer.empty()) return 0.0f;
        return std::accumulate(buffer.begin(), buffer.end(), 0.0f) / buffer.size();
    }

    void updateMovingAverage(std::deque<float>& buffer, float newValue, size_t windowSize, float maxChange = 0.0f) {
        if (!buffer.empty() && maxChange > 0.0f) {
            float lastAvg = getMovingAverage(buffer);
            if (std::abs(newValue - lastAvg) > maxChange) {
                return; // 값이 너무 많이 변경되면 무시
            }
        }

        buffer.push_back(newValue);
        while (buffer.size() > windowSize) {
            buffer.pop_front();
        }
    }

    bool initI2C() {
        if ((file_i2c = open(filename, O_RDWR)) < 0) {
            debugPrint("Failed to open i2c bus");
            return false;
        }

        if (ioctl(file_i2c, I2C_SLAVE, I2C_ADDR) < 0) {
            debugPrint("Failed to acquire bus access/talk to slave");
            return false;
        }

        return configureINA219();
    }

    bool configureINA219() {
        // 16-bit ADC, 32 samples averaging, continuous mode
        uint16_t config = 0x399F;
        
        uint8_t buf[3];
        buf[0] = INA219_REG_CONFIG;
        buf[1] = (config >> 8) & 0xFF;
        buf[2] = config & 0xFF;
        
        if (write(file_i2c, buf, 3) != 3) {
            debugPrint("Error writing config register");
            return false;
        }
        
        // Calibration for 16V FSR
        uint16_t calibration = 0x1000;
        buf[0] = INA219_REG_CALIBRATION;
        buf[1] = (calibration >> 8) & 0xFF;
        buf[2] = calibration & 0xFF;
        
        if (write(file_i2c, buf, 3) != 3) {
            debugPrint("Error writing calibration register");
            return false;
        }
        
        usleep(1000);
        return true;
    }

    int16_t readRegister(uint8_t reg) {
        uint8_t buf[2] = {0};
        
        if (write(file_i2c, &reg, 1) != 1) {
            debugPrint("Error writing to register");
            return -1;
        }
        
        usleep(1000); // 안정화를 위한 대기 시간 증가
        
        if (read(file_i2c, buf, 2) != 2) {
            debugPrint("Error reading from register");
            return -1;
        }
        
        return (buf[0] << 8) | buf[1];
    }

    float calculateCapacity(float voltage) {
        if (voltage >= BATTERY_MAX_VOLTAGE) return 100.0f;
        if (voltage <= BATTERY_MIN_VOLTAGE) return 0.0f;
        
        float voltage_range = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;
        float current_voltage = voltage - BATTERY_MIN_VOLTAGE;
        float percentage = (current_voltage / voltage_range) * 100.0f;
        
        // 비선형성 보정
        percentage = (log10(percentage + 1) / log10(101)) * 100.0f;
        
        return std::min(std::max(percentage, 0.0f), 100.0f);
    }

public:
    struct BatteryStatus {
        float voltage;
        float current;
        float power;
        float capacity;
        float timeRemaining;
        bool isCharging;
        bool valid;
    };

    BatteryMonitor() {
        if (!initI2C()) {
            std::cerr << "Battery monitor initialization failed" << std::endl;
        }
    }

    ~BatteryMonitor() {
        if (file_i2c >= 0) {
            close(file_i2c);
        }
    }

    BatteryStatus getBatteryStatus() {
        BatteryStatus status = {0, 0, 0, 0, 0, false, false};
        const float MAX_VOLTAGE_CHANGE = 0.5f;  // 최대 허용 전압 변화량
        const float MAX_CURRENT_CHANGE = 100.0f; // 최대 허용 전류 변화량
        
        // 전압 읽기 및 필터링
        int16_t raw_voltage = readRegister(INA219_REG_BUSVOLTAGE);
        if (raw_voltage >= 0) {
            float voltage = (raw_voltage >> 3) * 0.004;
            if (voltage >= BATTERY_MIN_VOLTAGE && voltage <= BATTERY_MAX_VOLTAGE) {
                updateMovingAverage(voltageBuffer, voltage, VOLTAGE_WINDOW_SIZE, MAX_VOLTAGE_CHANGE);
                status.voltage = getMovingAverage(voltageBuffer);
                lastValidVoltage = status.voltage;
                status.valid = true;
            } else {
                status.voltage = lastValidVoltage;
            }
        }

        // 전류 읽기 및 필터링
        int16_t raw_current = readRegister(INA219_REG_CURRENT);
        if (raw_current >= 0) {
            float current = raw_current * 0.1;
            updateMovingAverage(currentBuffer, current, CURRENT_WINDOW_SIZE, MAX_CURRENT_CHANGE);
            status.current = getMovingAverage(currentBuffer);
            lastValidCurrent = status.current;
        } else {
            status.current = lastValidCurrent;
        }

        if (status.valid) {
            status.power = status.voltage * status.current;
            status.capacity = calculateCapacity(status.voltage);
            status.isCharging = (status.current > 50.0f);
            
            if (status.isCharging) {
                float remainingCapacity = 100.0f - status.capacity;
                status.timeRemaining = (remainingCapacity / 100.0f) * 2.0f;
            } else if (status.current > 0) {
                status.timeRemaining = (status.capacity / 100.0f) * 2000.0f / status.current;
            }
        }

        return status;
    }
};