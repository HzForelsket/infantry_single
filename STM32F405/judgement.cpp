#include "judgement.h"
#include "NUC.h"
#include "RC.h"
#include "control.h"

void Judgement::OnIRQHandler(const uint16_t readnum)
{
	m_readnum = readnum;
	if ((m_whand + m_readnum) < (m_FIFO + BUFSIZE))
	{
		memcpy(m_whand, m_uartrx, m_readnum);
		m_whand = m_whand + m_readnum;
	}
	else if ((m_whand + m_readnum) == (m_FIFO + BUFSIZE))
	{
		memcpy(m_whand, m_uartrx, m_readnum);
		m_whand = m_FIFO;
	}
	else
	{
		const uint8_t left_size = m_FIFO + BUFSIZE - m_whand;
		memcpy(m_whand, m_uartrx, left_size);
		m_whand = m_FIFO;
		memcpy(m_whand, m_uartrx + left_size, m_readnum - left_size);
		m_whand = m_FIFO + m_readnum - left_size;
	}
	m_leftsize = m_leftsize + m_readnum;

}

void Judgement::Init(UART* huart, uint32_t baud, USART_TypeDef* uart_base)
{
	huart->Init(uart_base, baud).DMARxInit(m_uartrx);
	int addr = function(0, this);
	huart->huart.pthis = reinterpret_cast<void*>(addr);
	addr = function(0, &Judgement::OnIRQHandler);
	huart->huart.callback = reinterpret_cast<void(*)(void*, uint16_t)>(addr);
	m_uart = huart;
}

