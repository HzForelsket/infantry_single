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

	uint8_t baseRFID;//��������
	uint8_t highlandRFID;//�ߵ�����
	uint8_t energyRFID;//������������
	uint8_t feipoRFID;//��������
	uint8_t outpostRFID;//ǰ��վ����
	uint8_t resourseRFID;//��Դ������
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

		//��ϸ�����뷭�ġ�RoboMaster 2021 ����ϵͳ����Э�鸽¼ V1.0��20210203����
		//����Ƶ�ʣ�1Hz
		struct {
			uint8_t game_type : 4;//��������
			uint8_t game_progress : 4;//��ǰ�����׶�
			uint16_t stage_remain_time;//��ǰ�׶�ʣ��ʱ��
			uint64_t SyncTimeStamp;
		} ext_game_status_t;

		//��������ʱ����
		struct {
			uint8_t winner;//0��ƽ�֣�1���췽ʤ����2������ʤ��
		} ext_game_result_t;

		//����Ƶ�ʣ�1Hz
		struct {
			uint16_t red_1_robot_HP;//��1Ӣ�ۻ�����Ѫ��
			uint16_t red_2_robot_HP;//��2���̻�����Ѫ��
			uint16_t red_3_robot_HP;//��3����������Ѫ��
			uint16_t red_4_robot_HP;//��4����������Ѫ��
			uint16_t red_5_robot_HP;//��5����������Ѫ��
			uint16_t red_7_robot_HP;//��7�ڱ�������Ѫ��
			uint16_t red_outpost_HP;//�췽ǰ��վѪ��
			uint16_t red_base_HP;//�췽����Ѫ��
			uint16_t blue_1_robot_HP;//��1Ӣ�ۻ�����Ѫ��
			uint16_t blue_2_robot_HP;//��2���̻�����Ѫ��
			uint16_t blue_3_robot_HP;//��3����������Ѫ��
			uint16_t blue_4_robot_HP;//��4����������Ѫ��
			uint16_t blue_5_robot_HP;//��5����������Ѫ��
			uint16_t blue_7_robot_HP;//��7�ڱ�������Ѫ��
			uint16_t blue_outpost_HP;//����ǰ��վѪ��
			uint16_t blue_base_HP;//��������Ѫ��
		}  ext_game_robot_HP_t;

		//����Ƶ�ʣ����ڷ������
		struct {
			uint8_t dart_belong;//������ڵĶ��飻1���췽���ڣ�2����������
			uint16_t stage_remaining_time;//����ʱʣ�����ʱ��
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

		//�����¼����ݣ�0x0101������Ƶ�ʣ��¼��ı����
		struct {
			uint32_t event_type;
		} ext_event_data_t;

		//����վ������ʶ��0x0102������Ƶ�ʣ������ı����, ���ͷ�Χ������������
		struct {
			uint8_t supply_projectile_id;
			uint8_t supply_robot_id;
			uint8_t supply_projectile_step;
			uint8_t supply_projectile_num;
		} ext_supply_projectile_action_t;

		//���о�����Ϣ��cmd_id(0x0104)������Ƶ�ʣ����淢������
		struct {
			uint8_t level;
			uint8_t foul_robot_id;
		} ext_referee_warning_t;

		//���ڷ���ڵ���ʱ��cmd_id(0x0105)������Ƶ�ʣ�1Hz ���ڷ��ͣ����ͷ�Χ������������
		struct {
			uint8_t dart_remaining_time;
		}ext_dart_remaining_time_t;

		//����������״̬��0x0201������Ƶ�ʣ�10Hz
		struct {
			uint8_t robot_id;//������id
			uint8_t robot_level;//�����˵ȼ�
			uint16_t remain_HP;//������ʣ��Ѫ��
			uint16_t max_HP;//����������Ѫ��
			uint16_t shooter_id1_17mm_cooling_rate;//������1��17mmǹ��ÿ����ȴֵ
			uint16_t shooter_id1_17mm_cooling_limit;//������1��17mmǹ����������
			uint16_t shooter_id1_17mm_speed_limit;//������1��17mmǹ�������ٶ�
			uint16_t shooter_id2_17mm_cooling_rate;//������2��17mmǹ��ÿ����ȴֵ
			uint16_t shooter_id2_17mm_cooling_limit;//������2��17mmǹ����������
			uint16_t shooter_id2_17mm_speed_limit;//������2��17mmǹ�������ٶ�
			uint16_t shooter_id1_42mm_cooling_rate;//������42mmǹ��ÿ����ȴֵ
			uint16_t shooter_id1_42mm_cooling_limit;//������42mmǹ����������
			uint16_t shooter_id1_42mm_speed_limit;//������42mmǹ�������ٶ�
			uint16_t chassis_power_limit;//�����˵��̹�������
			uint8_t mains_power_gimbal_output : 1;//gimbal���Ƿ������
			uint8_t mains_power_chassis_output : 1;//chassis���Ƿ������
			uint8_t mains_power_shooter_output : 1;//shooter���Ƿ������
		}ext_game_robot_status_t;

		//ʵʱ�����������ݣ�0x0202������Ƶ�ʣ�50Hz
		struct {
			uint16_t chassis_volt;//���������ѹ ��λ����
			uint16_t chassis_current;//����������� ��λ����
			float chassis_power;//����������� ��λ��
			uint16_t chassis_power_buffer;//���̹��ʻ���
			uint16_t shooter_id1_17mm_cooling_heat;//1��17mmǹ������
			uint16_t shooter_id2_17mm_cooling_heat;//2��17mmǹ������
			uint16_t shooter_id1_42mm_cooling_heat;//42mmǹ������
		}ext_power_heat_data_t;

		//������λ�ã�0x0203������Ƶ�ʣ�10Hz
		struct {
			float x;//λ��x����
			float y;//λ��y����
			float z;//λ��z����
			float yaw;//λ��ǹ��
		}ext_game_robot_pos_t;

		//���������棺0x0204������Ƶ�ʣ�1Hz
		struct {
			uint8_t power_rune_buff;
		}ext_buff_t;

		//���л���������״̬��0x0205������Ƶ�ʣ�10Hz
		struct {
			uint8_t attack_time;
		}aerial_robot_energy_t;

		//�˺�״̬��0x0206������Ƶ�ʣ��˺���������
		struct {
			uint8_t armor_id : 4;
			uint8_t hurt_type : 4;
		}ext_robot_hurt_t;

		//ʵʱ�����Ϣ��0x0207������Ƶ�ʣ��������
		struct {
			uint8_t bullet_type;
			uint8_t shooter_id;
			uint8_t bullet_freq;//�ӵ���Ƶ ��λHz
			float bullet_speed;//�ӵ����� ��λm/s
		}ext_shoot_data_t;

		//�ӵ�ʣ�෢������0x0208������Ƶ�ʣ�10Hz ���ڷ��ͣ����л����˷���
		struct {
			uint16_t bullet_remaining_num_17mm;
			uint16_t bullet_remaining_num_42mm;
			uint16_t coin_remaining_num;
		} ext_bullet_remaining_t;

		//������ RFID ״̬��0x0209������Ƶ�ʣ�1Hz�����ͷ�Χ����һ������
		struct {
			uint32_t rfid_status;

		} ext_rfid_status_t;

		//���ڻ����˿ͻ���ָ�����ݣ�0x020A������Ƶ�ʣ�10Hz�����ͷ�Χ����һ������
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
		uint8_t  sof;//����֡��ʼ�ֽڣ��̶�ֵΪ0x05
		uint16_t data_length; //������data�ĳ���
		uint8_t  seq;  //��֡ͷ
		uint8_t  crc8;  //֡ͷCRC8У��
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
