#pragma once
//std headers
#include <string>
#include <vector>
#include <chrono>
#include <iostream>

// Unix headers
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>


struct ServoData
{
    int16_t current_angle;
    int16_t current_speed;
};

class Servos
{
public:
    Servos(std::string devicename = "/dev/ttyTHS1",int baudrate = 1000000)
    {
        port = ::open(devicename.c_str() ,O_RDWR | O_NOCTTY | O_SYNC);
        struct termios tty;

        if(tcgetattr(port,&tty)!=0)
        {
            return;
        }
        cfmakeraw(&tty);
        tty.c_iflag &= ~(IXON | IXOFF);

        cfsetspeed(&tty, baudrate);
        tty.c_cc[VTIME] = 0;
        tty.c_cc[VMIN] = 0;
        int err = ::tcsetattr(port, TCSAFLUSH, &tty);
        if (err != 0) {
            ::close(port);
            port = -1;
        }
    }
    ~Servos()
    {
        close(port);
    }
    bool isConnected() 
    { 
    return (port != -1);
    } 
    void setServoAngle(uint8_t scs_id, int16_t angle)
    {
        int16_t offset = 512 + (SERVO_INPUT_RANGE * angle)/SERVO_ANGLE_RANGE;
    uint8_t txpacket[7 + 4];        
    txpacket[PKT_ID] = 254;             /* ID of broadcast message */

	txpacket[PKT_LENGTH] = 3 + 4;       /* SCS_ID + OFFSET = 3 bytes */
                                        /* INSTRUCTION + ADDRESS + DATA_LENGTH + CHECKSUM */
	txpacket[PKT_INSTRUCTION] = INST_SYNC_WRITE;

	txpacket[PKT_PARAMETER0 ] = ADDR_STS_GOAL_POSITION;
	txpacket[PKT_PARAMETER0 + 1] = 2;   /* DATA_LENGTH */
	txpacket[PKT_PARAMETER0 + 2] = scs_id;

    /* Insert data with swapped ednianess */
    txpacket[PKT_PARAMETER0 + 3] = offset >>8;
    txpacket[PKT_PARAMETER0 + 4] = offset & 0xFF;

    txpacket[PKT_HEADER0] = 0xFF;
    txpacket[PKT_HEADER1] = 0xFF;

    uint8_t checksum = 0;
	for(int i=2; i < 10;i++)
	{
        checksum += txpacket[i];
	}
	checksum = ~checksum & 0xFF; //Flip bits
	txpacket[10] = checksum;

    ssize_t written_length = write(port,txpacket,11);
	if (written_length != 11)
    {
        std::cerr<<"Erro writing to port\n";
    }
    }
    int setServoSpeed(uint8_t scs_id, uint16_t speed)
    {
        using namespace std::chrono_literals;
    
    std::vector<uint8_t> txpacket; /* Sends instruction */
    std::vector<uint8_t> rxpacket; /* Reads result of instruction */

    txpacket.resize(9);
    txpacket[PKT_HEADER0] = 0xFF;
    txpacket[PKT_HEADER1] = 0xFF;
    txpacket[PKT_ID] = scs_id;
	txpacket[PKT_LENGTH] = 2 + 3;       /* SPEED: 2 bytes. */
                                        /* INSTRUCTION + ADDRESS + CHECKSUM : 3 bytes */

    txpacket[PKT_INSTRUCTION] = INST_WRITE;
    txpacket[PKT_PARAMETER0] = ADDR_STS_GOAL_SPEED; 

    /* Insert data with swapped endianess */
    txpacket[PKT_PARAMETER0 + 1] = speed >> 8;
    txpacket[PKT_PARAMETER0 + 2] = speed & 0xFF;



    /* Calculate the checksum */
    uint8_t checksum = 0;
	for(int i=2; i < 8;i++)
	{
        checksum += txpacket[i];
	}
	checksum = ~checksum & 0xFF; /* Flip bits */
	txpacket[8] = checksum;

    /* Write to device */
    ssize_t written_length = write(port,&txpacket[0],9);
    if (written_length != 9)
    {
        std::cerr << "Could not write to port\n";
        return COMM_TX_ERR;
    }
    
    
    /* Read rx until timeout or data found*/
    auto timeout = std::chrono::steady_clock::now() 
            + std::chrono::duration<double,std::milli>(2ms);
    int rx_result = COMM_RX_EMPTY;
    while(rx_result == COMM_RX_EMPTY )
    {
        rx_result = getRawRx(rxpacket);
        auto current_timeout = timeout - std::chrono::steady_clock::now();
        if(current_timeout <= 0ms)
            break;   
    }
    if( rx_result == COMM_SUCCESS)
    {
        if(rxpacket[PKT_ERROR] != SERVO_SUCCESS)
        {
            std::cerr<<"Servo error: " << rxpacket[PKT_ERROR]<<std::endl;
            return (int)rxpacket[PKT_ERROR];
        }
        return COMM_SUCCESS;
    }
    else if(rx_result==COMM_RX_EMPTY) 
    {
        std::cerr<<"Packet timeout!"<<std::endl;
        return COMM_RX_TIMEOUT;
    }
    else
    {
        std::cerr<<"Corrupt packet!"<<std::endl;
        return COMM_RX_CORRUPT;
    }
    }
    ServoData getServoData(uint8_t scs_id)
    {
        using namespace std::chrono_literals;
    uint8_t txpacket[8];

    txpacket[PKT_HEADER0] = 0xFF;
    txpacket[PKT_HEADER1] = 0xFF;
    txpacket[PKT_ID] = scs_id;
    txpacket[PKT_LENGTH] = 4;
    txpacket[PKT_INSTRUCTION] = INST_READ;
    txpacket[PKT_PARAMETER0 + 0] = ADDR_SCS_PRESENT_POSITION;
    txpacket[PKT_PARAMETER0 + 1] = 4;

    int checksum = 0;
	for(int i=2; i < 7;i++)
	{
        checksum += txpacket[i];
	}
    checksum = ~checksum & 0xFF; /* Flip bits */
	txpacket[7] = checksum;

    ssize_t written_length = write(port,txpacket,8);
    if (written_length != 8)
    {
        std::cerr <<"Error writing to port\n";
    }
    
    //Read result
    std::vector<uint8_t> rxpacket;
    ServoData servo_data;
    //servo_data.success = false;
    int result;

    auto timeout = std::chrono::steady_clock::now() 
                + std::chrono::duration<double,std::milli>(5ms);
    while(true)
    {  
        result = getRawRx(rxpacket);
        const auto current_timeout  = timeout - std::chrono::steady_clock::now();
    
        if(current_timeout <= 0ms)
        {
            result = COMM_RX_TIMEOUT;
            break;
        }
        else if(result == COMM_RX_CORRUPT) 
        {
            break;
        }
        else if( (rxpacket[PKT_ID] == scs_id) && rxpacket[PKT_LENGTH] == 6 )
        {
            uint16_t offset = uint16_t((rxpacket[5]) << 8) + rxpacket[6];
            servo_data.current_angle = (SERVO_ANGLE_RANGE*(offset-512))/SERVO_INPUT_RANGE;
            servo_data.current_speed = uint16_t(rxpacket[7] << 8) + rxpacket[8];
            if(servo_data.current_speed < 0)
                servo_data.current_speed = -32768 - servo_data.current_speed;
            //servo_data.success = true;
            break;
        }
    }
    return servo_data;
    }
private:
    // Packet Fields
    const int PKT_HEADER0 = 0;
    const int PKT_HEADER1 = 1;
    const int PKT_ID = 2;
    const int PKT_LENGTH = 3;
    const int PKT_INSTRUCTION = 4;
    const int PKT_ERROR = 4;
    const int PKT_PARAMETER0 = 5;

