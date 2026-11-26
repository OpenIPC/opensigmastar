/*
 * MHAL_DISP_DeviceSetTimeZoneConfig for the vendor mi_disp.
 *
 * The i2m NVR SDK's mi_disp (2020-03) calls it once from
 * MI_DISP_IMPL_SetPubAttr, with the share (percent) of each part of the
 * frame -- sync, back porch, active, front porch -- in which a flip is not
 * safe, and ignores the result. Its mhal used that to time
 * MHAL_DISP_DeviceGetTimeZone. SigmaStar's boot release (2020-05) has no
 * such call: its GetTimeZone reports the zone from the display's own
 * VSYNC/VDE interrupt flags. So the configuration is accepted and dropped.
 */
#include "mhal_common.h"
#include "mhal_disp_datatype.h"

/* The SDK's MHAL_DISP_TimeZoneConfig_t, from the vendor mhal's DWARF. */
typedef struct {
	MS_U16 u16InvalidArea[5];
} MHAL_DISP_TimeZoneConfig_t;

MS_BOOL MHAL_DISP_DeviceSetTimeZoneConfig(void *pDevCtx,
					  MHAL_DISP_TimeZoneConfig_t *pstTimeZoneCfg)
{
	return pDevCtx ? TRUE : FALSE;
}