void Judgement::GetData(void)
{
	if (transmit(5, m_frame) == 0)return;
	while (m_frame[0] != 0xA5)
	{
		memcpy(m_frame, m_frame + 1, 4);
		if (transmit(1, m_frame + 4) == 0)return;
	}
	if (VerifyCRC8CheckSum(m_frame, 5))
	{
		const uint16_t datalength = static_cast<uint16_t>((m_frame[1]) | (m_frame[2] << 8));
		if (transmit(datalength + 4, m_frame + 5) == 0)return;
		if (VerifyCRC16CheckSum(m_frame, (datalength + 9)))Decode(m_frame);
	}
}
void Judgement::Decode(uint8_t* m_frame)//未完全
{
	const uint16_t cmdID = static_cast<uint16_t>(m_frame[5] | m_frame[6] << 8);
	uint8_t* rawdata = &m_frame[7];
	switch (cmdID)
	{
	case 0x0001:
		data.ext_game_status_t.game_type = static_cast<uint8_t>(rawdata[0] & 0x0F);
		data.ext_game_status_t.game_progress = static_cast<uint8_t>(rawdata[0] >> 4);
		data.ext_game_status_t.stage_remain_time = static_cast<uint16_t>(rawdata[1] | rawdata[2] << 8);
		break;
		//data.ext_game_status_t.SyncTimeStamp

	case 0x0002:
		data.ext_game_result_t.winner = rawdata[0];
		break;

	case 0x0003:
		data.ext_game_robot_HP_t.red_1_robot_HP = static_cast<uint16_t>(rawdata[0] | rawdata[1] << 8);
		data.ext_game_robot_HP_t.red_2_robot_HP = static_cast<uint16_t>(rawdata[2] | rawdata[3] << 8);
		data.ext_game_robot_HP_t.red_3_robot_HP = static_cast<uint16_t>(rawdata[4] | rawdata[5] << 8);
		data.ext_game_robot_HP_t.red_4_robot_HP = static_cast<uint16_t>(rawdata[6] | rawdata[7] << 8);
		data.ext_game_robot_HP_t.red_5_robot_HP = static_cast<uint16_t>(rawdata[8] | rawdata[9] << 8);
		data.ext_game_robot_HP_t.red_7_robot_HP = static_cast<uint16_t>(rawdata[10] | rawdata[11] << 8);
		data.ext_game_robot_HP_t.red_outpost_HP = static_cast<uint16_t>(rawdata[12] | rawdata[13] << 8);
		data.ext_game_robot_HP_t.red_base_HP = static_cast<uint16_t>(rawdata[14] | rawdata[15] << 8);
		data.ext_game_robot_HP_t.blue_1_robot_HP = static_cast<uint16_t>(rawdata[16] | rawdata[17] << 8);
		data.ext_game_robot_HP_t.blue_2_robot_HP = static_cast<uint16_t>(rawdata[18] | rawdata[19] << 8);
		data.ext_game_robot_HP_t.blue_3_robot_HP = static_cast<uint16_t>(rawdata[20] | rawdata[21] << 8);
		data.ext_game_robot_HP_t.blue_4_robot_HP = static_cast<uint16_t>(rawdata[22] | rawdata[23] << 8);
		data.ext_game_robot_HP_t.blue_5_robot_HP = static_cast<uint16_t>(rawdata[24] | rawdata[25] << 8);
		data.ext_game_robot_HP_t.blue_7_robot_HP = static_cast<uint16_t>(rawdata[26] | rawdata[27] << 8);
		data.ext_game_robot_HP_t.blue_outpost_HP = static_cast<uint16_t>(rawdata[28] | rawdata[29] << 8);
		data.ext_game_robot_HP_t.blue_base_HP = static_cast<uint16_t>(rawdata[30] | rawdata[31] << 8);
		break;

	case 0x0004:
		data.ext_dart_status_t.dart_belong = rawdata[0];
		data.ext_dart_status_t.stage_remaining_time = static_cast<uint16_t>(rawdata[1] | rawdata[2] << 8);
		break;

		//case 0x0005:

		//case id_red

	case 0x0102:
		data.ext_supply_projectile_action_t.supply_projectile_id = rawdata[0];
		data.ext_supply_projectile_action_t.supply_robot_id = rawdata[1];
		data.ext_supply_projectile_action_t.supply_projectile_step = rawdata[2];
		data.ext_supply_projectile_action_t.supply_projectile_num = rawdata[3];


	case 0x0104:
		data.ext_referee_warning_t.level = rawdata[0];
		data.ext_referee_warning_t.foul_robot_id = rawdata[1];
		break;

	case 0x0105:
		data.ext_dart_remaining_time_t.dart_remaining_time = rawdata[0];
		break;

	case 0x0201:
		data.ext_game_robot_status_t.robot_id = rawdata[0];
		judgementready = true;
		data.ext_game_robot_status_t.robot_level = rawdata[1];
		data.ext_game_robot_status_t.remain_HP = static_cast<uint16_t>(rawdata[2] | rawdata[3] << 8);
		data.ext_game_robot_status_t.max_HP = static_cast<uint16_t>(rawdata[4] | rawdata[5] << 8);
		data.ext_game_robot_status_t.shooter_id1_17mm_cooling_rate = static_cast<uint16_t>(rawdata[6] | rawdata[7] << 8);
		data.ext_game_robot_status_t.shooter_id1_17mm_cooling_limit = static_cast<uint16_t>(rawdata[8] | rawdata[9] << 8);
		data.ext_game_robot_status_t.shooter_id1_17mm_speed_limit = static_cast<uint16_t>(rawdata[10] | rawdata[11] << 8);
		data.ext_game_robot_status_t.shooter_id2_17mm_cooling_rate = static_cast<uint16_t>(rawdata[12] | rawdata[13] << 8);
		data.ext_game_robot_status_t.shooter_id2_17mm_cooling_limit = static_cast<uint16_t>(rawdata[14] | rawdata[15] << 8);
		data.ext_game_robot_status_t.shooter_id2_17mm_speed_limit = static_cast<uint16_t>(rawdata[16] | rawdata[17] << 8);
		data.ext_game_robot_status_t.shooter_id1_42mm_cooling_rate = static_cast<uint16_t>(rawdata[18] | rawdata[19] << 8);
		data.ext_game_robot_status_t.shooter_id1_42mm_cooling_limit = static_cast<uint16_t>(rawdata[20] | rawdata[21] << 8);
		data.ext_game_robot_status_t.shooter_id1_42mm_speed_limit = static_cast<uint16_t>(rawdata[22] | rawdata[23] << 8);
		data.ext_game_robot_status_t.chassis_power_limit = static_cast<uint16_t>(rawdata[24] | rawdata[25] << 8);
		data.ext_game_robot_status_t.mains_power_gimbal_output = static_cast<uint8_t>(rawdata[26] & 0x01);
		data.ext_game_robot_status_t.mains_power_chassis_output = static_cast<uint8_t>(rawdata[26] & 0x02);
		data.ext_game_robot_status_t.mains_power_shooter_output = static_cast<uint8_t>(rawdata[26] & 0x04);
		break;

	case 0x0202:
		powerheatready = true;
		data.ext_power_heat_data_t.chassis_volt = static_cast<uint16_t>(rawdata[0] | rawdata[1] << 8);
		data.ext_power_heat_data_t.chassis_current = static_cast<uint16_t>(rawdata[2] | rawdata[3] << 8);
		data.ext_power_heat_data_t.chassis_power = u32_to_float(&rawdata[4]);
		data.ext_power_heat_data_t.chassis_power_buffer = static_cast<uint16_t>(rawdata[8] | rawdata[9] << 8);
		data.ext_power_heat_data_t.shooter_id1_17mm_cooling_heat = static_cast<uint16_t>(rawdata[10] | rawdata[11] << 8);
		data.ext_power_heat_data_t.shooter_id2_17mm_cooling_heat = static_cast<uint16_t>(rawdata[12] | rawdata[13] << 8);
		data.ext_power_heat_data_t.shooter_id1_42mm_cooling_heat = static_cast<uint16_t>(rawdata[14] | rawdata[15] << 8);
		break;

	case 0x0203:
		data.ext_game_robot_pos_t.x = u32_to_float(&rawdata[0]);
		data.ext_game_robot_pos_t.y = u32_to_float(&rawdata[4]);
		data.ext_game_robot_pos_t.z = u32_to_float(&rawdata[8]);
		data.ext_game_robot_pos_t.yaw = u32_to_float(&rawdata[12]);
		break;

	case 0x0204:
		data.ext_buff_t.power_rune_buff = rawdata[0];
		break;

	case 0x0205:
		data.aerial_robot_energy_t.attack_time = rawdata[0];
		break;

	case 0x0206:
		data.ext_robot_hurt_t.armor_id = static_cast<uint8_t>(rawdata[0] & 0x0F);
		data.ext_robot_hurt_t.hurt_type = static_cast<uint8_t>(rawdata[0] >> 4);
		break;

	case 0x0207:
		data.ext_shoot_data_t.bullet_type = rawdata[0];
		data.ext_shoot_data_t.shooter_id = rawdata[1];
		data.ext_shoot_data_t.bullet_freq = rawdata[2];
		data.ext_shoot_data_t.bullet_speed = u32_to_float(&rawdata[3]);
		if (prebulletspd != data.ext_shoot_data_t.bullet_speed)
		{
			nBullet++;
			prebulletspd = data.ext_shoot_data_t.bullet_speed;
		}
		break;

	case 0x0208:
		data.ext_bullet_remaining_t.bullet_remaining_num_17mm = static_cast<uint16_t>(rawdata[0] | rawdata[1] << 8);
		data.ext_bullet_remaining_t.bullet_remaining_num_42mm = static_cast<uint16_t>(rawdata[2] | rawdata[3] << 8);
		data.ext_bullet_remaining_t.coin_remaining_num = static_cast <uint16_t>(rawdata[4] | rawdata[5] << 8);
		break;

	case 0x0209:
		data.ext_rfid_status_t.rfid_status = u32_to_float(&rawdata[0]);
		baseRFID = static_cast<uint8_t>(data.ext_rfid_status_t.rfid_status & 0x01);
		highlandRFID = static_cast<uint8_t>(data.ext_rfid_status_t.rfid_status & 0x02);
		energyRFID = static_cast<uint8_t>(data.ext_rfid_status_t.rfid_status & 0x04);
		feipoRFID = static_cast<uint8_t>(data.ext_rfid_status_t.rfid_status & 0x08);
		outpostRFID = static_cast<uint8_t>(data.ext_rfid_status_t.rfid_status & 0x10);
		resourseRFID = static_cast<uint8_t>(data.ext_rfid_status_t.rfid_status & 0x20);
		break;

	//case 0x020A:

	default:
		break;
	}
}
bool Judgement::transmit(uint32_t read_size, uint8_t* plate)
{
	if (m_leftsize < read_size)return false;

	if ((m_rhand + read_size) < (m_FIFO + BUFSIZE))
	{
		memcpy(plate, m_rhand, read_size);
		m_rhand = m_rhand + read_size;
	}
	else if ((m_rhand + read_size) == (m_FIFO + BUFSIZE))
	{
		memcpy(plate, m_rhand, read_size);
		m_rhand = m_FIFO;
	}
	else
	{
		const uint8_t left_size = m_FIFO + BUFSIZE - m_rhand;
		memcpy(plate, m_rhand, left_size);
		memcpy(plate + left_size, m_rhand = m_FIFO, read_size - left_size);
		m_rhand = m_FIFO + read_size - left_size;
	}
	m_leftsize = m_leftsize - read_size;
	return true;
}

