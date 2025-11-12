# Storage Structure Description
## 1. FLASH Structure
| Start Address | End Address | Region Name | Size | Function Description |
| -------------- | ------------ | ------------ | ----- | -------------------- |
| 0x08000000 | 0x080017FF | Boot | 6 KB | Stores the bootloader and firmware upgrade logic |
| 0x08001800 | 0x0800F7FF | App | 56 KB | Storage area for the user application program |
| 0x0800F800 | 0x0800F9FF | Data | 1 KB | Stores system parameters |
---
## 2. RAM Structure

**Note:** Boot and App share the same RAM area and occupy their corresponding spaces during different runtime stages.

| Start Address | End Address | Region Name | Size | Function Description |
| -------------- | ------------ | ------------ | ----- | -------------------- |
| 0x20000000 | 0x20001FFF | Boot | 8 KB | Used during the boot stage for buffering and variable storage |
| 0x20000000 | 0x20001FFF | App | 8 KB | Used during application runtime for task stacks, variables, and caching |
---
## 3. Detailed Description
- **Boot Area (6 KB)**  
  Used to store the bootloader and firmware upgrade control logic.  
  The last byte stores the firmware version number for version management and upgrade validation.  
- **App Area (56 KB)**  
  Used to store the user application program.  
  The last 4 bytes are reserved for CRC checksum storage. The bootloader verifies the program before loading.  
  The verification uses the **standard CRC‑32 algorithm** (IEEE 802.3, polynomial `0x04C11DB7`, initial value `0xFFFFFFFF`, input/output reflected, result bitwise inverted, and stored in little‑endian byte order).  
- **Reserved Area (1 KB)**  
  Used to store system parameters.