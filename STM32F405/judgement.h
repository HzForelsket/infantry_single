#pragma once
#include "usart.h"
#include "CRC.h"
#include "tim.h"
#include "string.h"


#define BUFSIZE 100
#define DMA_RX_SIZE 100
#define DMA_TX_SIZE 150
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)    ) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

class Judgement
{
public:
	bool ready = false;
	bool powerheatready = false;
	bool judgementready = false;
	bool gragh_init = true;
	float prebulletspd = 0;
	uint32_t id_blue = 256;
	uint32_t id_red = 256;

	uint8_t baseRFID;//基地增益
	uint8_t highlandRFID;//高地增益
	uint8_t energyRFID;//能量机关增益
	uint8_t feipoRFID;//飞坡增益
	uint8_t outpostRFID;//前哨站增益
	uint8_t resourseRFID;//资源岛增益
	int32_t nBullet = 0;
	int32_t court = 0;

	Judgement(void)
	{
		m_uarttx[0] = 0xA5;
		m_uarttx[1] = 0x0D;
		m_uarttx[2] = 0x00;
		m_uarttx[3] = 0x00;
		AppendCRC8CheckSum(m_uarttx, 5);
		m_uarttx[5] = 0x00;
		m_uarttx[6] = 0x01;
	}	
	void Init(UART* huart, uint32_t baud, USART_TypeDef* uart_base);
	void GetData(void);
	void SendData(void);


	struct {
		uint16_t CmdID;

		//详细描述请翻阅《RoboMaster 2021 裁判系统串口协议附录 V1.0（20210203）》
		//发送频率：1Hz
		struct {
			uint8_t game_type : 4;//比赛类型
			uint8_t game_progress : 4;//当前比赛阶段
			uint16_t stage_remain_time;//当前阶段剩余时间
			uint64_t SyncTimeStamp;
		} ext_game_status_t;

		//比赛结束时发送
		struct {
			uint8_t winner;//0：平局，1：红方胜利，2：蓝方胜利
		} ext_game_result_t;

		//发送频率：1Hz
		struct {
			uint16_t red_1_robot_HP;//红1英雄机器人血量
			uint16_t red_2_robot_HP;//红2工程机器人血量
			uint16_t red_3_robot_HP;//红3步兵机器人血量
			uint16_t red_4_robot_HP;//红4步兵机器人血量
			uint16_t red_5_robot_HP;//红5步兵机器人血量
			uint16_t red_7_robot_HP;//红7哨兵机器人血量
			uint16_t red_outpost_HP;//红方前哨站血量
			uint16_t red_base_HP;//红方基地血量
			uint16_t blue_1_robot_HP;//蓝1英雄机器人血量
			uint16_t blue_2_robot_HP;//蓝2工程机器人血量
			uint16_t blue_3_robot_HP;//蓝3步兵机器人血量
			uint16_t blue_4_robot_HP;//蓝4步兵机器人血量
			uint16_t blue_5_robot_HP;//蓝5步兵机器人血量
			uint16_t blue_7_robot_HP;//蓝7哨兵机器人血量
			uint16_t blue_outpost_HP;//蓝方前哨站血量
			uint16_t blue_base_HP;//蓝方基地血量
		}  ext_game_robot_HP_t;

		//发送频率：飞镖发射后发送
		struct {
			uint8_t dart_belong;//发射飞镖的队伍；1：红方飞镖，2：蓝方飞镖
			uint16_t stage_remaining_time;//发射时剩余比赛时间
		} ext_dart_status_t;

		struct {
			uint8_t F1_zone_status : 1;
			uint8_t F1_zone_buff_debuff_status : 3;
			uint8_t F2_zone_status : 1;
			uint8_t F2_zone_buff_debuff_status : 3;
			uint8_t F3_zone_status : 1;
			uint8_t F3_zone_buff_debuff_status : 3;
			uint8_t F4_zone_status : 1;
			uint8_t F4_zone_buff_debuff_status : 3;
			uint8_t F5_zone_status : 1;
			uint8_t F5_zone_buff_debuff_status : 3;
			uint8_t F6_zone_status : 1;
			uint8_t F6_zone_buff_debuff_status : 3;
			uint16_t red1_bullet_left;
			uint16_t red2_bullet_left;
			uint16_t blue1_bullet_left;
			uint16_t blue2_bullet_left;
		} ext_ICRA_buff_debuff_zone_status_t;

		//场地事件数据：0x0101。发送频率：事件改变后发送
		struct {
			uint32_t event_type;
		} ext_event_data_t;