void Judgement::Encode(void)//UI界面操作测试
{

	court++;
	if (court % 100 == 0) {
		if (judgementready)
		{
			Gragh_Top();
			Gragh_Aim();
			Gragh_X();
			Gragh_Lock();
			Gragh_Launch();
			Gragh_Line();
			gragh_init = false;
		}

	}

}


void Judgement::Gragh_Top(void)
{
	uint8_t datalength;
	memset(m_uarttx, 0, DMA_TX_SIZE);
	CommunatianData_top.txFrameHeader.sof = 0xA5;
	CommunatianData_top.txFrameHeader.data_length = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_character_t);
	CommunatianData_top.txFrameHeader.seq = 0;
	memcpy(m_uarttx, &CommunatianData_top.txFrameHeader, (sizeof(frame_header_t)));
	AppendCRC8CheckSum(m_uarttx, sizeof(frame_header_t));
	CommunatianData_top.CmdID = 0x0301;
	CommunatianData_top.dataFrameHeader.data_cmd_id = 0x0110;
	CommunatianData_top.dataFrameHeader.sender_ID = data.ext_game_robot_status_t.robot_id;

	if (data.ext_game_robot_status_t.robot_id > 10)
		CommunatianData_top.dataFrameHeader.receiver_ID = id_blue+ data.ext_game_robot_status_t.robot_id;
	else
		CommunatianData_top.dataFrameHeader.receiver_ID = id_red+ data.ext_game_robot_status_t.robot_id;
	CommunatianData_top.interactData.grapic_data_struct.graphic_name[0] = 2;
	CommunatianData_top.interactData.grapic_data_struct.graphic_name[1] = 0;
	CommunatianData_top.interactData.grapic_data_struct.graphic_name[2] = 0;
	if (gragh_init) {
		CommunatianData_top.interactData.grapic_data_struct.operate_tpye = 1;
	}
	else {
		CommunatianData_top.interactData.grapic_data_struct.operate_tpye = 2;
	}
	CommunatianData_top.interactData.grapic_data_struct.graphic_tpye = 7;
	CommunatianData_top.interactData.grapic_data_struct.layer = 1;
	if (rc.top_mode) {
		CommunatianData_top.interactData.grapic_data_struct.color = 0;
	}
	else
	{
		CommunatianData_top.interactData.grapic_data_struct.color = 8;
	}
	CommunatianData_top.interactData.grapic_data_struct.start_angle = 30;
	CommunatianData_top.interactData.grapic_data_struct.end_angle = 5;
	CommunatianData_top.interactData.grapic_data_struct.width = 3;
	CommunatianData_top.interactData.grapic_data_struct.start_x = 30;
	CommunatianData_top.interactData.grapic_data_struct.start_y = 800;
	/*
	CommunatianData_top.interactData.grapic_data_struct.radius = 0;
	CommunatianData_top.interactData.grapic_data_struct.end_x = 960;
	CommunatianData_top.interactData.grapic_data_struct.end_y = 680;
	*/
	memset(CommunatianData_top.interactData.data, 0, sizeof(CommunatianData_top.interactData.data));
	CommunatianData_top.interactData.data[0] = 'T';
	CommunatianData_top.interactData.data[1] = 'O';
	CommunatianData_top.interactData.data[2] = 'P';
	memcpy(m_uarttx + 5, (uint8_t *)&CommunatianData_top.CmdID, (sizeof(CommunatianData_top.CmdID) + sizeof(CommunatianData_top.dataFrameHeader) + sizeof(CommunatianData_top.interactData)));
	AppendCRC16CheckSum(m_uarttx, sizeof(CommunatianData_top));
	datalength = sizeof(CommunatianData_top);
	uart6.UARTTransmit(m_uarttx, datalength);
}