    // Addresses
    const int ADDR_STS_GOAL_SPEED = 46;
    const int ADDR_STS_GOAL_POSITION = 42;
    const int ADDR_SCS_PRESENT_POSITION  = 56;

    // Instructions
    const int INST_WRITE = 3;
    const int INST_READ = 2;
    const int INST_SYNC_WRITE = 131;

    // Angle Constants
    const int SERVO_INPUT_RANGE = 850;
    const int SERVO_ANGLE_RANGE = 180;

    // ERROR CODES
    const int COMM_SUCCESS = 0;
    const int COMM_RX_EMPTY = 1;

    const int COMM_TX_ERR = -1;
    const int COMM_RX_CORRUPT = -2;
    const int COMM_RX_TIMEOUT = -3;
    const int SERVO_ERR = -4;

    // SERVO ERRORS
    const int SERVO_SUCCESS = 0;
    int getRawRx(std::vector<uint8_t>& rxpacket)
    {
        rxpacket.resize(6); /* 6 is minimum size */
    int result = COMM_SUCCESS;
    int bytes_read = 0;
    int bytes_to_read = 6; 
    

    bytes_read = read(port, &rxpacket[0], bytes_to_read);


    if( bytes_read != bytes_to_read)
    {
        result = COMM_RX_EMPTY;
    }
    else if( rxpacket[0] != 0xFF && rxpacket [0] !=0xFF )
    {
        result = COMM_RX_CORRUPT;
    }
    else if(rxpacket[PKT_LENGTH]>2)
    {
        bytes_to_read = rxpacket[PKT_LENGTH] - 2;
        rxpacket.resize( 6 + bytes_to_read);
        bytes_read = read(port, &rxpacket[6], bytes_to_read);
        if(bytes_read != bytes_to_read)
        {
            result = COMM_RX_CORRUPT;
        }
    }

    if(result != COMM_SUCCESS)
    {
        rxpacket.clear();
    }

    return result;
    }
    int port = -1;
};







