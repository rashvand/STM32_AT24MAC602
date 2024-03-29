

#define AT24MAC602_EEPROM_CHIPADDRESS_BASE  ( 0xA0 ) //!< EEPROM chip base address
#define AT24MAC602_PSWP_CHIPADDRESS_BASE    ( 0x60 ) //!< Permanent Software Write Protection (PSWP) chip base address
#define AT24MAC602_SERIAL_CHIPADDRESS_BASE  ( 0xB0 ) //!< Unique Serial Number chip base address
#define AT24MAC602_EUI_CHIPADDRESS_BASE     ( 0xB0 ) //!< EUI-64™ chip base address
#define AT24MAC602_CHIPADDRESS_MASK         ( 0xFE ) //!< Chip address mask

#define AT24MAC602_RSWP_SET_CHIPADDRESS     ( 0x62 ) //!< Set Reversible Software Write Protection (RSWP) chip base address (See datasheet for hardware configuration)
#define AT24MAC602_RSWP_CLEAR_CHIPADDRESS   ( 0x66 ) //!< Clear Reversible Software Write Protection (RSWP) chip base address (See datasheet for hardware configuration)

#define AT24MAC602_SERIAL_MEMORYADDR        ( 0x80 ) //!< Memory address where the Serial Number is
#define AT24MAC602_EUI64_MEMORYADDR         ( 0x98 ) //!< Memory address where the EUI64 is

#define AT24MAC602_ADDRESS_SIZE_MAX         ( 16 ) //!< The AT24MAC602 has 256 page maximum

#define AT24MAC602_PAGE_SIZE                ( 16 ) //!< The AT24MAC602 is 16 bytes page size
#define AT24MAC602_PAGE_SIZE_MASK           ( AT24MAC602_PAGE_SIZE - 1 ) //!< The AT24MAC602 page mask is 0x0F
#define AT24MAC602_EEPROM_SIZE              ( AT24MAC602_ADDRESS_SIZE_MAX * AT24MAC602_PAGE_SIZE ) //!< The AT24MAC602 total EEPROM size

#define AT24MAC602_SERIAL_SIZE              ( 16 ) //!< AT24MAC602 Unique Serial Number size

#define EUI64_OUI_LEN  ( 3 ) 											 //!< Organizationally Unique Identifier (OUI) size is 3 bytes
#define EUI64_NIC_LEN  ( 5 ) 											 //!< Network Interface Controller (NIC) size is 5 bytes
#define EUI64_LEN      ( EUI64_OUI_LEN + EUI64_NIC_LEN )





__STATIC_INLINE void MX_I2C4_Init(void){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
	PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
		Error_Handler();
	}
	
	/**I2C4 GPIO Configuration
	PB7     ------> I2C4_SDA
	PB8     ------> I2C4_SCL
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF6_I2C4;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Peripheral clock enable */
	__HAL_RCC_I2C4_CLK_ENABLE();
	
	hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00401242; // 0x009034B6 -> 400KHz, 0x00401242 -> 1MHz
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK){
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK){
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK){
    Error_Handler();
  }
}





__STATIC_INLINE HAL_StatusTypeDef EEPROM_GetEUI64(uint8_t *pData){
	return HAL_I2C_Mem_Read(&hi2c4, AT24MAC602_EUI_CHIPADDRESS_BASE, AT24MAC602_EUI64_MEMORYADDR, I2C_MEMADD_SIZE_8BIT, pData, EUI64_LEN, 100);
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_Get128bitsSerialNumber(uint8_t *pData){
	return HAL_I2C_Mem_Read(&hi2c4, AT24MAC602_EUI_CHIPADDRESS_BASE, AT24MAC602_SERIAL_MEMORYADDR, I2C_MEMADD_SIZE_8BIT, pData, AT24MAC602_SERIAL_SIZE, 100);
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_SetPermanentWriteProtection(void){ // Alert! PSWP cannot be reversed even if the device is powered down
	uint8_t data[1] = {0};
	HAL_StatusTypeDef Status;
	
	Status = HAL_I2C_Mem_Write(&hi2c4, AT24MAC602_PSWP_CHIPADDRESS_BASE, 0, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
	
	return Status;
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_ReversibleWriteProtection(bool RSWP_State){
	uint8_t data[1] = {0};
	HAL_StatusTypeDef Status = HAL_OK;
	
	EEPROM_WP____RESET;
	
	if(RSWP_State == false)
		Status = HAL_I2C_Mem_Write(&hi2c4, AT24MAC602_RSWP_CLEAR_CHIPADDRESS, 0, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
	else
		Status = HAL_I2C_Mem_Write(&hi2c4, AT24MAC602_RSWP_SET_CHIPADDRESS, 0, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
	
	EEPROM_WP____SET;
	
	return Status;
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_ReadRegister(uint16_t address, uint8_t* data, uint16_t size){
	
	return HAL_I2C_Mem_Read(&hi2c4, (AT24MAC602_EEPROM_CHIPADDRESS_BASE | 0x01), address, I2C_MEMADD_SIZE_8BIT, data, size, 100);
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_Write(uint16_t address, uint8_t* pData, uint16_t size){
	
	HAL_StatusTypeDef Status = HAL_OK;
	
	EEPROM_WP____RESET;
	if(HAL_I2C_Mem_Write(&hi2c4, AT24MAC602_EEPROM_CHIPADDRESS_BASE, address, I2C_MEMADD_SIZE_8BIT, pData, size, 100) == HAL_OK){
		HAL_Delay(10); // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics)
	}
	EEPROM_WP____SET;
	
	return Status;
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_Read(uint16_t address, uint8_t* pData, uint16_t size){
		
	return HAL_I2C_Mem_Read(&hi2c4, AT24MAC602_EEPROM_CHIPADDRESS_BASE, address, I2C_MEMADD_SIZE_8BIT, pData, size, 100);
}

__STATIC_INLINE HAL_StatusTypeDef EEPROM_ErasePage(uint8_t Page){
	
	HAL_StatusTypeDef Status;
	
	uint8_t EraseBuf[16] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	
	EEPROM_WP____RESET;
	
	if(Page == 255){
		for(uint16_t i=0; i<AT24MAC602_ADDRESS_SIZE_MAX; i++){
			Status = HAL_I2C_Mem_Write(&hi2c4, AT24MAC602_EEPROM_CHIPADDRESS_BASE, i, I2C_MEMADD_SIZE_8BIT, EraseBuf, AT24MAC602_PAGE_SIZE, 100);
			if(Status != HAL_OK){
				Status = HAL_ERROR;
				break;
			}
			HAL_Delay(10);
		}
	}
	else if(Page <= AT24MAC602_PAGE_SIZE){
		Status = HAL_I2C_Mem_Write(&hi2c4, AT24MAC602_EEPROM_CHIPADDRESS_BASE, Page, I2C_MEMADD_SIZE_8BIT, EraseBuf, AT24MAC602_PAGE_SIZE, 100);
		if(Status != HAL_OK){
			Status = HAL_ERROR;
		}
		HAL_Delay(10);
	}
	else{
		Status = HAL_ERROR;
	}
	
	EEPROM_WP____SET;
	
	return HAL_OK;
}