void Judgement::Gragh_Aim(void)
{
	uint8_t datalength;
	memset(m_uarttx, 0, DMA_TX_SIZE);
	CommunatianData_aim.txFrameHeader.sof = 0xA5;
	CommunatianData_aim.txFrameHeader.data_length = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_character_t);
	CommunatianData_aim.txFrameHeader.seq = 0;
	memcpy(m_uarttx, &CommunatianData_aim.txFrameHeader, (sizeof(frame_header_t)));
	AppendCRC8CheckSum(m_uarttx, sizeof(frame_header_t));
	CommunatianData_aim.CmdID = 0x0301;
	CommunatianData_aim.dataFrameHeader.data_cmd_id = 0x0110;
	CommunatianData_aim.dataFrameHeader.sender_ID = data.ext_game_robot_status_t.robot_id;

	if (data.ext_game_robot_status_t.robot_id > 10)
		CommunatianData_top.dataFrameHeader.receiver_ID = id_blue + data.ext_game_robot_status_t.robot_id;
	else
		CommunatianData_top.dataFrameHeader.receiver_ID = id_red + data.ext_game_robot_status_t.robot_id;
	CommunatianData_aim.interactData.grapic_data_struct.graphic_name[0] = 3;
	CommunatianData_aim.interactData.grapic_data_struct.graphic_name[1] = 0;
	CommunatianData_aim.interactData.grapic_data_struct.graphic_name[2] = 0;
	if (gragh_init) {
		CommunatianData_aim.interactData.grapic_data_struct.operate_tpye = 1;
	}
	else {
		CommunatianData_aim.interactData.grapic_data_struct.operate_tpye = 2;
	}
	CommunatianData_aim.interactData.grapic_data_struct.graphic_tpye = 7;
	CommunatianData_aim.interactData.grapic_data_struct.layer = 1;
	if (ctrl.pantile.aim) {
		CommunatianData_aim.interactData.grapic_data_struct.color = 0;
	}
	else
	{
		CommunatianData_aim.interactData.grapic_data_struct.color = 8;
	}
	CommunatianData_aim.interactData.grapic_data_struct.start_angle = 30;
	CommunatianData_aim.interactData.grapic_data_struct.end_angle = 5;
	CommunatianData_aim.interactData.grapic_data_struct.width = 3;
	CommunatianData_aim.interactData.grapic_data_struct.start_x = 30;
	CommunatianData_aim.interactData.grapic_data_struct.start_y = 740;
	/*
	CommunatianData_aim.interactData.grapic_data_struct.radius = 0;
	CommunatianData_aim.interactData.grapic_data_struct.end_x = 960;
	CommunatianData_aim.interactData.grapic_data_struct.end_y = 680;
	*/
	memset(CommunatianData_aim.interactData.data, 0, sizeof(CommunatianData_aim.interactData.data));
	CommunatianData_aim.interactData.data[0] = 'A';
	CommunatianData_aim.interactData.data[1] = 'I';
	CommunatianData_aim.interactData.data[2] = 'M';
	memcpy(m_uarttx + 5, (uint8_t *)&CommunatianData_aim.CmdID, (sizeof(CommunatianData_aim.CmdID) + sizeof(CommunatianData_aim.dataFrameHeader) + sizeof(CommunatianData_aim.interactData)));
	AppendCRC16CheckSum(m_uarttx, sizeof(CommunatianData_aim));
	datalength = sizeof(CommunatianData_aim);
	uart6.UARTTransmit(m_uarttx, datalength);
}

