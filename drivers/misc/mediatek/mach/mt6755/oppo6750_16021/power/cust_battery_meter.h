#ifndef _CUST_BATTERY_METER_H
#define _CUST_BATTERY_METER_H

#include <mach/mt_typedefs.h>

/* ============================================================*/
/* define*/
/* ============================================================*/
/*#define SOC_BY_AUXADC*/
#define SOC_BY_HW_FG
/*#define HW_FG_FORCE_USE_SW_OCV*/
/*#define SOC_BY_SW_FG*/

/*#define CONFIG_DIS_CHECK_BATTERY*/
/* #define FIXED_TBAT_25 */

/* ADC resistor  */
#define R_BAT_SENSE	4
#define R_I_SENSE	4
#define R_CHARGER_1	330
#define R_CHARGER_2	39

#define TEMPERATURE_T0	110
#define TEMPERATURE_T1	0
#define TEMPERATURE_T2	25
#define TEMPERATURE_T3	50
#define TEMPERATURE_T	255 /* This should be fixed, never change the value*/

#ifndef VENDOR_EDIT /* OPPO 2016-03-15 sjc Modify for charging */
#define FG_METER_RESISTANCE	0
#else
#define FG_METER_RESISTANCE	5
#endif

/* Qmax for battery  */
#ifndef VENDOR_EDIT /* OPPO 2016-01-30 sjc Modify for charging */
#define Q_MAX_POS_50	2743
#define Q_MAX_POS_25 2709
#define Q_MAX_POS_0 1168
#define Q_MAX_NEG_10 762

#define Q_MAX_POS_50_H_CURRENT	2688
#define Q_MAX_POS_25_H_CURRENT 2655
#define Q_MAX_POS_0_H_CURRENT 1145
#define Q_MAX_NEG_10_H_CURRENT 747
#else
#define Q_MAX_POS_50	3031
#define Q_MAX_POS_25	3035
#define Q_MAX_POS_0	2818
#define Q_MAX_NEG_10	2375

#define Q_MAX_POS_50_H_CURRENT	3016
#define Q_MAX_POS_25_H_CURRENT	3014
#define Q_MAX_POS_0_H_CURRENT	2612
#define Q_MAX_NEG_10_H_CURRENT	1388
#endif /*VENDOR_EDIT*/


/* Discharge Percentage */
#define OAM_D5	1		/*  1 : D5,   0: D2*/


/* battery meter parameter */
#define CHANGE_TRACKING_POINT
#ifdef CONFIG_MTK_HAFG_20
#define CUST_TRACKING_POINT  0
#else
#define CUST_TRACKING_POINT  1
#endif
#define CUST_R_SENSE         56
#define CUST_HW_CC 		    0
#define AGING_TUNING_VALUE   103
#define CUST_R_FG_OFFSET    0

#define OCV_BOARD_COMPESATE	0 /*mV*/
#define R_FG_BOARD_BASE		1000
#define R_FG_BOARD_SLOPE	1000 /*slope*/
#ifndef VENDOR_EDIT /* OPPO 2016-03-15 sjc Modify for charging */
#define CAR_TUNE_VALUE		101 /*1.00*/
#else
#define CAR_TUNE_VALUE		102 /*1.00*/
#endif

/* HW Fuel gague  */
#define CURRENT_DETECT_R_FG	10  /*1mA*/
#define MinErrorOffset       1000
#define FG_VBAT_AVERAGE_SIZE 18
#define R_FG_VALUE	10 /* mOhm, base is 20*/

/* fg 2.0 */
#define DIFFERENCE_HWOCV_RTC		30
#define DIFFERENCE_HWOCV_SWOCV		10
#ifndef VENDOR_EDIT /* OPPO 2016-03-28 sjc Modify for charging */
#define DIFFERENCE_SWOCV_RTC		30
#else
#define DIFFERENCE_SWOCV_RTC		10
#endif
#define DIFFERENCE_HWOCV_VBAT		30
#define DIFFERENCE_VBAT_RTC			30
#ifndef VENDOR_EDIT /* OPPO 2016-04-25 sjc Modify for charging */
#define DIFFERENCE_SWOCV_RTC_POS	15
#else
#define DIFFERENCE_SWOCV_RTC_POS	100
#endif
#define MAX_SWOCV			3