		//补给站动作标识：0x0102。发送频率：动作改变后发送, 发送范围：己方机器人
		struct {
			uint8_t supply_projectile_id;
			uint8_t supply_robot_id;
			uint8_t supply_projectile_step;
			uint8_t supply_projectile_num;
		} ext_supply_projectile_action_t;

		//裁判警告信息：cmd_id(0x0104)。发送频率：警告发生后发送
		struct {
			uint8_t level;
			uint8_t foul_robot_id;
		} ext_referee_warning_t;

		//飞镖发射口倒计时：cmd_id(0x0105)。发送频率：1Hz 周期发送，发送范围：己方机器人
		struct {
			uint8_t dart_remaining_time;
		}ext_dart_remaining_time_t;

		//比赛机器人状态：0x0201。发送频率：10Hz
		struct {
			uint8_t robot_id;//机器人id
			uint8_t robot_level;//机器人等级
			uint16_t remain_HP;//机器人剩余血量
			uint16_t max_HP;//机器人上限血量
			uint16_t shooter_id1_17mm_cooling_rate;//机器人1号17mm枪口每秒冷却值
			uint16_t shooter_id1_17mm_cooling_limit;//机器人1号17mm枪口热量上限
			uint16_t shooter_id1_17mm_speed_limit;//机器人1号17mm枪口上限速度
			uint16_t shooter_id2_17mm_cooling_rate;//机器人2号17mm枪口每秒冷却值
			uint16_t shooter_id2_17mm_cooling_limit;//机器人2号17mm枪口热量上限
			uint16_t shooter_id2_17mm_speed_limit;//机器人2号17mm枪口上限速度
			uint16_t shooter_id1_42mm_cooling_rate;//机器人42mm枪口每秒冷却值
			uint16_t shooter_id1_42mm_cooling_limit;//机器人42mm枪口热量上限
			uint16_t shooter_id1_42mm_speed_limit;//机器人42mm枪口上限速度
			uint16_t chassis_power_limit;//机器人底盘功率上限
			uint8_t mains_power_gimbal_output : 1;//gimbal口是否有输出
			uint8_t mains_power_chassis_output : 1;//chassis口是否有输出
			uint8_t mains_power_shooter_output : 1;//shooter口是否有输出
		}ext_game_robot_status_t;

		//实时功率热量数据：0x0202。发送频率：50Hz
		struct {
			uint16_t chassis_volt;//底盘输出电压 单位毫伏
			uint16_t chassis_current;//底盘输出电流 单位毫安
			float chassis_power;//底盘输出功率 单位瓦
			uint16_t chassis_power_buffer;//底盘功率缓冲
			uint16_t shooter_id1_17mm_cooling_heat;//1号17mm枪口热量
			uint16_t shooter_id2_17mm_cooling_heat;//2号17mm枪口热量
			uint16_t shooter_id1_42mm_cooling_heat;//42mm枪口热量
		}ext_power_heat_data_t;

		//机器人位置：0x0203。发送频率：10Hz
		struct {
			float x;//位置x坐标
			float y;//位置y坐标
			float z;//位置z坐标
			float yaw;//位置枪口
		}ext_game_robot_pos_t;

		//机器人增益：0x0204。发送频率：1Hz
		struct {
			uint8_t power_rune_buff;
		}ext_buff_t;

		//空中机器人能量状态：0x0205。发送频率：10Hz
		struct {
			uint8_t attack_time;
		}aerial_robot_energy_t;

		//伤害状态：0x0206。发送频率：伤害发生后发送
		struct {
			uint8_t armor_id : 4;
			uint8_t hurt_type : 4;
		}ext_robot_hurt_t;

		//实时射击信息：0x0207。发送频率：射击后发送
		struct {
			uint8_t bullet_type;
			uint8_t shooter_id;
			uint8_t bullet_freq;//子弹射频 单位Hz
			float bullet_speed;//子弹射速 单位m/s
		}ext_shoot_data_t;

		//子弹剩余发射数：0x0208。发送频率：10Hz 周期发送，所有机器人发送
		struct {
			uint16_t bullet_remaining_num_17mm;
			uint16_t bullet_remaining_num_42mm;
			uint16_t coin_remaining_num;
		} ext_bullet_remaining_t;

		//机器人 RFID 状态：0x0209。发送频率：1Hz，发送范围：单一机器人
		struct {
			uint32_t rfid_status;

		} ext_rfid_status_t;

