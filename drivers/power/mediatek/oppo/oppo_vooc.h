/**********************************************************************************
* Copyright (c)  2008-2015  Guangdong OPPO Mobile Comm Corp., Ltd
* VENDOR_EDIT
* Description: Charger IC management module for charger system framework.
*              Manage all charger IC and define abstarct function flow.
* Version   : 1.0
* Date      : 2015-05-22
* Author    : fanhui@PhoneSW.BSP
* 			: Fanhong.Kong@ProDrv.CHG		   	
* ------------------------------ Revision History: --------------------------------
* <version>       <date>        <author>              			<desc>
* Revision 1.0    2015-05-22    fanhui@PhoneSW.BSP    			Created for new architecture
* Revision 1.0    2015-05-22    Fanhong.Kong@ProDrv.CHG   		Created for new architecture
***********************************************************************************/

#ifndef _OPPO_VOOC_H_
#define _OPPO_VOOC_H_

#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/timer.h>

enum {
	VOOC_CHARGER_MODE,
	HEADPHONE_MODE,
	NORMAL_CHARGER_MODE,
};

enum {
	ADAPTER_FW_UPDATE_NONE,
	ADAPTER_FW_NEED_UPDATE,
	ADAPTER_FW_UPDATE_SUCCESS,
	ADAPTER_FW_UPDATE_FAIL,
};

struct vooc_gpio_control {
	int								switch1_gpio;
	int								switch2_gpio;
	int								switch3_gpio;
	int								reset_gpio;
	int								clock_gpio;
	int								data_gpio;
	int                             data_irq;

	struct pinctrl 					*pinctrl;
	struct pinctrl_state 			*gpio_switch1_act_switch2_act;
	struct pinctrl_state 			*gpio_switch1_sleep_switch2_sleep;
	struct pinctrl_state 			*gpio_switch1_act_switch2_sleep;
	struct pinctrl_state 			*gpio_switch1_sleep_switch2_act;
	
	struct pinctrl_state 			*gpio_clock_active;
	struct pinctrl_state 			*gpio_clock_sleep;
	struct pinctrl_state 			*gpio_data_active;
	struct pinctrl_state 			*gpio_data_sleep;
	struct pinctrl_state 			*gpio_reset_active;
	struct pinctrl_state 			*gpio_reset_sleep;
};

struct oppo_vooc_chip {
	struct i2c_client		*client;
	struct device       *dev;
	struct oppo_vooc_operations	*vops;
	struct vooc_gpio_control	vooc_gpio;
	struct delayed_work		fw_update_work;
	struct delayed_work		adapter_update_work;
	struct delayed_work		fastchg_work;
	struct delayed_work		delay_reset_mcu_work;
	struct delayed_work		check_adapter_out_work;
	struct timer_list		watchdog;
	struct wake_lock		vooc_wake_lock;
	struct power_supply		*batt_psy;
	int		pcb_version;
	bool	allow_reading;
	bool	fastchg_started;
	bool	fastchg_ing;
	bool	fastchg_allow;
	bool	fastchg_to_normal;
	bool	fastchg_to_warm;
	bool	fastchg_low_temp_full;
	bool	btb_temp_over;
	bool 	fastchg_dummy_started;

	bool	need_to_up;
	bool	have_updated;
	bool	mcu_update_ing;
	bool 	mcu_boot_by_gpio;
	unsigned char	*firmware_data;
	unsigned int	fw_data_count;
	int		fw_mcu_version;
	int		fw_data_version;
	int		adapter_update_real;
	int		adapter_update_report;
	int		dpdm_switch_mode;
// wenbin.liu@BSP.CHG.Vooc, 2016/10/20
// Add for vooc batt 4.40
	bool	batt_type_4400mv;
	bool vooc_fw_check;
	int		vooc_fw_type;
};

struct oppo_vooc_operations {
	int (*fw_update) (struct oppo_vooc_chip *chip);
	void (*fw_check_then_recover) (struct oppo_vooc_chip *chip);
	void (*eint_regist)(struct oppo_vooc_chip *chip);
	void (*eint_unregist)(struct oppo_vooc_chip *chip);
	void (*set_data_active)(struct oppo_vooc_chip *chip);
	void (*set_data_sleep)(struct oppo_vooc_chip *chip);
	void (*set_clock_active)(struct oppo_vooc_chip *chip);
	void (*set_clock_sleep)(struct oppo_vooc_chip *chip);
	void (*set_switch_mode) (struct oppo_vooc_chip *chip, int mode);
	int (*get_gpio_ap_data)(struct oppo_vooc_chip *chip);
	int (*read_ap_data) (struct oppo_vooc_chip *chip);
	void (*reply_mcu_data) (struct oppo_vooc_chip *chip, int ret_info,int device_type);
	void (*reset_fastchg_after_usbout)(struct oppo_vooc_chip *chip);
	void (*switch_fast_chg)(struct oppo_vooc_chip *chip);
	void (*reset_mcu)(struct oppo_vooc_chip *chip);
	bool (*is_power_off_charging)(struct oppo_vooc_chip *chip);
	int (*get_reset_gpio_val)(struct oppo_vooc_chip *chip);
	int (*get_switch_gpio_val)(struct oppo_vooc_chip *chip);
	int (*get_ap_clk_gpio_val)(struct oppo_vooc_chip *chip);
};

struct oppo_adapter_chip {
	int		timer_delay;
	bool	tx_byte_over;
	bool	rx_byte_over;
	bool	rx_timeout;
	unsigned long uart_tx_gpio;
	unsigned long uart_rx_gpio;
	char	*adapter_firmware_data;
	unsigned int	adapter_fw_data_count;
	bool	adapter_update_ing;
	struct oppo_adapter_operations	*vops;
};

struct oppo_adapter_operations {
	bool (*adapter_update) (struct oppo_adapter_chip *chip, 
			unsigned long tx_pin, unsigned long rx_pin);
};

void oppo_vooc_init(struct oppo_vooc_chip *chip);
void oppo_vooc_shedule_fastchg_work(void);


void oppo_vooc_fw_update_work_init(struct oppo_vooc_chip *chip);

bool oppo_vooc_wake_fastchg_work(struct oppo_vooc_chip *chip);
void oppo_vooc_print_log(void);


void oppo_vooc_switch_mode(int mode);

bool oppo_vooc_get_allow_reading(void);

bool oppo_vooc_get_fastchg_started(void);

bool oppo_vooc_get_fastchg_ing(void);

bool oppo_vooc_get_fastchg_allow(void);

void oppo_vooc_set_fastchg_allow(int enable);
bool oppo_vooc_get_fastchg_to_normal(void);

void oppo_vooc_set_fastchg_to_normal_false(void);

bool oppo_vooc_get_fastchg_to_warm(void);

void oppo_vooc_set_fastchg_to_warm_false(void);

bool oppo_vooc_get_fastchg_low_temp_full(void);

void oppo_vooc_set_fastchg_low_temp_full_false(void);
bool oppo_vooc_get_fastchg_dummy_started(void);
void oppo_vooc_set_fastchg_dummy_started_false(void);
int oppo_vooc_get_adapter_update_status(void);
int oppo_vooc_get_adapter_update_real_status(void);
bool oppo_vooc_get_btb_temp_over(void);
void oppo_vooc_reset_fastchg_after_usbout(void);
void oppo_vooc_switch_fast_chg(void);
void oppo_vooc_reset_mcu(void);
void oppo_vooc_set_ap_clk_high(void);
int oppo_vooc_get_vooc_switch_val(void);
bool oppo_vooc_check_chip_is_null(void);



#endif /* _OPPO_VOOC_H */