void Judgement::Gragh_X(void)
{
	uint8_t datalength;
	memset(m_uarttx, 0, DMA_TX_SIZE);
	CommunatianData_x.txFrameHeader.sof = 0xA5;
	CommunatianData_x.txFrameHeader.data_length = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_character_t);
	CommunatianData_x.txFrameHeader.seq = 0;
	memcpy(m_uarttx, &CommunatianData_x.txFrameHeader, (sizeof(frame_header_t)));
	AppendCRC8CheckSum(m_uarttx, sizeof(frame_header_t));
	CommunatianData_x.CmdID = 0x0301;
	CommunatianData_x.dataFrameHeader.data_cmd_id = 0x0110;
	CommunatianData_x.dataFrameHeader.sender_ID = data.ext_game_robot_status_t.robot_id;

	if (data.ext_game_robot_status_t.robot_id > 10)
		CommunatianData_top.dataFrameHeader.receiver_ID = id_blue + data.ext_game_robot_status_t.robot_id;
	else
		CommunatianData_top.dataFrameHeader.receiver_ID = id_red + data.ext_game_robot_status_t.robot_id;
	CommunatianData_x.interactData.grapic_data_struct.graphic_name[0] = 4;
	CommunatianData_x.interactData.grapic_data_struct.graphic_name[1] = 0;
	CommunatianData_x.interactData.grapic_data_struct.graphic_name[2] = 0;
	if (gragh_init) {
		CommunatianData_x.interactData.grapic_data_struct.operate_tpye = 1;
	}
	else {
		CommunatianData_x.interactData.grapic_data_struct.operate_tpye = 2;
	}
	CommunatianData_x.interactData.grapic_data_struct.graphic_tpye = 7;
	CommunatianData_x.interactData.grapic_data_struct.layer = 1;
	if (ctrl.shooter.heat_ulimit) {
		CommunatianData_x.interactData.grapic_data_struct.color = 0;
	}
	else
	{
		CommunatianData_x.interactData.grapic_data_struct.color = 8;
	}
	CommunatianData_x.interactData.grapic_data_struct.start_angle = 30;
	CommunatianData_x.interactData.grapic_data_struct.end_angle = 5;
	CommunatianData_x.interactData.grapic_data_struct.width = 3;
	CommunatianData_x.interactData.grapic_data_struct.start_x = 30;
	CommunatianData_x.interactData.grapic_data_struct.start_y = 680;
	/*
	CommunatianData_x.interactData.grapic_data_struct.radius = 0;
	CommunatianData_x.interactData.grapic_data_struct.end_x = 960;
	CommunatianData_x.interactData.grapic_data_struct.end_y = 680;
	*/
	memset(CommunatianData_x.interactData.data, 0, sizeof(CommunatianData_x.interactData.data));
	CommunatianData_x.interactData.data[0] = 'X';
	memcpy(m_uarttx + 5, (uint8_t *)&CommunatianData_x.CmdID, (sizeof(CommunatianData_x.CmdID) + sizeof(CommunatianData_x.dataFrameHeader) + sizeof(CommunatianData_x.interactData)));
	AppendCRC16CheckSum(m_uarttx, sizeof(CommunatianData_x));
	datalength = sizeof(CommunatianData_x);
	uart6.UARTTransmit(m_uarttx, datalength);
}

