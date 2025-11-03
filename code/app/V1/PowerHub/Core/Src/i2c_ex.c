/* Includes ------------------------------------------------------------------*/
#include "i2c_ex.h"
#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "main.h"

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

__IO uint8_t aReceiveBuffer[I2C_RECEIVE_BUFFER_LEN];
__IO uint8_t tx_buffer[I2C_RECEIVE_BUFFER_LEN];
__IO uint16_t ubReceiveIndex = 0;
volatile uint8_t i2c_addr = 0;
volatile uint16_t tx_buffer_index = 0;
volatile uint16_t tx_len = 0;
volatile uint32_t i2c_timeout_counter = 0;
volatile uint32_t i2c_stop_timeout_flag = 0;
volatile uint32_t i2c_stop_timeout_counter = 0;
volatile uint8_t i2c_slave_state = I2C_SLAVE_STATE_IDLE;
volatile uint32_t i2c_ex_timeout_start = 0;

void set_i2c_slave_address(uint8_t addr)
{
  i2c_addr = (addr << 1);
}

__weak void Slave_Complete_Callback(uint8_t *rx_data, uint16_t len)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(rx_data);
	UNUSED(len);  
}

void i2c1_it_enable(void)
{
  LL_I2C_Enable(I2C1);

  /* (6) Enable I2C1 address match/error interrupts:
   *  - Enable Address Match Interrupt
   *  - Enable Not acknowledge received interrupt
   *  - Enable Error interrupts
   *  - Enable Stop interrupt
   */
  LL_I2C_EnableIT_ADDR(I2C1);
	// LL_I2C_EnableIT_TX(I2C1);
	// LL_I2C_EnableIT_RX(I2C1);  
  LL_I2C_EnableIT_NACK(I2C1);
  LL_I2C_EnableIT_ERR(I2C1);
  LL_I2C_EnableIT_STOP(I2C1);
}

void i2c1_tx_rx_it_enable(void)
{
	LL_I2C_EnableIT_TX(I2C1);
	LL_I2C_EnableIT_RX(I2C1);  
}

void i2c1_it_disable(void)
{
  LL_I2C_DisableIT_ADDR(I2C1);
	// LL_I2C_DisableIT_TX(I2C1);
	// LL_I2C_DisableIT_RX(I2C1);
  LL_I2C_DisableIT_NACK(I2C1);
  LL_I2C_DisableIT_ERR(I2C1);
  LL_I2C_DisableIT_STOP(I2C1);
}

void i2c1_all_it_disable(void)
{
  LL_I2C_DisableIT_ADDR(I2C1);
	LL_I2C_DisableIT_TX(I2C1);
	LL_I2C_DisableIT_RX(I2C1);
  LL_I2C_DisableIT_NACK(I2C1);
  LL_I2C_DisableIT_ERR(I2C1);
  LL_I2C_DisableIT_STOP(I2C1);
}

void i2c1_reset(void)
{
  LL_I2C_DeInit(I2C1);
  LL_I2C_Disable(I2C1);
  i2c1_all_it_disable();
  user_i2c_init();    
  i2c1_it_enable(); 
  i2c_stop_timeout_flag = 0;
  i2c_stop_timeout_counter = 0;
  i2c_timeout_counter = 0;   
}

void i2c1_set_send_data(uint8_t *tx_ptr, uint16_t len) {
  if (len > I2C_RECEIVE_BUFFER_LEN) {
    len = I2C_RECEIVE_BUFFER_LEN;
	}

  if (len == 0 || tx_ptr == NULL) {
    return;
  }
  memcpy((void *)tx_buffer, tx_ptr, len);
  tx_buffer_index = 0;
  tx_len = len;
}

void Slave_Reception_Callback(void)
{
  if (ubReceiveIndex >= I2C_RECEIVE_BUFFER_LEN) {
    ubReceiveIndex = 0;
  }  
  /* Read character in Receive Data register.
  RXNE flag is cleared by reading data in RXDR register */
  aReceiveBuffer[ubReceiveIndex] = LL_I2C_ReceiveData8(I2C1);
  ubReceiveIndex++;
}

void Slave_Ready_To_Transmit_Callback(void)
{
  if (tx_buffer_index >= I2C_RECEIVE_BUFFER_LEN) {
    tx_buffer_index = 0;
  }  
  /* Send the Byte requested by the Master */
  LL_I2C_TransmitData8(I2C1, tx_buffer[tx_buffer_index]);
  tx_buffer_index++;
}

