/**********************************************************************************************
*Copyright (C), 2017-2018 ,BV. AI. Technology Co., Ltd. All Rights Reserved.
*
*
*File name: BvSerial.c
*Description: 实现serial处理相关的函数及相关变量的定义
*Author:Dougz
*
*Date:2017-9-06
*
**********************************************************************************************/
#include "BvSerial.h"
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/prctl.h>

/******************************全局变量定义***************************************/
int serial_data_flag = false;
Packet_t serial_packet;

Eagle_Serial_t eagle_serial_recieve;
Eagle_Serial_t eagle_serial_send;
volatile cleaner_IMU_data_t cleaner_imu_data;
volatile path_goal_pose_t goal_pose;

int Cleaner_Radian=0;
int Square_init_flag=0;
unsigned char Run_mode=1;
int serial_port=0;
int gawFdUart[UART_PORT_NUM] = {-1, -1, -1, -1, -1};

volatile char posestateflag=0;

/******************************函数定义*******************************************/

/******************************************************************************
*函数名称:set_serial_open（现在可以不用）
*功能描述:  用于获取扫地机上发数据
*全局影响:无
*输入:无
*输出:无
*返回值:串口发送数据个数 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_serial_open(void)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Open_Serial_Send;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}



/******************************************************************************
*函数名称:set_serial_close
*功能描述: 关闭后扫地机不再发送数据 
*全局影响:无
*输入:无
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_serial_close(void)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Close_Serial_Send;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_cleaner_stop
*功能描述:扫地机不再工作  
*全局影响:无
*输入:无
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_cleaner_stop(void)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Stop_Cleaner;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_cleaner_position
*功能描述:修正扫地机坐标  
*全局影响:无
*输入:state:跟踪状态 坐标值与角度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_cleaner_position(int state,int pos_x, int pos_y, int pos_t)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x00010;
	eagle_serial_send.command_ID = ID_Set_Position;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	

	serial_packet.buf[5] = (char)(state);
	serial_packet.buf[6] = (char)(pos_x >> 24);
	serial_packet.buf[7] = (char)(pos_x >> 16);
	serial_packet.buf[8] = (char)(pos_x >> 8);
	serial_packet.buf[9] = (char)(pos_x >> 0);
	
	serial_packet.buf[10]  = (char)(pos_y >> 24);
	serial_packet.buf[11] = (char)(pos_y >> 16);
	serial_packet.buf[12] = (char)(pos_y >> 8);
	serial_packet.buf[13] = (char)(pos_y >> 0);
	
	serial_packet.buf[14] = (char)(pos_t >> 24);
	serial_packet.buf[15] = (char)(pos_t >> 16);
	serial_packet.buf[16] = (char)(pos_t >> 8);
	serial_packet.buf[17] = (char)(pos_t >> 0);
	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[18] = (char)(check_sum >> 8);
	serial_packet.buf[19] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}



/******************************************************************************
*函数名称:set_cleaner_board_up
*功能描述:用于告知扫地机可以旋转，从而视觉模块可以获得图像
*全局影响:无
*输入:无
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/

int set_cleaner_board_up(void)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Vision_Board_Up;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}

/******************************************************************************
*函数名称:set_heading_theta
*功能描述:设置扫地机行进方向
*全局影响:无
*输入:扫地机行进方向角度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/

int set_heading_theta(int pos_t)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x00007;
	eagle_serial_send.command_ID = ID_Set_Heading_Theta;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(pos_t >> 24);
	serial_packet.buf[6] = (char)(pos_t >> 16);
	serial_packet.buf[7] = (char)(pos_t >> 8);
	serial_packet.buf[8] = (char)(pos_t >> 0);

	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[9] = (char)(check_sum >> 8);
	serial_packet.buf[10] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}
/******************************************************************************
*函数名称:set_cleaner_run_around
*功能描述:在行进过程中当视觉模块丢失一段时间后调用此函数
*全局影响:无
*输入:无
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/

int set_cleaner_run_around(void)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Run_Around;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:get_imu_data_from_cleanr
*功能描述:转换扫地机原始数据，用于算法计算  
*全局影响:无
*输入:
*输出:扫地机数据
*返回值: 
*作者: 
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
void get_imu_data_from_cleanr(char* imudata)
{
	cleaner_IMU_data_t cleaner_imu_data_temp;
	cleaner_IMU_data_in_eagle_t clr_eagle_imu_data;

     //call fun 
     read_cleaner_imu_data((cleaner_IMU_data_t *)&cleaner_imu_data_temp);
	 clr_eagle_imu_data.left_encoder = cleaner_imu_data_temp.left_encoder;
	 clr_eagle_imu_data.right_encoder = cleaner_imu_data_temp.right_encoder;
	 clr_eagle_imu_data.gyro_z = cleaner_imu_data_temp.gyro_z/100.0;
	 clr_eagle_imu_data.yaw = cleaner_imu_data_temp.yaw/100.0;
	 clr_eagle_imu_data.pose_x = cleaner_imu_data_temp.pose_x/10000.0;
	 clr_eagle_imu_data.pose_y = cleaner_imu_data_temp.pose_y/10000.0;
	 clr_eagle_imu_data.pose_t = cleaner_imu_data_temp.pose_t/10000.0;
	 
	 clr_eagle_imu_data.leftwall = cleaner_imu_data_temp.leftwall;
	 clr_eagle_imu_data.rightwall = cleaner_imu_data_temp.rightwall;
	 clr_eagle_imu_data.irbumps = cleaner_imu_data_temp.irbumps;
	 clr_eagle_imu_data.bump = cleaner_imu_data_temp.bump;
	 clr_eagle_imu_data.cliffs = cleaner_imu_data_temp.cliffs;
	 memcpy(imudata,&clr_eagle_imu_data,sizeof(cleaner_IMU_data_in_eagle_t));
}




/******************************************************************************
*函数名称:serial_read_callback
*功能描述:接收扫地机串口数据  
*全局影响:无
*输入:串口buffer与buffer长度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
void  serial_read_callback(char* read_buffer, int read_buffer_length)
{
//	int length = 0;
//	int i= 0;
//	int rtn;
//	static cleaner_IMU_data_in_eagle_t	clr_imu_data;
//	volatile static char headflag=0;
//	volatile static char tailflag=0;
//	volatile static int  tindex=0;
//	volatile static int  rindex=0;
//	volatile static char tbuff[128]={0};
//
//
//	length=read_buffer_length;
//	//printf("\n Recieve Callback %d",length);
//	tindex=0;
//
//	if(headflag==0)
//	{
//		while(tindex<length-1)
//		{
//			eagle_serial_recieve.head =((((unsigned short)read_buffer[tindex])<<8) + read_buffer[tindex+1]);
//			if(eagle_serial_recieve.head==0xB562)//查找帧头
//			{
//				headflag=1;
//				break;
//			}
//			else
//			tindex++;
//
//		}
//
//	}
//
//	if (headflag==1)
//	{
//		while (tindex<length-1)
//		{
//			if (tailflag==0)
//			{
//				if (((length-tindex)>3)&&(rindex==0))//buff还没存数据，计算出长度
//				{
//					eagle_serial_recieve.length = ((((unsigned short)read_buffer[tindex+2])<<8) + read_buffer[tindex+3])+4;
//				}
//
//				if (eagle_serial_recieve.length>100)//长度出错 直接丢弃数据
//				{
//					headflag=0;
//					tailflag=0;
//					rindex=0;
//					tindex=0;
//					return;
//				}
//
//				if((length-tindex)>=(eagle_serial_recieve.length-rindex))
//				{
//					int tn=eagle_serial_recieve.length-rindex;
//					for(int i=0;i<(tn);i++)
//					{
//						tbuff[rindex++] = read_buffer[tindex+i];
//					}
//					tailflag=1;
//					tindex+=tn;
//				}
//				else
//				{
//					for(int i=0;i<(length);i++)
//					{
//						tbuff[rindex++]=read_buffer[tindex+i];
//						if(rindex==eagle_serial_recieve.length)
//						{
//						tailflag=1;
//						tindex+=i+1;
//						break;
//						}
//					}
//					tindex+=(length-tindex);
//				}
//			}
//
//			if (tailflag==1)
//			{
//			    eagle_serial_recieve.command_ID = tbuff[4];
//				if((eagle_serial_recieve.head == 0xB562)&&(eagle_serial_recieve.command_ID==0x61))
//					{
//						if(posestateflag==0)
//						{
//							goal_pose.state=(NavGetPathstateOptions)tbuff[5];
//							goal_pose.x=((short)tbuff[6]<<8)+tbuff[7];
//							goal_pose.y=((short)tbuff[8]<<8)+tbuff[9];
//							posestateflag=1;
//						}
//					}
//				else if((eagle_serial_recieve.head == 0xB562)&&(eagle_serial_recieve.command_ID==0x30))
//				{
//					//memset((char *)&cleaner_imu_data,0,sizeof(cleaner_IMU_data_in_eagle_t));
//					//cleaner_imu_data.left_encoder=(((int)tbuff[5])<<24)+(((int)tbuff[6])<<16)+(((int)tbuff[7])<<8)+tbuff[8];
//					//cleaner_imu_data.right_encoder=(((int)tbuff[9])<<24)+(((int)tbuff[10])<<16)+(((int)tbuff[11])<<8)+tbuff[12];
//					//for (i=0;i<4;i++)
//					//{
//					// cleaner_imu_data.left_encoder+=((int)tbuff[5+i])<<((3-i)*8);//buf[5]~[buf[8]
//					//
//					//}
//					//for (i=0;i<4;i++)
//					//{
//					// cleaner_imu_data.right_encoder+=((int)tbuff[9+i])<<((3-i)*8);//buf[9]~[buf[12]
//					//}
//					cleaner_imu_data.gyro_z=0;
//					cleaner_imu_data.yaw=0;//改成方向了为了配合地图
//
//                    if(Global_Set_Heading==0)
//                    	{
//							if(tbuff[12])
//							Global_Set_Heading=1;
//                    	}
//					Global_Init_JudgeDir=tbuff[13];
//					Global_FollowOverstatus=tbuff[14];
//					Global_FlightStatus=tbuff[15];
//					Global_JudgeDirection=tbuff[16];
//
//					if (Square_init_flag==0)
//					{
//						Square_init_flag=1;
//					}
//
//					else if (Square_init_flag==1)
//					{
//						Cleaner_Radian=(cleaner_imu_data.yaw*3.14159)/18;
//					}
//
//					cleaner_imu_data.pose_x=(((int)tbuff[17])<<24)+(((int)tbuff[18])<<16)+(((int)tbuff[19])<<8)+tbuff[20];
//					cleaner_imu_data.pose_y=(((int)tbuff[21])<<24)+(((int)tbuff[22])<<16)+(((int)tbuff[23])<<8)+tbuff[24];
//					cleaner_imu_data.pose_t=(((int)tbuff[25])<<24)+(((int)tbuff[26])<<16)+(((int)tbuff[27])<<8)+tbuff[28];
//
//
//
//					Global_Radian=cleaner_imu_data.pose_t;
//					Global_x=cleaner_imu_data.pose_x;
//					Global_y=cleaner_imu_data.pose_y;
//					Global_t=cleaner_imu_data.pose_t;
//
//					cleaner_imu_data.leftwall=(((int)tbuff[29])<<8) + tbuff[30];
//					cleaner_imu_data.rightwall=(((int)tbuff[31])<<8) + tbuff[32];
//					cleaner_imu_data.irbumps=tbuff[33];
//					cleaner_imu_data.bump=tbuff[34];
//					cleaner_imu_data.cliffs=tbuff[35];
//
//					/*if(posestateflag==0)
//						{
//							goal_pose.state=(NavGetPathstateOptions)tbuff[36];
//							goal_pose.x=((short)tbuff[37]<<8)+tbuff[38];
//							goal_pose.y=((short)tbuff[39]<<8)+tbuff[40];
//							posestateflag=1;
//						}*/
//				    //bv_logi("IMU_Clisffs:%d\n",cleaner_imu_data.cliffs);
//
//					gridstate.flag=cleaner_imu_data.bump;
//
//
//			         /*cleaner imu data*/
//					 get_imu_data_from_cleanr((char *)&clr_imu_data);
//					 rtn = bv_feed_imudata(g_mHub,&clr_imu_data);
//
//					 if (0 == rtn)
//					 {
//						 serial_data_flag = TRUE;
//						 bv_logi("bv_feed_imudata ok! serial_data_flag=%d \n",serial_data_flag);
//					 }
//
//					 #if 0 /*如需调试imu信息，可打开该开关，imu数据写入log文件*/
//					 uint64_t sys_time_usec;
//					 uint64_t delta_time;
//					 sys_time_usec = bv_get_sys_time_usec();
//					 delta_time = sys_time_usec - g_cleanr_sys_time_usec_pre;
//					 g_cleanr_sys_time_usec_pre = sys_time_usec;
//
//					 sprintf(imufilename,"/mnt/UDISK/bvlog/imudata_cleaner.log");
//					 imutxtfd = fopen(imufilename,"a+");
//					 if (NULL == imutxtfd)
//					 {
//						bv_logw("fopen imudata_cleaner Error! \n");
//						sprintf(imudata,"fopen imudata_cleaner Error! imutxtfd=%d \n",imutxtfd);
//						fwrite(imudata,strlen(imudata),1,imutxtfd);
//					 }
//					 else
//					 {
//						 /* imu data*/
//						 sprintf(imudata,"%llu %llu %d %d %f %f %f %f %f %d\r\n", sys_time_usec,delta_time,
//						 clr_imu_data.left_encoder,clr_imu_data.right_encoder,clr_imu_data.gyro_z,clr_imu_data.yaw,
//						 clr_imu_data.pose_x,clr_imu_data.pose_y,clr_imu_data.pose_t,clr_imu_data.cliffs);
//
//						 fwrite(imudata,strlen(imudata),1,imutxtfd);
//						 fflush(imutxtfd);
//						 fclose(imutxtfd);
//					 }
//					 #endif
//
//				}
//
//				headflag=0;
//				tailflag=0;
//				rindex=0;
//			}
//		}
//		tindex=0;
//	}

}


