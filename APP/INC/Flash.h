#ifndef _FLASH_H
#define _FLASH_H

void EraseAllFlash(void);
void StoreIntoFlash(void);
void ReadFlashData(u32 ReadAddress,u16 Length);
void FlashProcess(void);   //Store Laser Parameter into flash

#endif /*_FLASH_H*/
