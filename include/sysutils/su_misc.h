/*
 * Misc utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SU_MISC_H__
#define __SU_MISC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * Sysutils header file of other functions
 */

/**
 * @defgroup Sysutils_Misc
 * @ingroup sysutils
 * @brief other functions.
 * @{
 */

/**
 * key event.
 */
typedef enum {
	KEY_RELEASED,	/**< key release*/
	KEY_PRESSED,	/**< key press*/
} SUKeyEvent;

/**
 * LED behavior command.
 */
typedef enum {
	LED_OFF,		/**< Turn on LED */
	LED_ON,			/**< Turn off LED */
} SULedCmd;

/**
 * @fn int SU_Key_OpenEvent(void)
 *
 * Obtain the key event handle.
 *
 * @param none
 *
 * @retval >0 key event handle.
 * @retval <=0 failed.
 *
 * @remarks After successfully obtaining a key event handle, it begins to "log" the key event until the key event is closed
 * @remarks If more than one handle is opened, a copy of each keystroke event is recorded.
 * @remarks For example, if two threads each open a key press event, each holding a handle, both threads will read the same sequence of events.
 * But if two threads share the same handle, each keystroke event can be read only once.
 *
 * @attention none
 */
int SU_Key_OpenEvent(void);

/**
 * @fn int SU_Key_CloseEvent(int evfd)
 *
 * Turn off key event.
 *
 * @param[in] evfd key event handle
 *
 * @retval 0 success.
 * @retval non-0 failed
 *
 * @remarks none
 *
 * @attention none
 */
int SU_Key_CloseEvent(int evfd);

/**
 * @fn int SU_Key_ReadEvent(int evfd, int *keyCode, SUKeyEvent *event)
 *
 * Read key event.
 *
 * @param[in] evfd key event handle
 * @param[in] keyCode keycode
 * @param[out] event ket event pointer.
 *
 * @retval 0 success.
 * @retval non-0 failed.
 *
 * @remarks The function blocks until a keystroke event returns.
 * @remarks Keycodes are defined in linux/input.h, and mappings to GPIOs are defined in kernel board files.
 * @remarks such as several commonly used keys:
 * @code
	#define KEY_HOME                102 //HOME key
	#define KEY_POWER               116 //Power key, It can also be used as a wake-up key
	#define KEY_WAKEUP              143 //Wake-up key, Keys used to wake up the system other than POWER key
	#define KEY_F13                 183 //When PIR is used as a key, it is defined as the F13 key
 * @endcode
 *
 * @remarks The definition of key code and GPIO number, whether it is used as a wake-up source, and the effective power level information are defined in the core board-level file, as follows:
 *
 * @code
	struct gpio_keys_button __attribute__((weak)) board_buttons[] = {
	#ifdef GPIO_HOME
		{
			.gpio           = GPIO_HOME,		//Define GPIO num
			.code           = KEY_HOME,			//Define key code
			.desc           = "home key",
			.active_low     = ACTIVE_LOW_HOME,	//Define active level
	#ifdef WAKEUP_HOME
			.wakeup         = WAKEUP_HOME,		//Define whether can be used as wake-up source, 1 means be able to wake-up suspend
	#endif
	#ifdef CAN_DISABLE_HOME
			.can_disable    = CAN_DISABLE_HOME,	//Define whether be able to Disabled
	#endif
		},
	#endif
	#ifdef GPIO_POWER
		{
			.gpio           = GPIO_POWER,
			.code           = KEY_POWER,
			.desc           = "power key",
			.active_low     = ACTIVE_LOW_POWER,
	#ifdef WAKEUP_POWER
			.wakeup         = WAKEUP_POWER,
	#endif
	#ifdef CAN_DISABLE_POWER
			.can_disable    = CAN_DISABLE_POWER,
	#endif
		},
	#endif
	}
 * @endcode
 * @remarks For digital PIR, one way to use PIR is to define PIR as a key, and PIR triggers the equivalent of a key press event (@ref KEY_PRESSED),
* PIR recovery is equivalent to a key lift event (@ref KEY_RELEASED). If the PIR wake-up function is required, the button corresponding to the PIR can be defined as the wake-up source.
 * @remarks For details on how to use the API, please refer to sample-keyevent.c.
 *
 * @attention none
 */