/******************************************************************************
*函数名称:read_cleaner_imu_data
*功能描述:读取扫地机数据  
*全局影响:无
*输入:读取扫地机数据
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
void read_cleaner_imu_data(cleaner_IMU_data_t *imu_data)
{
	imu_data->left_encoder = cleaner_imu_data.left_encoder;
	imu_data->right_encoder = cleaner_imu_data.right_encoder;
	imu_data->gyro_z = cleaner_imu_data.gyro_z;
	imu_data->yaw = cleaner_imu_data.yaw;
	imu_data->pose_x = cleaner_imu_data.pose_x;
	imu_data->pose_y = cleaner_imu_data.pose_y;
	imu_data->pose_t = cleaner_imu_data.pose_t;
	imu_data->leftwall = cleaner_imu_data.leftwall;
	imu_data->rightwall = cleaner_imu_data.rightwall;
	imu_data->irbumps = cleaner_imu_data.irbumps;
	imu_data->bump	= cleaner_imu_data.bump;
	imu_data->cliffs = cleaner_imu_data.cliffs;//对地检测
}

/******************************************************************************
*函数名称:calcCheckSum
*功能描述:计算串口发送的检验值 
*全局影响:无
*输入:要发送的数据包
*输出:无
*返回值: 校验值
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
unsigned short calcCheckSum(Packet_t *packet) 
{
    int i;
    unsigned char n;
    unsigned short c = 0;

    i = 4;
    n = ((packet->buf[2] << 8) | packet->buf[3])  - 2;
    while (n > 1)
    {
        c += ((unsigned char)packet->buf[i] << 8) | (unsigned char)packet->buf[i + 1];
        c = c & 0xffff;
        n -= 2;
        i += 2;
    }
    if (n > 0)
        c = c ^ (unsigned short)((unsigned char) packet->buf[i]);
    return c;
}


/******************************************************************************
*函数名称:set_linear_angular_velocity
*功能描述:设置扫地机线速度角速度  
*全局影响:无
*输入:线速度值与角速度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_linear_angular_velocity(short linear_vel,short angular_vel)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0007;
	eagle_serial_send.command_ID = ID_Set_Linear_Angular;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(linear_vel >> 8);
	serial_packet.buf[6] = (char)(linear_vel );
	
	serial_packet.buf[7] = (char)(angular_vel >> 8);
	serial_packet.buf[8] = (char)(angular_vel );
	
	check_sum = calcCheckSum(&serial_packet);
	
	serial_packet.buf[9]  = (char)(check_sum >> 8);
	serial_packet.buf[10] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_distance_radian
*功能描述:设定行走距离和旋转角度  
*全局影响:无
*输入:距离值 角度值（弧度）
*输出:无
*返回值: 
*作者:chenbin 
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_distance_radian(short distance,short radian)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0007;
	eagle_serial_send.command_ID = ID_Set_Distance_Radian;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(distance >> 8);
	serial_packet.buf[6] = (char)(distance );
	
	serial_packet.buf[7] = (char)(radian >> 8);
	serial_packet.buf[8] = (char)(radian );
	
	check_sum = calcCheckSum(&serial_packet);
	
	serial_packet.buf[9] = (char)(check_sum >> 8);
	serial_packet.buf[10] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_linear_velocity
*功能描述:设置线速度  
*全局影响:无
*输入:线速度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_linear_velocity(short linear_vel)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0005;
	eagle_serial_send.command_ID = ID_Set_Linaer;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(linear_vel >> 8);
	serial_packet.buf[6] = (char)(linear_vel );
	
	check_sum = calcCheckSum(&serial_packet);
	
	serial_packet.buf[7] = (char)(check_sum >> 8);
	serial_packet.buf[8] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_relative_radian
*功能描述:让扫地机旋转一个相对角度  
*全局影响:无
*输入:角度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_relative_radian(short relative_radian)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0005;
	eagle_serial_send.command_ID = ID_Set_Relative_Radian;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(relative_radian >> 8);
	serial_packet.buf[6] = (char)(relative_radian );
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[7] = (char)(check_sum >> 8);
	serial_packet.buf[8] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_absolute_radian
*功能描述:让扫地机旋转一个绝对角度
*全局影响:无
*输入:角度值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_absolute_radian(short absolute_radian)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0005;
	eagle_serial_send.command_ID = ID_Set_Absolute_Radian;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(absolute_radian >> 8);
	serial_packet.buf[6] = (char)(absolute_radian );
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[7] = (char)(check_sum >> 8);
	serial_packet.buf[8] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_cleaner_pause
*功能描述:暂停/启动
*全局影响:无
*输入:无
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_cleaner_pause(void)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Pause_Play_Cleaner;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}

/******************************************************************************
*函数名称:set_move_model
*功能描述: 设置扫地机行走模式 
*全局影响:无
*输入:1-沿墙 2-单间 3-螺旋 4-弓字
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_move_model(unsigned char move_model)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0004;
	eagle_serial_send.command_ID = ID_Set_Move_Model;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(move_model );
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[6] = (char)(check_sum >> 8);
	serial_packet.buf[7] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


/******************************************************************************
*函数名称:set_side_length
*功能描述: 设置行走方形时边长 
*全局影响:无
*输入:short side_length 边长值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/
int set_side_length(short side_length)
{
	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0005;
	eagle_serial_send.command_ID = ID_Set_Side_Length;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(side_length >> 8);
	serial_packet.buf[6] = (char)(side_length );
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[7] = (char)(check_sum >> 8);
	serial_packet.buf[8] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}



/******************************************************************************
*函数名称:module_report_robot_state
*功能描述: 下发扫地机状态
*全局影响:无
*输入:short side_length 边长值
*输出:无
*返回值: 
*作者: chenbin
*修改记录:
*2017-x-x　新建
*
*
******************************************************************************/