void Judgement::Gragh_Lock(void)
{
	uint8_t datalength;
	memset(m_uarttx, 0, DMA_TX_SIZE);
	CommunatianData_lock.txFrameHeader.sof = 0xA5;
	CommunatianData_lock.txFrameHeader.data_length = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_character_t);
	CommunatianData_lock.txFrameHeader.seq = 0;
	memcpy(m_uarttx, &CommunatianData_lock.txFrameHeader, (sizeof(frame_header_t)));
	AppendCRC8CheckSum(m_uarttx, sizeof(frame_header_t));
	CommunatianData_lock.CmdID = 0x0301;
	CommunatianData_lock.dataFrameHeader.data_cmd_id = 0x0110;
	CommunatianData_lock.dataFrameHeader.sender_ID = data.ext_game_robot_status_t.robot_id;

	if (data.ext_game_robot_status_t.robot_id > 10)
		CommunatianData_top.dataFrameHeader.receiver_ID = id_blue + data.ext_game_robot_status_t.robot_id;
	else
		CommunatianData_top.dataFrameHeader.receiver_ID = id_red + data.ext_game_robot_status_t.robot_id;

	CommunatianData_lock.interactData.grapic_data_struct.graphic_name[0] = 5;
	CommunatianData_lock.interactData.grapic_data_struct.graphic_name[1] = 0;
	CommunatianData_lock.interactData.grapic_data_struct.graphic_name[2] = 0;
	if (gragh_init) {
		CommunatianData_lock.interactData.grapic_data_struct.operate_tpye = 1;
	}
	else {
		CommunatianData_lock.interactData.grapic_data_struct.operate_tpye = 2;
	}
	CommunatianData_lock.interactData.grapic_data_struct.graphic_tpye = 7;
	CommunatianData_lock.interactData.grapic_data_struct.layer = 1;
	if (rc.fix) {
		CommunatianData_lock.interactData.grapic_data_struct.color = 0;
	}
	else
	{
		CommunatianData_lock.interactData.grapic_data_struct.color = 8;
	}
	CommunatianData_lock.interactData.grapic_data_struct.start_angle = 30;
	CommunatianData_lock.interactData.grapic_data_struct.end_angle = 5;
	CommunatianData_lock.interactData.grapic_data_struct.width = 3;
	CommunatianData_lock.interactData.grapic_data_struct.start_x = 30;
	CommunatianData_lock.interactData.grapic_data_struct.start_y = 620;
	/*
	CommunatianData_lock.interactData.grapic_data_struct.radius = 0;
	CommunatianData_lock.interactData.grapic_data_struct.end_x = 960;
	CommunatianData_lock.interactData.grapic_data_struct.end_y = 680;
	*/
	memset(CommunatianData_lock.interactData.data, 0, sizeof(CommunatianData_lock.interactData.data));
	CommunatianData_lock.interactData.data[0] = 'L';
	CommunatianData_lock.interactData.data[1] = 'O';
	CommunatianData_lock.interactData.data[2] = 'C';
	CommunatianData_lock.interactData.data[3] = 'K';
	memcpy(m_uarttx + 5, (uint8_t *)&CommunatianData_lock.CmdID, (sizeof(CommunatianData_lock.CmdID) + sizeof(CommunatianData_lock.dataFrameHeader) + sizeof(CommunatianData_lock.interactData)));
	AppendCRC16CheckSum(m_uarttx, sizeof(CommunatianData_lock));
	datalength = sizeof(CommunatianData_lock);
	uart6.UARTTransmit(m_uarttx, datalength);
}

void Judgement::Gragh_Launch(void)
{
	uint8_t datalength;
	memset(m_uarttx, 0, DMA_TX_SIZE);
	CommunatianData_lock.txFrameHeader.sof = 0xA5;
	CommunatianData_lock.txFrameHeader.data_length = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_character_t);
	CommunatianData_lock.txFrameHeader.seq = 0;
	memcpy(m_uarttx, &CommunatianData_lock.txFrameHeader, (sizeof(frame_header_t)));
	AppendCRC8CheckSum(m_uarttx, sizeof(frame_header_t));
	CommunatianData_lock.CmdID = 0x0301;
	CommunatianData_lock.dataFrameHeader.data_cmd_id = 0x0110;
	CommunatianData_lock.dataFrameHeader.sender_ID = data.ext_game_robot_status_t.robot_id;

	if (data.ext_game_robot_status_t.robot_id > 10)
		CommunatianData_top.dataFrameHeader.receiver_ID = id_blue + data.ext_game_robot_status_t.robot_id;
	else
		CommunatianData_top.dataFrameHeader.receiver_ID = id_red + data.ext_game_robot_status_t.robot_id;
	CommunatianData_lock.interactData.grapic_data_struct.graphic_name[0] = 6;
	CommunatianData_lock.interactData.grapic_data_struct.graphic_name[1] = 0;
	CommunatianData_lock.interactData.grapic_data_struct.graphic_name[2] = 0;
	if (gragh_init) {
		CommunatianData_lock.interactData.grapic_data_struct.operate_tpye = 1;
	}
	else {
		CommunatianData_lock.interactData.grapic_data_struct.operate_tpye = 2;
	}
	CommunatianData_lock.interactData.grapic_data_struct.graphic_tpye = 7;
	CommunatianData_lock.interactData.grapic_data_struct.layer = 1;
	if (ctrl.shooter.fraction) {
		CommunatianData_lock.interactData.grapic_data_struct.color = 0;
	}
	else
	{
		CommunatianData_lock.interactData.grapic_data_struct.color = 8;
	}
	CommunatianData_lock.interactData.grapic_data_struct.start_angle = 30;
	CommunatianData_lock.interactData.grapic_data_struct.end_angle = 5;
	CommunatianData_lock.interactData.grapic_data_struct.width = 3;
	CommunatianData_lock.interactData.grapic_data_struct.start_x = 30;
	CommunatianData_lock.interactData.grapic_data_struct.start_y = 560;
	/*
	CommunatianData_lock.interactData.grapic_data_struct.radius = 0;
	CommunatianData_lock.interactData.grapic_data_struct.end_x = 960;
	CommunatianData_lock.interactData.grapic_data_struct.end_y = 680;
	*/
	memset(CommunatianData_lock.interactData.data, 0, sizeof(CommunatianData_lock.interactData.data));
	CommunatianData_lock.interactData.data[0] = 'L';
	CommunatianData_lock.interactData.data[1] = 'A';
	CommunatianData_lock.interactData.data[2] = 'U';
	CommunatianData_lock.interactData.data[3] = 'N';
	CommunatianData_lock.interactData.data[4] = 'C';
	CommunatianData_lock.interactData.data[5] = 'H';
	memcpy(m_uarttx + 5, (uint8_t *)&CommunatianData_lock.CmdID, (sizeof(CommunatianData_lock.CmdID) + sizeof(CommunatianData_lock.dataFrameHeader) + sizeof(CommunatianData_lock.interactData)));
	AppendCRC16CheckSum(m_uarttx, sizeof(CommunatianData_lock));
	datalength = sizeof(CommunatianData_lock);
	uart6.UARTTransmit(m_uarttx, datalength);
}