		//飞镖机器人客户端指令数据：0x020A。发送频率：10Hz，发送范围：单一机器人
		struct
		{
			uint8_t dart_launch_opening_status;
			uint8_t dart_attack_target;
			uint16_t target_change_time;
			uint16_t operate_launch_cmd_time;
		} ext_dart_client_cmd_t;




	}data;

#pragma pack(1)
	//-----------------------------------------------------------
	typedef  __packed  struct {
		uint16_t data_cmd_id;
		uint16_t sender_ID;
		uint16_t receiver_ID;
	}ext_student_interactive_header_data_t;
	typedef  __packed  struct {
		uint8_t data[15];
	} robot_interactive_data_t;
	typedef __packed struct
	{
		uint8_t graphic_name[3];
		uint32_t operate_tpye : 3;
		uint32_t graphic_tpye : 3;
		uint32_t layer : 4;
		uint32_t color : 4;
		uint32_t start_angle : 9;
		uint32_t end_angle : 9;
		uint32_t width : 10;
		uint32_t start_x : 11;
		uint32_t start_y : 11;
		uint32_t radius : 10;
		uint32_t end_x : 11;
		uint32_t end_y : 11;
	} graphic_data_struct_t;
	typedef __packed  struct
	{
		graphic_data_struct_t grapic_data_struct;
	} ext_client_custom_graphic_single_t;
	typedef __packed struct
	{
		graphic_data_struct_t grapic_data_struct[5];
	} ext_client_custom_graphic_five_t;
	typedef __packed struct
	{
		graphic_data_struct_t grapic_data_struct[7];
	} ext_client_custom_graphic_seven_t;
	typedef __packed struct
	{
		graphic_data_struct_t grapic_data_struct;
		uint8_t data[30];
	} ext_client_custom_character_t;
	//-----------------------------------------------------------

	typedef __packed  struct
	{
		uint8_t  sof;//数据帧起始字节，固定值为0x05
		uint16_t data_length; //数据中data的长度
		uint8_t  seq;  //包帧头
		uint8_t  crc8;  //帧头CRC8校验
	} frame_header_t;
	typedef __packed  struct
	{
		frame_header_t   							txFrameHeader;
		uint16_t								CmdID;
		ext_student_interactive_header_data_t   dataFrameHeader;
		ext_client_custom_graphic_seven_t	 	interactData;
		uint16_t		 						FrameTail;
	}ext_CommunatianData_graphic_seven_t;
	typedef __packed  struct
	{
		frame_header_t   							txFrameHeader;
		uint16_t								CmdID;
		ext_student_interactive_header_data_t   dataFrameHeader;
		ext_client_custom_character_t 	        interactData;
		uint16_t		 						FrameTail;
	}ext_CommunatianData_client_custom_character_t;

	ext_CommunatianData_graphic_seven_t  CommunatianData;
	ext_CommunatianData_client_custom_character_t CommunatianData_top;
	ext_CommunatianData_client_custom_character_t CommunatianData_aim;
	ext_CommunatianData_client_custom_character_t CommunatianData_x;
	ext_CommunatianData_client_custom_character_t CommunatianData_lock;
	ext_CommunatianData_client_custom_character_t CommunatianData_launch;
#pragma pack()


private:
	uint8_t m_uartrx[DMA_RX_SIZE] = { 0 };
	uint8_t m_uarttx[DMA_TX_SIZE] = { 0 };
	uint8_t m_frame[DMA_RX_SIZE] = { 0 };
	uint8_t m_FIFO[BUFSIZE] = { 0 };
	uint8_t* m_whand = m_FIFO;
	uint8_t* m_rhand = m_FIFO;
	uint32_t m_readnum = 0;
	uint32_t m_leftsize = 0;

//	uint8_t* GetDMARx(void) { return m_uartrx; }
	void OnIRQHandler(uint16_t readnum);
	UART* m_uart;

	union _4bytefloat
	{
		uint8_t b[4];
		float f;
	};
	float u32_to_float(uint8_t* chReceive)
	{
		union _4bytefloat x;
		memcpy(x.b, chReceive, sizeof(float));
		return x.f;
	}
	bool transmit(uint32_t read_size, uint8_t* plate);
	void Decode(uint8_t* m_frame);
	void Encode(void);

	void Gragh_Top(void);
	void Gragh_Aim(void);
	void Gragh_X(void);
	void Gragh_Lock(void);
	void Gragh_Launch(void);
	void Gragh_Line(void);
};

extern "C" Judgement judgement;
//extern "C" void JudgementGetData(void* thisObj);
//extern "C" void JudgementOnIRQHandler(void* thisObj);