int module_report_robot_state(int state)//下发扫地机状态 1被卡死 2被大幅度移动
{
 	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0004;
	eagle_serial_send.command_ID = ID_Report_Robot_State;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	
	serial_packet.buf[5] = (char)(state );
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[6] = (char)(check_sum >> 8);
	serial_packet.buf[7] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


int  set_vision_mapping_mode(void)
{

	short check_sum = 0;
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 0x0003;
	eagle_serial_send.command_ID = ID_Vision_Mapping_Mode;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID);	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5] = (char)(check_sum >> 8);
	serial_packet.buf[6] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


int sent_nav_path_data(unsigned char pointnum,char *pointbuf)
{
	short check_sum = 0;
	
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = pointnum*12+3;
	eagle_serial_send.command_ID = ID_Send_Nav_Path;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	

	for(int i=0;i<pointnum*12;i++)
	serial_packet.buf[5+i] = pointbuf[i];
	
	
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[5+pointnum*12] = (char)(check_sum >> 8);
	serial_packet.buf[6+pointnum*12] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}

int sent_search_state(char state)
{
	short check_sum = 0;
	
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 4;
	eagle_serial_send.command_ID = ID_Search_State;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	serial_packet.buf[5] = state;
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[6] = (char)(check_sum >> 8);
	serial_packet.buf[7] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}