int SU_Key_ReadEvent(int evfd, int *keyCode, SUKeyEvent *event);

/**
 * @fn int SU_Key_DisableEvent(int keyCode)
 *
 * Disable key event.
 *
 * @param[in] keyCode 
 *
 * @retval 0 success
 * @retval non-0 failed.
 *
 * @remarks If a key key is configured as a wake-up source, pressing the key (whether the key is open or not) wakes the system when the system suspends.
* After the Disable key event, the system turns off the interruption of the key press event, and the key cannot wake the system
 * @remarks This API can be used to disable PIR "keystrokes" to wake up the system.
 *
 * @attention none
 */
int SU_Key_DisableEvent(int keyCode);

/**
 * @fn int SU_Key_EnableEvent(int keyCode)
 *
 * Enable key event.
 *
 * @param[in] keyCode 
 *
 * @retval 0 success
 * @retval non-0 failed.
 *
 * @remarks The reverse process as a disable keystroke event, please refer to@ref SU_Key_DisableEvent(int keyCode)
 *
 * @attention none
 */
int SU_Key_EnableEvent(int keyCode);

/**
 * @fn int SU_LED_Command(int ledNum, SULedCmd cmd)
 *
 * Send LED command.
 *
 * @param[in] ledNum LED number.
 * @param[in] cmd LED behaior command.
 *
 * @retval 0 success.
 * @retval non-0 failed.
 *
 * @remarksThe LED number varies depending on the development board. The LED number is defined in the kernel board-level file and registered as a Linux standard
* Quasi-Fixed Regulator equipment. In the board-level file, the GPIO number, active level, power recursion relationship, etc. of the LED need to be defined
*Information. Here is an example of defining two LED fixed regulators:
 * @code
    FIXED_REGULATOR_DEF(  // define fixed regulator
            led0,
            "LED0",         3300000,        GPIO_PA(14),
            HIGH_ENABLE,    UN_AT_BOOT,     0,
            "ldo7",         "vled0",        NULL);

    FIXED_REGULATOR_DEF(
            led1,
            "LED1",         3300000,        GPIO_PA(15),
            HIGH_ENABLE,    UN_AT_BOOT,     0,
            "ldo7",         "vled1",        NULL);

    static struct platform_device *fixed_regulator_devices[] __initdata = {
            &gsensor_regulator_device,
            &led0_regulator_device,
            &led1_regulator_device,
    };

    static int __init fix_regulator_init(void)  //register regulator in subsys_initcall_sync
    {
            int i;

            for (i = 0; i < ARRAY_SIZE(fixed_regulator_devices); i++)
                    fixed_regulator_devices[i]->id = i;

            return platform_add_devices(fixed_regulator_devices,
                                        ARRAY_SIZE(fixed_regulator_devices));
    }
    subsys_initcall_sync(fix_regulator_init);
 * @endcode
 * @remarks Examples of the use of this API
 * @code
   if (SU_LED_Command(0, LED_ON) < 0)  //enable LED0
       printf("LED0 turn on error\n");
   if (SU_LED_Command(1, LED_ON) < 0)  //enable LED1
       printf("LED0 turn on error\n");
   if (SU_LED_Command(0, LED_OFF) < 0)  //disable LED0
       printf("LED1 turn off error\n");
   if (SU_LED_Command(1, LED_OFF) < 0)  //disable LED1
       printf("LED1 turn off error\n");
 * @endcode
 * @attention none
 */
int SU_LED_Command(int ledNum, SULedCmd cmd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SU_MISC_H__ */