#define DIFFERENCE_VOLTAGE_UPDATE	20
#define AGING1_LOAD_SOC			70
#define AGING1_UPDATE_SOC		30
#ifndef VENDOR_EDIT /* OPPO 2016-01-30 sjc Modify for charging */
#define BATTERYPSEUDO100		95
#define BATTERYPSEUDO1 6
#else
#define BATTERYPSEUDO100		94
#define BATTERYPSEUDO1			2
#endif

#ifndef VENDOR_EDIT /* OPPO 2016-03-18 sjc Delete for charging */
#define Q_MAX_BY_SYS			/*8. Qmax varient by system drop voltage.*/
#endif
#define Q_MAX_SYS_VOLTAGE		3350
#define SHUTDOWN_GAUGE0
#define SHUTDOWN_GAUGE1_XMINS
#define SHUTDOWN_GAUGE1_MINS		60

#define SHUTDOWN_SYSTEM_VOLTAGE		3400
#define CHARGE_TRACKING_TIME		60
#define DISCHARGE_TRACKING_TIME		10

#define RECHARGE_TOLERANCE		10
/* SW Fuel Gauge */
#define MAX_HWOCV			5
#define MAX_VBAT			90

/* fg 1.0 */
#define CUST_POWERON_DELTA_CAPACITY_TOLRANCE	40
#define CUST_POWERON_LOW_CAPACITY_TOLRANCE		5
#define CUST_POWERON_MAX_VBAT_TOLRANCE			90
#define CUST_POWERON_DELTA_VBAT_TOLRANCE		30
#define CUST_POWERON_DELTA_HW_SW_OCV_CAPACITY_TOLRANCE	10


/* Disable Battery check for HQA */
#ifdef CONFIG_MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
#define FIXED_TBAT_25
#endif

/* Dynamic change wake up period of battery thread when suspend*/
#define VBAT_NORMAL_WAKEUP	3600	/*3.6V*/
#define VBAT_LOW_POWER_WAKEUP	3500	/*3.5v*/
#define NORMAL_WAKEUP_PERIOD	5400	/*90 * 60 = 90 min*/
#define LOW_POWER_WAKEUP_PERIOD	300	/*5 * 60 = 5 min*/
#define CLOSE_POWEROFF_WAKEUP_PERIOD	30	/*30 s*/

#define INIT_SOC_BY_SW_SOC

/*3. UI SOC sync to FG SOC immediately*/
/*#define SYNC_UI_SOC_IMM*/

/*6. Q_MAX aging algorithm*/
#define MTK_ENABLE_AGING_ALGORITHM

/*5. Gauge Adjust by OCV 9. MD sleep current check*/
#define MD_SLEEP_CURRENT_CHECK

/*7. Qmax varient by current loading.*/
/*#define Q_MAX_BY_CURRENT*/

#define FG_BAT_INT
#define IS_BATTERY_REMOVE_BY_PMIC

/* Calculate do in Kernel */
#define FORCE_D0_IN_KERNEL

/* Use UI_SOC3 to smooth UI_SOC2 */
#define USING_SMOOTH_UI_SOC2

/* SOC track to SWOCV */
#define CUST_TRACKING_GAP		15	/* start tracking gap */
#define CUST_TRACKINGOFFSET		0	/* Force offset to shift SOC to 0 */
#define CUST_TRACKINGEN			0	/* 0:disable, 1:enable */

#ifdef VENDOR_EDIT /* OPPO 2016-03-24 sjc Add for charging */
#define CONFIG_MTK_EMBEDDED_BATTERY
#endif /* VENDOR_EDIT */
#endif