int sent_judge_direction(char dir)
{
	short check_sum = 0;
	
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 4;
	eagle_serial_send.command_ID = ID_Direction;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	serial_packet.buf[5] = dir;
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[6] = (char)(check_sum >> 8);
	serial_packet.buf[7] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}

int sent_init_direction(char dir)
{
	short check_sum = 0;
	
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 4;
	eagle_serial_send.command_ID = ID_Init_Dir;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	serial_packet.buf[5] = dir;
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[6] = (char)(check_sum >> 8);
	serial_packet.buf[7] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}






int sent_follow_over_status(char noinfonum)
{
	short check_sum = 0;
	
	eagle_serial_send.head = 0xB562;
	eagle_serial_send.length = 4;
	eagle_serial_send.command_ID = ID_FollowOver;
	
	serial_packet.buf[0] = (char)(eagle_serial_send.head >> 8);
	serial_packet.buf[1] = (char)(eagle_serial_send.head );
	serial_packet.buf[2] = (char)(eagle_serial_send.length >> 8);
	serial_packet.buf[3] = (char)(eagle_serial_send.length );
	serial_packet.buf[4] = (char)(eagle_serial_send.command_ID );	
	serial_packet.buf[5] = noinfonum;
	check_sum = calcCheckSum(&serial_packet);
	serial_packet.buf[6] = (char)(check_sum >> 8);
	serial_packet.buf[7] = (char)(check_sum );
	
	#ifdef serial_port_Debug
	for (int i=0; i<(eagle_serial_send.length+4);i++)
	{
	 bv_logd("%02x ",serial_packet.buf[i]);		
	}
	#endif
	
	
	return (bv_uart_write(serial_port, serial_packet.buf, eagle_serial_send.length+4));
}



