/**********************************************************************************************
*Copyright (C), 2017-2018 ,BV. AI. Technology Co., Ltd. All Rights Reserved.
*
*
*File name: BvSerial.h
*Description: 串口相关的结构体、变量和函数声明 
*Author:Dougz
*
*Date:2017-9-06
*
**********************************************************************************************/

#ifndef _BV_SERIAL_H_
#define _BV_SERIAL_H_


/******************************宏定义*********************************************/

enum
{
	UART_PORT_0 = 0,
	UART_PORT_1, 
	UART_PORT_2, 
	UART_PORT_3, 
	UART_PORT_4, 
	UART_PORT_NUM
};


enum
{
	UART_DATAT_BIT_7 = 7,
	UART_DATAT_BIT_8
};

enum
{
	UART_STOP_BIT_1 = 0,
	UART_STOP_BIT_2
};


#define  	UART_PORT_NUM 5

#define		ID_Open_Serial_Send		1
#define		ID_Close_Serial_Send	2

#define		ID_Stop_Cleaner			29
#define		ID_Pause_Play_Cleaner	20

#define		ID_Set_Distance_Radian	8

#define		ID_Set_Absolute_Radian	12
#define		ID_Set_Relative_Radian	13

#define		ID_Set_Linear_Angular	32
#define		ID_Set_Linaer			14

#define		ID_Set_Move_Model		16
#define		ID_Set_Side_Length		31	

#define		ID_Set_Position			34

#define     ID_Vision_Board_Up 		0xF0 
#define		ID_Set_Heading_Theta    0xF1
#define   	ID_Run_Around			0xF2
#define     ID_TEST_MAP				0xF3

#define   	ID_Send_Nav_Path		0xF4
#define   	ID_Search_State			0xF5
#define   	ID_Direction			0xF6
#define   	ID_FollowOver			0xF7
#define     ID_Init_Dir				0xF8



#define    ID_Report_Robot_State    0x2A
#define    ID_Vision_Mapping_Mode   0x29



//#define     serial_port_Debug 
//#define	  write_serial_file




/******************************结构体定义*****************************************/

typedef struct   
{
  char	buf[100];
}Packet_t;

typedef struct 
{
 unsigned short head;
 unsigned short length;
 unsigned char  command_ID;
 unsigned char  buffer[100];
}Eagle_Serial_t;

typedef struct 
{
	int 	left_encoder;
	int		right_encoder;
	short	gyro_z;
    short 	yaw;
	int		pose_x;
	int		pose_y;
	int		pose_t;
	
	short   leftwall;
	short   rightwall;
	char    irbumps;
	char 	bump;
	char    cliffs;
}cleaner_IMU_data_t;

typedef struct 
{
	
	int 	left_encoder;
	int		right_encoder;
	float	gyro_z;
    float 	yaw;
	float	pose_x;
	float	pose_y;
	float	pose_t;
	
	short   leftwall;
	short   rightwall;
	char    irbumps;
	char 	bump;
	char    cliffs;
}cleaner_IMU_data_in_eagle_t;

typedef enum  {
    NAVGETPATH_STATE_UNKNOW=0,
    NAVGETPATH_STATE_DOCKING=1,
    NAVGETPATH_STATE_BREAKING=2,
    NAVGETPATH_STATE_SEARCHING=3,
    NAVGETPATH_STATE_FAILING=4,
    NAVGETPATH_STATE_FRONTING=5
} NavGetPathstateOptions;


typedef struct 
{
	NavGetPathstateOptions  state;
	short x;
	short y;
}path_goal_pose_t;


/******************************全局变量声明***************************************/

extern int serial_data_flag;
extern Packet_t serial_packet;
extern volatile path_goal_pose_t goal_pose;
extern Eagle_Serial_t eagle_serial_recieve;
extern Eagle_Serial_t eagle_serial_send;
extern volatile cleaner_IMU_data_t cleaner_imu_data;




extern int Cleaner_Radian;
extern int Square_init_flag;
extern unsigned char Run_mode;
extern int serial_port;
extern int gawFdUart[];

extern volatile char posestateflag;

/******************************函数声明*******************************************/

extern int set_serial_open(void);
extern int set_serial_close(void);

extern int set_cleaner_stop(void);

//extern int set_cleaner_position(int pos_x, int pos_y, int pos_t);

extern void get_imu_data_from_cleanr(char* imudata);
extern void serial_read_callback(char* read_buffer, int read_buffer_length);

extern void read_cleaner_imu_data(cleaner_IMU_data_t *imu_data);
extern int set_linear_angular_velocity(short linear_vel,short angular_vel);
extern int set_linear_velocity(short linear_vel);
extern int set_serial_open(void);
extern int set_serial_close(void);
extern int set_cleaner_stop(void);
extern int set_cleaner_pause(void);
extern int set_distance_radian(short distance,short radian);
extern int set_absolute_radian(short absolute_radian);
extern int set_relative_radian(short relative_radian);
extern int set_move_model(unsigned char move_model);
extern int set_side_length(short side_length);
extern int set_cleaner_position(int state,int pos_x, int pos_y, int pos_t);
extern unsigned short calcCheckSum(Packet_t *packet);
extern int set_cleaner_run_around(void);
extern int set_heading_theta(int pos_t);
extern int set_cleaner_board_up(void);
extern int module_report_robot_state(int state);
extern int  set_vision_mapping_mode(void);
extern int sent_nav_path_data(unsigned char pointnum,char *pointbuf);
extern int sent_search_state(char state);
extern int sent_judge_direction(char dir);
extern int sent_follow_over_status(char noinfonum);
extern int sent_init_direction(char dir);






extern int set_opt(int fd,int wSpeed, int wBits, char cEvent, int wStop);
extern int bv_uart_open(int uart_port,int baud_rate);
extern void bv_uart_close(int uart_port);
extern int bv_uart_read(int uart_port, char* p_data_buf,int buf_size);
extern int bv_uart_write(int uart_port, char* p_data_buf,int buf_size);
extern void* bv_uart_thread(void *data);

#endif


