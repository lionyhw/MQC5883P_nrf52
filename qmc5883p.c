/**
  ******************************************************************************
  * @file    qmc5883p.c
  * @author  Llane
  * @version V1.0
  * @date    2021-xx-xx
  * @brief    qmc5883pÇý¶¯
  *****************************************************************************
  */ 

#include "qmc5883p.h"

//#define QMC5883P_DEBUG
#define QMC5883P_INIT_DEBUG

uint8_t chipid = 0;
int t1_printf = 0;

//IIC address  0x2c//7bit
//IIC  read  
static void qmc5883p_read_block(uint8_t addr, uint8_t *buf, uint32_t len)
{
		nrf_drv_twi_tx(&m_twi,QMC5883P_IIC_ADDR,&addr,1,false);

	  nrf_drv_twi_rx(&m_twi,QMC5883P_IIC_ADDR,buf,len);

}

static void qmc5883p_write_reg(uint8_t addr, uint8_t buf)
{
	uint8_t write_data[2];
	write_data[0] = addr;
	write_data[1] = buf;
	nrf_drv_twi_tx(&m_twi,QMC5883P_IIC_ADDR,write_data,2,false);
}

int qmc5883p_init(void)
{	
	qmc5883p_read_block(QMC5883P_CHIP_ID_REG, &chipid , 1);

#ifdef QMC5883P_INIT_DEBUG
	uart_printf("QMC5883P_get_chipid chipid = 0x%x,i2c_addr = 0x2c\n", chipid);
#endif	
	
	if(chipid == 0x80)
	{
		qmc5883p_write_reg(0x0d, 0x40);
		nrf_delay_us(10000);
		qmc5883p_write_reg(0x29, 0x06);
		nrf_delay_us(10000);
		qmc5883p_write_reg(QMC5883P_CTL_REG_ONE, 0xCF);//0x0A = 0xC3/0xC7/0xCB/0xCF; 	/*ODR = 10/50/100/200Hz,  MODE = continuous*/
		nrf_delay_us(10000);
		qmc5883p_write_reg(QMC5883P_CTL_REG_TWO, 0x00); //0x0B = 0x00/0x04/0x08/0x0C; 	/*RNG = ¡À 30G / ¡À 12G / ¡À 8G / ¡À 2G */
		nrf_delay_us(10000);

	#ifdef QMC5883P_DEBUG
		unsigned char ctrl_value;
		
		qmc5883p_read_block(0x0d, &ctrl_value, 1);
		uart_printf("QMC5883P 0x%x=0x%x \r\n", 0x0d, ctrl_value);//0x40
		
		qmc5883p_read_block(0x29, &ctrl_value, 1);
		uart_printf("QMC5883P 0x%x=0x%x \r\n", 0x29, ctrl_value);//0x06
		
		qmc5883p_read_block(QMC5883P_CTL_REG_ONE, &ctrl_value, 1);
		uart_printf("QMC5883P 0x%x=0x%x \r\n", QMC5883P_CTL_REG_ONE, ctrl_value);//0xc3

		qmc5883p_read_block(QMC5883P_CTL_REG_TWO, &ctrl_value, 1);
		uart_printf("QMC5883P 0x%x=0x%x \r\n", QMC5883P_CTL_REG_TWO, ctrl_value);//0x00
	#endif
		return 1;
	}
	else
	{
		uart_printf("%s: QMC5883P_get_chipid failed\n");
		return 0;
	}
}



uint8_t qmc5883p_read_mag_xyz(float *data)
{

	uint8_t mag_data[6];
	short hw_d[3] = {0};
	short raw_c[3];
	int t1 = 0;
	unsigned char rdy = 0;
	t1 = 0;
	t1_printf = 0;
	/* Check status register for data availability */
	while(!(rdy & 0x01) && (t1 < 5))
	{
		rdy = QMC5883P_STATUS_REG;
		qmc5883p_read_block(QMC5883P_STATUS_REG, &rdy, 1);
		t1++;
		t1_printf = t1;
	}

	mag_data[0] = QMC5883P_DATA_OUT_X_LSB_REG;

	qmc5883p_read_block(QMC5883P_DATA_OUT_X_LSB_REG, mag_data, 6);

	hw_d[0] = (((mag_data[1]) << 8) | mag_data[0]);
	hw_d[1] = (((mag_data[3]) << 8) | mag_data[2]);
	hw_d[2] = (((mag_data[5]) << 8) | mag_data[4]);
	uart_printf("Magnetic: %d %d %d \r\n",hw_d[0], hw_d[1], hw_d[2]);

//	//Unit:mG  1G = 100uT = 1000mG
//	//printf("Hx=%d, Hy=%d, Hz=%d\n",hw_d[0],hw_d[1],hw_d[2]);
//	raw_c[0] = (int)(hw_d[0]);
//	raw_c[1] = (int)(hw_d[1]);
//	raw_c[2] = (int)(hw_d[2]);
//	//If Range = ¡À2G, 15000LSB/G	If Range = ¡À 8G, 3750LSB/G 
//	//If Range = ¡À12G, 2500LSB/G	If Range = ¡À 30G, 1000LSB/G
//	data[0] = (float)raw_c[0] / 1000.0f;
//	data[1] = (float)raw_c[1] / 1000.0f;
//	data[2] = (float)raw_c[2] / 1000.0f;

	return 1;
}