/******************************************************************************
*函数名称:set_opt
*功能描述:配置串口参数的通用函数
*全局影响:无
*输入:int fd,int wSpeed, int wBits, char cEvent, int wStop
*输出:无
*返回值:-1：失败；0：成功
*作者:窦广正
*修改记录:
*2017-7-6 窦广正　新建
*
*
******************************************************************************/


int set_opt(int fd,int wSpeed, int wBits, char cEvent, int wStop)
{
    struct termios newtio, oldtio;

	/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if (tcgetattr(fd,&oldtio) != 0) {
        //perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));

	/*步骤一，设置字符大小*/
	newtio.c_cflag |= CLOCAL|CREAD;     //CLOCAL:忽略modem控制线  CREAD：打开接受者
    newtio.c_cflag &= ~CSIZE;           //字符长度掩码。取值为：CS5，CS6，CS7或CS8

	/*设置停止位*/
    switch(wBits)
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

	/*设置奇偶校验位*/
    switch( cEvent )
    {
        case 'O'://奇数
            newtio.c_cflag |= PARENB; //允许输出产生奇偶信息以及输入到奇偶校验
            newtio.c_cflag |= PARODD;  //输入和输出是奇及校验
            newtio.c_iflag |= (INPCK|ISTRIP); // INPACK:启用输入奇偶检测；ISTRIP：去掉第八位
            break;
        case 'E'://偶数
            newtio.c_iflag |= (INPCK|ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N'://无奇偶校验位
            newtio.c_cflag &= ~PARENB;
            break;
    }
	
	/*设置波特率*/
    switch( wSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        case 1500000:
            cfsetispeed(&newtio, B1500000);
            cfsetospeed(&newtio, B1500000);
            break;
        default:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
    }

	/*设置停止位*/

    if (wStop == 1)
        newtio.c_cflag &= ~CSTOPB; //CSTOPB:设置两个停止位，而不是一个
    else if (wStop == 2)
        newtio.c_cflag |= CSTOPB;

	/*设置等待时间和最小接收字符*/ 
    newtio.c_cc[VTIME] = 0; //VTIME:非cannoical模式读时的延时，以十分之一秒位单位
    newtio.c_cc[VMIN] = 0; //VMIN:非canonical模式读到最小字符数

	/*处理未接收字符*/
	tcflush(fd,TCIFLUSH); // 改变在所有写入 fd 引用的对象的输出都被传输后生效，所有已接受但未读入的输入都在改变发生前丢弃。
    if ((tcsetattr(fd,TCSANOW,&newtio))!=0) //TCSANOW:改变立即发生
    {
        //bv_logw("com set error");
        return -1;
    }
    //bv_logi("set done!\n\r");
    return 0;
}


/******************************************************************************
*函数名称:bv_uart_open
*功能描述: 打开串口，并配置串口参数
*全局影响:无
*输入:int uart_port-串口端口号，int baud_rate-波特率
*输出:无
*返回值:-1：失败；>0 打开串口的描述符id
*作者:窦广正
*修改记录:
*2017-7-6 窦广正　新建
*
*
******************************************************************************/

int bv_uart_open(int uart_port,int baud_rate)
{    
	char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4","/dev/ttyUSB0"};
	int wRlt = 0;
	int fd = -1;


    fd = open(dev[uart_port], O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    if (fd < 0)
    {
        printf("uart %d open err!",uart_port);
        return -1;
    }

    //判断串口的状态是否为阻塞状态                            
	if (fcntl(fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return -1;
	}     
	else
	{
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	}

	if (isatty(STDIN_FILENO)==0)
	{
        printf("standard input is not a terminal device.\n");
		return -1;
	}
	else
	{
        printf("isatty success!\n");
	}

    printf("fd-open=%d\n",fd);
	gawFdUart[uart_port] = fd;
	wRlt = set_opt(fd, baud_rate, UART_DATAT_BIT_8, 'N', UART_STOP_BIT_2);
	if (wRlt < 0)
    {
        printf("set_opt  err!\n");
        return -1;
    }
	
	return fd;
}

/******************************************************************************
*函数名称:bv_uart_close
*功能描述: 关闭串口
*全局影响:无
*输入:int uart_port-串口端口号，
*输出:无
*返回值:无
*作者:窦广正
*修改记录:
*2017-7-6 窦广正　新建
*
*
******************************************************************************/
void bv_uart_close(int uart_port)
{
	 int fd=-1;
	 
	 fd = gawFdUart[uart_port];
	 if (fd > 0) 
	 {
		 close(fd);
	 }

}


/******************************************************************************
*函数名称:bv_uart_write
*功能描述: 向串口发送数据
*全局影响:无
*输入:int uart_port-发送数据的串口端口号，
	  int data_len -发送数据大小

*输出:char* p_data_buf-发送数据缓冲区地址
*返回值:int len: -1 发送数据错误；>0 串口发送数据字节数
*作者:窦广正
*修改记录:
*2017-7-6 窦广正　新建
*
*
******************************************************************************/
int bv_uart_write(int uart_port, unsigned char *p_data_buf, int data_len)
{
	int len;
	int fd=-1;
	 
	fd = gawFdUart[uart_port];
	len = write(fd, p_data_buf, data_len);
	if (len == data_len)
	{
		return len;
	}     
	else   
	{
		tcflush(fd,TCOFLUSH);
		printf("uart write err.\n");
		return -1;
	}
}


/******************************************************************************
*函数名称:bv_uart_read
*功能描述: 从串口中读取数据
*全局影响:无
*输入:int uart_port-读取数据的串口端口号，
	  int buf_size -读取数据缓冲区大小

*输出:char* p_data_buf-读取数据缓冲区地址
*返回值:int recvlen：<=0 读取数据错误；>0 串口接收数据字节数
*作者:窦广正
*修改记录:
*2017-7-6 窦广正　新建
*
*
******************************************************************************/
int bv_uart_read(int uart_port, char* p_data_buf,int buf_size)
{
	int fd=-1;
	int len,fs_sel;
	int recvlen;
    fd_set fs_read;
    struct timeval time;
    char * pbuf;


	pbuf = p_data_buf;
	fd = gawFdUart[uart_port];
	printf("fd %d\n",fd);

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 0;
    time.tv_usec = 100000;
	recvlen = 0;

    /*使用select实现串口的多路通信*/
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
	if (fs_sel)
	{

	    while(1)
	    {
			len = read(fd, pbuf, buf_size);
			if (len > 0)
	        {
				//printf("%s\n", pbuf);
	            recvlen += len;
				pbuf += len;
	        }
	        else 
	        {
	            break;
	    	}
			if (len >= buf_size) break;
	    }
	}
	else
	{
		printf("uart read time out\n.");
		recvlen = -1;
	}
	
	return recvlen;

}

/******************************************************************************
*函数名称:bv_uart_thread
*功能描述: 串口数据处理线程
*全局影响:
*输入:无
*输出:无
*返回值:空指针
*作者:窦广正
*修改记录:
*2017-7-6 窦广正　新建
*
*
******************************************************************************/

void* bv_uart_thread(void *data)
{
//	char buff[128];
//	int fd=-1;
//	int len;
//	char file_name[64];
//	FILE *fp;
//	int rtn;
//
//
//	/*设置线程名称*/
//	prctl(PR_SET_NAME, "bv_uart_thread");
//
//    fp = fopen(file_name, "wt+");
//	if (!fp)
//	{
//		printf("fopen uart_idx_%d.log  err!");
//	}
//
//	/*init uart*/
//	rtn = bv_uart_open(serial_port,115200);
//	if (rtn > 0)
//	{
//		gawFdUart[serial_port] = rtn;
//		printf("uart%d opened[%d].\n",serial_port,rtn);
//		fprintf(fp, "uart%d opened[%d].\n",serial_port,rtn);
//	}
//	else
//	{
//		printf("uart%d opened[%d] false.\n",serial_port,rtn);
//		fprintf(fp, "uart%d opened[%d] false.\n",serial_port,rtn);
//	}
//
//	fd = gawFdUart[serial_port];
//	if (fd < 0)
//	{
//		fprintf(fp, "serial_port %d not inited.\r\n", fd);
//	}
//
//    while (fd < 0)
//    {
//		printf( "waiting uart%d init ... \n",serial_port);
//		bv_seconds_sleep(1);
//		fd = gawFdUart[serial_port];
//	}
//
//
//
//    while (!m_exit_thread)
//    {
//        memset(buff, 0, sizeof(buff));
//
//		len = bv_uart_read(serial_port, buff, sizeof(buff));
//
//		#if 0
//		bv_logi("get uart%d %d(Bytes) data: \n",serial_port,len);
//		now = bv_get_sys_time_usec();
//		printf("get uart%d %d(Bytes) data [%llu(us)]: \n",serial_port,len,now-pre);
//		pre = now;
//		#endif
//		if (len > 0 && fp)
//		{
//		#if 0
//            for (i = 0; i < len; i++)
//			{
//                fprintf(fp, " %02x", buff[i]);
//				printf( "%02x ", buff[i]);
//            }
//			printf("\r\n");
//			fprintf(fp, "\r\n");
//		#endif
//			serial_read_callback(buff,len);
//        }
//
//    }

    return (void*)0;


}