void I2C1_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_IRQn 0 */
  i2c_timeout_counter++;
  if (i2c_timeout_counter > 90000) {
    LL_I2C_DeInit(I2C1);
    LL_I2C_DisableAutoEndMode(I2C1);
    LL_I2C_Disable(I2C1);
    LL_I2C_DisableIT_ADDR(I2C1);         
    user_i2c_init();    
    i2c1_it_enable();          
    i2c_timeout_counter = 0;
  }
  /* Check ADDR flag value in ISR register */
  if(LL_I2C_IsActiveFlag_ADDR(I2C1))
  {
    /* Verify the Address Match with the OWN Slave address */
    if(LL_I2C_GetAddressMatchCode(I2C1) == i2c_addr)
    {
      i2c_ex_timeout_start = HAL_GetTick();
      /* Verify the transfer direction, a write direction, Slave enters receiver mode */
      if(LL_I2C_GetTransferDirection(I2C1) == LL_I2C_DIRECTION_WRITE)
      {
        ubReceiveIndex = 0;
        /* Clear ADDR flag value in ISR register */
        LL_I2C_ClearFlag_ADDR(I2C1);

        /* Enable Receive Interrupt */
        LL_I2C_EnableIT_RX(I2C1);
        i2c_stop_timeout_flag = 1;
        i2c_slave_state = I2C_SLAVE_STATE_READ;
      }
      /* Verify the transfer direction, a read direction, Slave enters transmitter mode */
      else if(LL_I2C_GetTransferDirection(I2C1) == LL_I2C_DIRECTION_READ)
      {
        tx_buffer_index = 0;
        /* Clear ADDR flag value in ISR register */
        LL_I2C_ClearFlag_ADDR(I2C1);

        // 准备I2C数据
        if (ubReceiveIndex) {
          i2c1_it_disable();
          Slave_Complete_Callback((uint8_t *)aReceiveBuffer, ubReceiveIndex);
          ubReceiveIndex = 0;
          i2c1_it_enable(); 
        }        

        /* Enable Transmit Interrupt */
        LL_I2C_EnableIT_TX(I2C1);

        i2c_slave_state = I2C_SLAVE_STATE_WRITE;
      }      
      else
      {
        /* Clear ADDR flag value in ISR register */
        LL_I2C_ClearFlag_ADDR(I2C1);
      }
    }
    else
    {
      /* Clear ADDR flag value in ISR register */
      LL_I2C_ClearFlag_ADDR(I2C1);
    }
  }
  /* Check NACK flag value in ISR register */
  else if(LL_I2C_IsActiveFlag_NACK(I2C1))
  {
    /* End of Transfer */
    LL_I2C_ClearFlag_NACK(I2C1);
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    i2c_stop_timeout_flag = 0;
    i2c_stop_timeout_counter = 0;
    i2c_timeout_counter = 0;     
  } 
  /* Check TXIS flag value in ISR register */
  else if(LL_I2C_IsActiveFlag_TXIS(I2C1))
  {
    /* Call function Slave Ready to Transmit Callback */
    Slave_Ready_To_Transmit_Callback();
  }   
  /* Check RXNE flag value in ISR register */
  else if(LL_I2C_IsActiveFlag_RXNE(I2C1))
  {
    /* Call function Slave Reception Callback */
    Slave_Reception_Callback();
  }
  /* Check STOP flag value in ISR register */
  else if(LL_I2C_IsActiveFlag_STOP(I2C1))
  {
    /* End of Transfer */
    LL_I2C_ClearFlag_STOP(I2C1);

    /* Check TXE flag value in ISR register */
    if(!LL_I2C_IsActiveFlag_TXE(I2C1))
    {
      /* Flush the TXDR register */
      LL_I2C_ClearFlag_TXE(I2C1);
    }    
    
    i2c1_it_disable();
    /* Call function Slave Complete Callback */
    if (ubReceiveIndex) {
      Slave_Complete_Callback((uint8_t *)aReceiveBuffer, ubReceiveIndex);
    }
    ubReceiveIndex = 0;
    i2c1_it_enable();
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
    i2c_stop_timeout_flag = 0;
    i2c_stop_timeout_counter = 0;
  }
  /* Check TXE flag value in ISR register */
  else if(!LL_I2C_IsActiveFlag_TXE(I2C1))
  {
    /* Flush the TXDR register */
    LL_I2C_ClearFlag_TXE(I2C1);    
    /* Do nothing */
    /* This Flag will be set by hardware when the TXDR register is empty */
    /* If needed, use LL_I2C_ClearFlag_TXE() interface to flush the TXDR register  */
  }
  /* USER CODE END I2C1_IRQn 0 */
  if (LL_I2C_IsActiveFlag_BERR(I2C1)) {
    LL_I2C_ClearFlag_BERR(I2C1);
    i2c1_reset();
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
  }
  if (LL_I2C_IsActiveFlag_ARLO(I2C1)) {
    LL_I2C_ClearFlag_ARLO(I2C1);
    i2c1_reset();
    i2c_slave_state = I2C_SLAVE_STATE_IDLE;
  }
  /* USER CODE BEGIN I2C1_IRQn 1 */

  /* USER CODE END I2C1_IRQn 1 */
}
