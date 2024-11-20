#ifndef MOTORS_H
#define MOTORS_H

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <chrono>
#include <thread>

class Motors
{
public:    
    // Constructor
    Motors()
    {
        // Opening i2c file
        int adapter_nr = 1;
        char filename[20];

        snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
        this->file = open(filename, O_RDWR);
        if (this->file < 0)
        {
            printf("Could not open %s.\n", filename);
            exit(1);
        }

        if (ioctl(this->file, I2C_SLAVE, this->addr) < 0)
        {
            printf("IO Control failed.\n");
            exit(1);
        }

        // Initializing file
        __u8 initial_val[70] = {0x01, 0x04, 0xe2, 0xe4, 0xe8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        __u8 last_val[6] = {0x00, 0x00, 0x00, 0x00, 0x03, 0x00};
        __u8 reg = 0;
        for (int i = 0; i < sizeof(initial_val); i++)
        {
            res = i2c_smbus_write_byte_data(this->file, reg, initial_val[i]);
            reg++;
        }
        reg = 0xfa;
        for (int i = 0; i < 6; i++)
        {
            res = i2c_smbus_write_byte_data(this->file, reg, last_val[i]);
            reg++;
        }
        printf("Motors initialized\n");
    };
    
    // Setters
    void setSpeed(double _speed)
    {
        if (_speed < 0.0)
        {
            _speed = 0.0;
        }
        else if (_speed > 1.0)
        {
            _speed = 1.0;
        }
        this->speed = _speed;
        this->speed_int = convert_speed(this->speed);
        this->s1 = val1(this->speed_int);
        this->s2 = val2(this->speed_int);
        if (this->left_forward)
        {
            left_motor_forward_start(true);
        }
        if (this->right_forward)
        {
            right_motor_forward_start(true);
        }
        if (this->left_backward)
        {
            left_motor_backward_start(true);
        }
        if (this->right_backward)
        {
            right_motor_backward_start(true);
        }
    };

    // Movement
    void moveForward()
    {
        if (this->left_backward)
        {
            left_motor_backward_stop();
        }
        if (this->right_backward)
        {
            right_motor_backward_stop();
        }
        left_motor_forward_start(false);
        right_motor_forward_start(false);
    };

    void moveBackward()
    {
        if (this->left_forward)
        {
            left_motor_forward_stop();
        }
        if (this->right_forward)
        {
            right_motor_forward_stop();
        }
        left_motor_backward_start(false);
        right_motor_backward_start(false);
    };

    void moveLeft()
    {
        if (this->left_forward)
        {
            left_motor_forward_stop();
        }
        if (this->right_backward)
        {
            right_motor_backward_stop();
        }
        left_motor_backward_start(false);
        right_motor_forward_start(false);
    };

    void moveRight()
    {
        if (this->left_backward)
        {
            left_motor_backward_stop();
        }
        if (this->right_forward)
        {
            right_motor_forward_stop();
        }
        left_motor_forward_start(false);
        right_motor_backward_start(false);
    };

    void moveStop()
    {
        left_motor_forward_stop();
        right_motor_forward_stop();
        left_motor_backward_stop();
        right_motor_backward_stop();
    }; 
    

private:
    const int addr = 0x60;
    int file;
    double speed = 0.0;
    int speed_int = 0;
    int s1 = 0x00;
    int s2 = 0x00;
    __s32 res;

    // Motor booleans
    bool left_forward = false;
    bool left_backward = false;
    bool right_forward = false;
    bool right_backward = false;


    // Registers
    __u8 left_forward_speed[4] = {0x0c, 0x0d, 0x28, 0x29};
    __u8 left_forward_minor[2] = {0x2b, 0x31};
    __u8 right_forward_speed[4] = {0x10, 0x11, 0x3c, 0x3d};
    __u8 right_forward_minor[2] = {0x35, 0x37};
    __u8 left_backward_speed[4] = {0x08, 0x09, 0x28, 0x29};
    __u8 left_backward_minor[2] = {0x2d, 0x2f};
    __u8 right_backward_speed[4] = {0x14, 0x15, 0x3c, 0x3d};
    __u8 right_backward_minor[2] = {0x33, 0x39};

    void left_motor_forward_start(bool speed_changed)
    {
        if (!left_forward || speed_changed)
        {
            for (int i = 0; i < sizeof(left_forward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, left_forward_minor[i], 0x10);
            }
            for (int i = 0; i < sizeof(left_forward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, left_forward_speed[i], s1);
                res = i2c_smbus_write_byte_data(file, left_forward_speed[i+1], s2);
            }
            left_forward = true;
        }
        
    };

    void left_motor_forward_stop()
    {
        if (left_forward)
        {
            for (int i = 0; i < sizeof(left_forward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, left_forward_minor[i], 0x00);
            }
            for (int i = 0; i < sizeof(left_forward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, left_forward_speed[i], 0x00);
                res = i2c_smbus_write_byte_data(file, left_forward_speed[i+1], 0x00);
            }
            left_forward = false;
        }
    };

    void right_motor_forward_start(bool speed_changed)
    {
        if (!right_forward || speed_changed)
        {
            for (int i = 0; i < sizeof(right_forward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, right_forward_minor[i], 0x10);
            }
            for (int i = 0; i < sizeof(right_forward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, right_forward_speed[i], s1);
                res = i2c_smbus_write_byte_data(file, right_forward_speed[i+1], s2);
            }
            right_forward = true;
        }
    };

    void right_motor_forward_stop()
    {
        if (right_forward)
        {
            for (int i = 0; i < sizeof(right_forward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, right_forward_minor[i], 0x00);
            }
            for (int i = 0; i < sizeof(right_forward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, right_forward_speed[i], 0x00);
                res = i2c_smbus_write_byte_data(file, right_forward_speed[i+1], 0x00);
            }
            right_forward = false;
        }
    };

    void left_motor_backward_start(bool speed_changed)
    {
        if (!left_backward || speed_changed)
        {
            for (int i = 0; i < sizeof(left_backward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, left_backward_minor[i], 0x10);
            }
            for (int i = 0; i < sizeof(left_backward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, left_backward_speed[i], s1);
                res = i2c_smbus_write_byte_data(file, left_backward_speed[i+1], s2);
            }
            left_backward = true;
        }
        
    };

    void left_motor_backward_stop()
    {
        if (left_backward)
        {
            for (int i = 0; i < sizeof(left_backward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, left_backward_minor[i], 0x00);
            }
            for (int i = 0; i < sizeof(left_backward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, left_backward_speed[i], 0x00);
                res = i2c_smbus_write_byte_data(file, left_backward_speed[i+1], 0x00);
            }
            left_backward = false;
        }
    };

    void right_motor_backward_start(bool speed_changed)
    {
        if (!right_backward || speed_changed)
        {
            for (int i = 0; i < sizeof(right_backward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, right_backward_minor[i], 0x10);
            }
            for (int i = 0; i < sizeof(right_backward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, right_backward_speed[i], s1);
                res = i2c_smbus_write_byte_data(file, right_backward_speed[i+1], s2);
            }
            right_backward = true;
        }
    };

    void right_motor_backward_stop()
    {
        if (right_backward)
        {
            for (int i = 0; i < sizeof(right_backward_minor); i++)
            {
                res = i2c_smbus_write_byte_data(file, right_backward_minor[i], 0x00);
            }
            for (int i = 0; i < sizeof(right_backward_speed); i += 2)
            {
                res = i2c_smbus_write_byte_data(file, right_backward_speed[i], 0x00);
                res = i2c_smbus_write_byte_data(file, right_backward_speed[i+1], 0x00);
            }
            right_backward = false;
        }
    };

    __s32 i2c_smbus_access(int file, char read_write, __u8 command,
           int size, union i2c_smbus_data *data)
    {
        struct i2c_smbus_ioctl_data args;
        __s32 err;

        args.read_write = read_write;
        args.command = command;
        args.size = size;
        args.data = data;
        err = ioctl(file, I2C_SMBUS, &args);
        if (err == -1)
            err = -errno;
        return err;
    };

    __s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value)
    {
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
                I2C_SMBUS_BYTE_DATA, &data);
    };

    int convert_speed(double speed)
    {
        int s = (int) (255 * speed * 16);
        return s;
    };

    __u8 val1(int s)
    {
        return s & 0xff;
    };

    __u8 val2(int s)
    {
        return s >> 8;
    };
};

#endif