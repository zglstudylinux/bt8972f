#include "config.h"
depend(0x01030500);
setflash(1, FLASH_SIZE, FLASH_ERASE_4K, FLASH_DUAL_READ, FLASH_QUAD_READ);
#if (FLASH_RES2_SIZE > 0)
setkeepseg(FLASH_RES2_SIZE);
#endif
setspace(FLASH_RESERVE_SIZE);
//setauth(0xDDE05A0D, soft_key);    //LDAC加密狗
#if ((FLASH_SIZE == FSIZE_2M) && (AB_FOT_TYPE == AB_FOT_TYPE_PACK))
setunpack(unpack.bin);
setpkgarea(0x12C000, 0xC3000);
#endif
make(dcf_buf, header.bin, app.bin, res.bin, xcfg.bin, updater.bin);
save(dcf_buf, app.dcf);
