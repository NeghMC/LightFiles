/*
 * Author: NeghMC
 * Project: LightFiles
 * Description:
 *   This file provides implementation of the low-level functions
 *   which are used to operate on the flash memory.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Driver Header files */
#include <ti/drivers/NVS.h>

/* Driver configuration */
#include "ti_drivers_config.h"

// LF header
#include "light_files.h"

static NVS_Handle nvsHandle;
static NVS_Attrs regionAttrs;
static NVS_Params nvsParams;

lf_result_t lf_app_init(uint16_t *blockCount, uint16_t *blockSize)
{
    NVS_init();

    NVS_Params_init(&nvsParams);
    nvsHandle = NVS_open(CONFIG_NVSEXTERNAL, &nvsParams);

    if (nvsHandle == NULL)
    {
        return LF_RESULT_FAILED;
    }

    /*
     * This will populate a NVS_Attrs structure with properties specific
     * to a NVS_Handle such as region base address, region size,
     * and sector size.
     */
    NVS_getAttrs(nvsHandle, &regionAttrs);

    // set LF parameters
    *blockSize = regionAttrs.sectorSize;
    *blockCount = regionAttrs.regionSize/regionAttrs.sectorSize;

    //NVS_erase(nvsHandle, 0, regionAttrs.regionSize);

    return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, uint16_t length, uint8_t flush)
{
    uint16_t ret = NVS_write(nvsHandle, block*regionAttrs.sectorSize + offset, buffer, length, 0);
    if(ret == NVS_STATUS_SUCCESS)
    {
        return LF_RESULT_SUCCESS;
    }
    else
    {
        return LF_RESULT_FAILED;
    }
}

lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, uint16_t length)
{
    uint16_t ret = NVS_read(nvsHandle, block*regionAttrs.sectorSize + offset, buffer, length);
    if(ret == NVS_STATUS_SUCCESS)
    {
        return LF_RESULT_SUCCESS;
    }
    else
    {
        return LF_RESULT_FAILED;
    }
}

lf_result_t lf_app_delete(uint16_t block)
{
    uint16_t ret = NVS_erase(nvsHandle, block*regionAttrs.sectorSize, regionAttrs.sectorSize);
    if(ret == NVS_STATUS_SUCCESS)
    {
        return LF_RESULT_SUCCESS;
    }
    else
    {
        return LF_RESULT_FAILED;
    }
}
