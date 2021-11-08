#ifndef __CLOCK_H__
#define __CLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define       ACLK_FREQ  32768
#define  CLK_LFXT_DRIVE  UCS_XT1_DRIVE_0

void clock_port_init(void);
void clock_init(void);

#ifdef __cplusplus
}
#endif

#endif