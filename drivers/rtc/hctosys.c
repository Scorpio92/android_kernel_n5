/*
 * RTC subsystem, initialize system time on startup
 *
 * Copyright (C) 2005 Tower Technologies
 * Author: Alessandro Zummo <a.zummo@towertech.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,start.
	/*
	 *2010-11-08 zhengchao fix the alarm sync bug.This fix only worked on EVDO tag=CONFIG_ZTE_FIX_ALARM_SYNC
	 *
	 */
//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,end.

#include <linux/rtc.h>
/*[ECID:000000] ZTEBSP zhangbo, sync time with modem, start*/
#ifdef CONFIG_ZTE_TIME_SYNC_WITH_MODEM
#include <linux/workqueue.h>
#endif
/*[ECID:000000] ZTEBSP zhangbo, sync time with modem, end*/

/* IMPORTANT: the RTC only stores whole seconds. It is arbitrary
 * whether it stores the most close value or the value with partial
 * seconds truncated. However, it is important that we use it to store
 * the truncated value. This is because otherwise it is necessary,
 * in an rtc sync function, to read both xtime.tv_sec and
 * xtime.tv_nsec. On some processors (i.e. ARM), an atomic read
 * of >32bits is not possible. So storing the most close value would
 * slow down the sync API. So here we have the truncated value and
 * the best guess is to add 0.5s.
 */
 
//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,start.
#ifdef CONFIG_ZTE_FIX_ALARM_SYNC
extern void fix_sync_alarm(void);
#endif
//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,end.

int rtc_hctosys_ret = -ENODEV;

/*[ECID:000000] ZTEBSP zhangbo, sync time with modem, start*/
#ifdef CONFIG_ZTE_TIME_SYNC_WITH_MODEM
struct delayed_work		time_sync_work;

void  modem_time_sync(struct work_struct *work)
{
	int err = -ENODEV;
	struct rtc_time tm;
	struct timespec tv = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};
	struct timeval current_time;
	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);

	if (rtc == NULL) {
		pr_err("%s: unable to open rtc device (%s)\n",
			__FILE__, CONFIG_RTC_HCTOSYS_DEVICE);
		goto err_open;
	}

	do_gettimeofday(&current_time);

	err = rtc_read_time(rtc, &tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: unable to read the hardware clock\n");
		goto err_read;

	}

	err = rtc_valid_tm(&tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: invalid date/time\n");
		goto err_invalid;
	}

	rtc_tm_to_time(&tm, &tv.tv_sec);

	if((current_time.tv_sec - tv.tv_sec > 30) || (current_time.tv_sec - tv.tv_sec < -30) ) {
		do_settimeofday(&tv);
	}


err_invalid:
err_read:
	rtc_class_close(rtc);

err_open:
	rtc_hctosys_ret = err;

	schedule_delayed_work(&time_sync_work, HZ*30);
}

int modem_time_sync_init(void)
{
	static bool first = true;
	if(first) {
		INIT_DELAYED_WORK(&time_sync_work, modem_time_sync);
		schedule_delayed_work(&time_sync_work, HZ*30);
		first = false;
	}
	return 0;
}
#endif
/*[ECID:000000] ZTEBSP zhangbo, sync time with modem, end*/


int rtc_hctosys(void)
{
	int err = -ENODEV;
	struct rtc_time tm;
	struct timespec tv = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};
	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);

	/*[ECID:000000] ZTEBSP zhangbo, sync time with modem, start*/
	#ifdef CONFIG_ZTE_TIME_SYNC_WITH_MODEM
	modem_time_sync_init();
	#endif
	/*[ECID:000000] ZTEBSP zhangbo, sync time with modem, end*/
	
	if (rtc == NULL) {
		pr_err("%s: unable to open rtc device (%s)\n",
			__FILE__, CONFIG_RTC_HCTOSYS_DEVICE);
		goto err_open;
	}

	err = rtc_read_time(rtc, &tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: unable to read the hardware clock\n");
		goto err_read;

	}

	err = rtc_valid_tm(&tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: invalid date/time\n");
		goto err_invalid;
	}

	rtc_tm_to_time(&tm, &tv.tv_sec);

	do_settimeofday(&tv);
	//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,start.
	#ifdef CONFIG_ZTE_FIX_ALARM_SYNC
		fix_sync_alarm();
	#endif
	//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,end.
	
	dev_info(rtc->dev.parent,
		"setting system clock to "
		"%d-%02d-%02d %02d:%02d:%02d UTC (%u)\n",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec,
		(unsigned int) tv.tv_sec);

err_invalid:
err_read:
	rtc_class_close(rtc);

err_open:
	rtc_hctosys_ret = err;

	return err;
}

late_initcall(rtc_hctosys);