void Judgement::Gragh_Line(void)
{
	uint8_t datalength;
	memset(m_uarttx, 0, DMA_TX_SIZE);
	CommunatianData.txFrameHeader.sof = 0xA5;
	CommunatianData.txFrameHeader.data_length = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_graphic_seven_t);
	CommunatianData.txFrameHeader.seq = 0;
	memcpy(m_uarttx, &CommunatianData.txFrameHeader, (sizeof(frame_header_t)));
	AppendCRC8CheckSum(m_uarttx, sizeof(frame_header_t));
	CommunatianData.CmdID = 0x0301;
	CommunatianData.dataFrameHeader.data_cmd_id = 0x0104;
	CommunatianData.dataFrameHeader.sender_ID = data.ext_game_robot_status_t.robot_id;

	if (data.ext_game_robot_status_t.robot_id > 10)
		CommunatianData_top.dataFrameHeader.receiver_ID = id_blue + data.ext_game_robot_status_t.robot_id;
	else
		CommunatianData_top.dataFrameHeader.receiver_ID = id_red + data.ext_game_robot_status_t.robot_id;

	CommunatianData.interactData.grapic_data_struct[0].graphic_name[0] = 1;
	CommunatianData.interactData.grapic_data_struct[0].graphic_name[1] = 0;
	CommunatianData.interactData.grapic_data_struct[0].graphic_name[2] = 0;
	CommunatianData.interactData.grapic_data_struct[0].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[0].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[0].layer = 1;
	CommunatianData.interactData.grapic_data_struct[0].color = 0;
	CommunatianData.interactData.grapic_data_struct[0].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[0].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[0].width = 1;
	CommunatianData.interactData.grapic_data_struct[0].start_x = 960;
	CommunatianData.interactData.grapic_data_struct[0].start_y = 540;
	CommunatianData.interactData.grapic_data_struct[0].radius = 0;
	CommunatianData.interactData.grapic_data_struct[0].end_x = 960;
	CommunatianData.interactData.grapic_data_struct[0].end_y = 180;

	CommunatianData.interactData.grapic_data_struct[1].graphic_name[0] = 0;
	CommunatianData.interactData.grapic_data_struct[1].graphic_name[1] = 1;
	CommunatianData.interactData.grapic_data_struct[1].graphic_name[2] = 0;
	CommunatianData.interactData.grapic_data_struct[1].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[1].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[1].layer = 1;
	CommunatianData.interactData.grapic_data_struct[1].color = 0;
	CommunatianData.interactData.grapic_data_struct[1].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[1].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[1].width = 1;
	CommunatianData.interactData.grapic_data_struct[1].start_x = 900;
	CommunatianData.interactData.grapic_data_struct[1].start_y = 480;
	CommunatianData.interactData.grapic_data_struct[1].radius = 0;
	CommunatianData.interactData.grapic_data_struct[1].end_x = 1020;
	CommunatianData.interactData.grapic_data_struct[1].end_y = 480;

	CommunatianData.interactData.grapic_data_struct[2].graphic_name[0] = 0;
	CommunatianData.interactData.grapic_data_struct[2].graphic_name[1] = 1;
	CommunatianData.interactData.grapic_data_struct[2].graphic_name[2] = 1;
	CommunatianData.interactData.grapic_data_struct[2].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[2].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[2].layer = 1;
	CommunatianData.interactData.grapic_data_struct[2].color = 0;
	CommunatianData.interactData.grapic_data_struct[2].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[2].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[2].width = 1;
	CommunatianData.interactData.grapic_data_struct[2].start_x = 840;
	CommunatianData.interactData.grapic_data_struct[2].start_y = 420;
	CommunatianData.interactData.grapic_data_struct[2].radius = 0;
	CommunatianData.interactData.grapic_data_struct[2].end_x = 1080;
	CommunatianData.interactData.grapic_data_struct[2].end_y = 420;

	CommunatianData.interactData.grapic_data_struct[3].graphic_name[0] = 0;
	CommunatianData.interactData.grapic_data_struct[3].graphic_name[1] = 0;
	CommunatianData.interactData.grapic_data_struct[3].graphic_name[2] = 1;
	CommunatianData.interactData.grapic_data_struct[3].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[3].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[3].layer = 1;
	CommunatianData.interactData.grapic_data_struct[3].color = 0;
	CommunatianData.interactData.grapic_data_struct[3].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[3].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[3].width = 1;
	CommunatianData.interactData.grapic_data_struct[3].start_x = 900;
	CommunatianData.interactData.grapic_data_struct[3].start_y = 360;
	CommunatianData.interactData.grapic_data_struct[3].radius = 0;
	CommunatianData.interactData.grapic_data_struct[3].end_x = 1020;
	CommunatianData.interactData.grapic_data_struct[3].end_y = 360;

	CommunatianData.interactData.grapic_data_struct[4].graphic_name[0] = 1;
	CommunatianData.interactData.grapic_data_struct[4].graphic_name[1] = 0;
	CommunatianData.interactData.grapic_data_struct[4].graphic_name[2] = 1;
	CommunatianData.interactData.grapic_data_struct[4].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[4].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[4].layer = 1;
	CommunatianData.interactData.grapic_data_struct[4].color = 0;
	CommunatianData.interactData.grapic_data_struct[4].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[4].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[4].width = 1;
	CommunatianData.interactData.grapic_data_struct[4].start_x = 900;
	CommunatianData.interactData.grapic_data_struct[4].start_y = 300;
	CommunatianData.interactData.grapic_data_struct[4].radius = 0;
	CommunatianData.interactData.grapic_data_struct[4].end_x = 1020;
	CommunatianData.interactData.grapic_data_struct[4].end_y = 300;

	CommunatianData.interactData.grapic_data_struct[5].graphic_name[0] = 0;
	CommunatianData.interactData.grapic_data_struct[5].graphic_name[1] = 1;
	CommunatianData.interactData.grapic_data_struct[5].graphic_name[2] = 1;
	CommunatianData.interactData.grapic_data_struct[5].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[5].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[5].layer = 1;
	CommunatianData.interactData.grapic_data_struct[5].color = 0;
	CommunatianData.interactData.grapic_data_struct[5].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[5].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[5].width = 1;
	CommunatianData.interactData.grapic_data_struct[5].start_x = 900;
	CommunatianData.interactData.grapic_data_struct[5].start_y = 180;
	CommunatianData.interactData.grapic_data_struct[5].radius = 0;
	CommunatianData.interactData.grapic_data_struct[5].end_x = 1020;
	CommunatianData.interactData.grapic_data_struct[5].end_y = 180;

	CommunatianData.interactData.grapic_data_struct[6].graphic_name[0] = 1;
	CommunatianData.interactData.grapic_data_struct[6].graphic_name[1] = 1;
	CommunatianData.interactData.grapic_data_struct[6].graphic_name[2] = 1;
	CommunatianData.interactData.grapic_data_struct[6].operate_tpye = 1;
	CommunatianData.interactData.grapic_data_struct[6].graphic_tpye = 0;
	CommunatianData.interactData.grapic_data_struct[6].layer = 1;
	CommunatianData.interactData.grapic_data_struct[6].color = 0;
	CommunatianData.interactData.grapic_data_struct[6].start_angle = 30;
	CommunatianData.interactData.grapic_data_struct[6].end_angle = 5;
	CommunatianData.interactData.grapic_data_struct[6].width = 3;
	CommunatianData.interactData.grapic_data_struct[6].start_x = 900;
	CommunatianData.interactData.grapic_data_struct[6].start_y = 240;
	CommunatianData.interactData.grapic_data_struct[6].radius = 0;
	CommunatianData.interactData.grapic_data_struct[6].end_x = 1020;
	CommunatianData.interactData.grapic_data_struct[6].end_y = 240;
	/*
	memset(CommunatianData.interactData.data, 0, sizeof(CommunatianData.interactData.data));
	CommunatianData.interactData.data[0] = 'R';
	CommunatianData.interactData.data[1] = 'C';
	CommunatianData.interactData.data[2] = 'S';
	*/

	memcpy(m_uarttx + 5, (uint8_t *)&CommunatianData.CmdID, (sizeof(CommunatianData.CmdID) + sizeof(CommunatianData.dataFrameHeader) + sizeof(CommunatianData.interactData)));
	/*
	memcpy(TeammateTxBuffer+5,(uint8_t *)&CommuData.CmdID,sizeof(CommuData.CmdID));
	memcpy(TeammateTxBuffer+7,(uint8_t *)&CommuData.dataFrameHeader,sizeof(CommuData.dataFrameHeader));
	memcpy(TeammateTxBuffer+13,(uint8_t *)&CommuData.interactData,sizeof(CommuData.interactData));
	*/
	AppendCRC16CheckSum(m_uarttx, sizeof(CommunatianData));

	datalength = sizeof(CommunatianData);

	uart6.UARTTransmit(m_uarttx, datalength);
}


void Judgement::SendData(void)
{
	Encode();
}
