/* Copyright (c) 2010-2011, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtc.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include<linux/spinlock.h>

#include <linux/mfd/pm8xxx/core.h>
#include <linux/mfd/pm8xxx/rtc.h>


/* RTC Register offsets from RTC CTRL REG */
#define PM8XXX_ALARM_CTRL_OFFSET 0x01
#define PM8XXX_RTC_WRITE_OFFSET 0x02
#define PM8XXX_RTC_READ_OFFSET 0x06
#define PM8XXX_ALARM_RW_OFFSET 0x0A

/* RTC_CTRL register bit fields */
#define PM8xxx_RTC_ENABLE	BIT(7)
#define PM8xxx_RTC_ALARM_ENABLE	BIT(1)
#define PM8xxx_RTC_ABORT_ENABLE BIT(0)

#define PM8xxx_RTC_ALARM_CLEAR  BIT(0)

#define NUM_8_BIT_RTC_REGS	0x4

/**
 * struct pm8xxx_rtc - rtc driver internal structure
 * @rtc: rtc device for this driver
 * @rtc_alarm_irq: rtc alarm irq number
 */
struct pm8xxx_rtc {
	struct rtc_device *rtc;
	int rtc_alarm_irq;
	int rtc_base;
	int rtc_read_base;
	int rtc_write_base;
	int alarm_rw_base;
	u8  ctrl_reg;
	struct device *rtc_dev;
	spinlock_t ctrl_reg_lock;
};

/*
 * The RTC registers need to be read/written one byte at a time. This is a
 * hardware limitation.
 */

static int pm8xxx_read_wrapper(struct pm8xxx_rtc *rtc_dd, u8 *rtc_val,
			int base, int count)
{
	int i, rc;
	struct device *parent = rtc_dd->rtc_dev->parent;

	for (i = 0; i < count; i++) {
		rc = pm8xxx_readb(parent, base + i, &rtc_val[i]);
		if (rc < 0) {
			dev_err(rtc_dd->rtc_dev, "PM8xxx read failed\n");
			return rc;
		}
	}

	return 0;
}

static int pm8xxx_write_wrapper(struct pm8xxx_rtc *rtc_dd, u8 *rtc_val,
			int base, int count)
{
	int i, rc;
	struct device *parent = rtc_dd->rtc_dev->parent;

	for (i = 0; i < count; i++) {
		rc = pm8xxx_writeb(parent, base + i, rtc_val[i]);
		if (rc < 0) {
			dev_err(rtc_dd->rtc_dev, "PM8xxx write failed\n");
			return rc;
		}
	}

	return 0;
}


/*
 * Steps to write the RTC registers.
 * 1. Disable alarm if enabled.
 * 2. Write 0x00 to LSB.
 * 3. Write Byte[1], Byte[2], Byte[3] then Byte[0].
 * 4. Enable alarm if disabled in step 1.
 */
static int
pm8xxx_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	int rc;
	unsigned long secs, irq_flags;
	u8 value[4], reg = 0, alarm_enabled = 0, ctrl_reg;
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);

	rtc_tm_to_time(tm, &secs);

	value[0] = secs & 0xFF;
	value[1] = (secs >> 8) & 0xFF;
	value[2] = (secs >> 16) & 0xFF;
	value[3] = (secs >> 24) & 0xFF;

	dev_dbg(dev, "Seconds value to be written to RTC = %lu\n", secs);

	spin_lock_irqsave(&rtc_dd->ctrl_reg_lock, irq_flags);
	ctrl_reg = rtc_dd->ctrl_reg;

	if (ctrl_reg & PM8xxx_RTC_ALARM_ENABLE) {
		alarm_enabled = 1;
		ctrl_reg &= ~PM8xxx_RTC_ALARM_ENABLE;
		rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base,
									1);
		if (rc < 0) {
			dev_err(dev, "PM8xxx write failed\n");
			goto rtc_rw_fail;
		}
	} else
		spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);

	/* Write Byte[1], Byte[2], Byte[3], Byte[0] */
	/* Write 0 to Byte[0] */
	reg = 0;
	rc = pm8xxx_write_wrapper(rtc_dd, &reg, rtc_dd->rtc_write_base, 1);
	if (rc < 0) {
		dev_err(dev, "PM8xxx write failed\n");
		goto rtc_rw_fail;
	}

	/* Write Byte[1], Byte[2], Byte[3] */
	rc = pm8xxx_write_wrapper(rtc_dd, value + 1,
					rtc_dd->rtc_write_base + 1, 3);
	if (rc < 0) {
		dev_err(dev, "Write to RTC registers failed\n");
		goto rtc_rw_fail;
	}

	/* Write Byte[0] */
	rc = pm8xxx_write_wrapper(rtc_dd, value, rtc_dd->rtc_write_base, 1);
	if (rc < 0) {
		dev_err(dev, "Write to RTC register failed\n");
		goto rtc_rw_fail;
	}

	if (alarm_enabled) {
		ctrl_reg |= PM8xxx_RTC_ALARM_ENABLE;
		rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base,
									1);
		if (rc < 0) {
			dev_err(dev, "PM8xxx write failed\n");
			goto rtc_rw_fail;
		}
	}

	rtc_dd->ctrl_reg = ctrl_reg;

rtc_rw_fail:
	if (alarm_enabled)
		spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);

	return rc;
}

static int
pm8xxx_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	int rc;
	u8 value[4], reg;
	unsigned long secs;
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);

	rc = pm8xxx_read_wrapper(rtc_dd, value, rtc_dd->rtc_read_base,
							NUM_8_BIT_RTC_REGS);
	if (rc < 0) {
		dev_err(dev, "RTC time read failed\n");
		return rc;
	}

	/*
	 * Read the LSB again and check if there has been a carry over.
	 * If there is, redo the read operation.
	 */
	rc = pm8xxx_read_wrapper(rtc_dd, &reg, rtc_dd->rtc_read_base, 1);
	if (rc < 0) {
		dev_err(dev, "PM8xxx read failed\n");
		return rc;
	}

	if (unlikely(reg < value[0])) {
		rc = pm8xxx_read_wrapper(rtc_dd, value,
				rtc_dd->rtc_read_base, NUM_8_BIT_RTC_REGS);
		if (rc < 0) {
			dev_err(dev, "RTC time read failed\n");
			return rc;
		}
	}

	secs = value[0] | (value[1] << 8) | (value[2] << 16) \
						| (value[3] << 24);

	rtc_time_to_tm(secs, tm);

	rc = rtc_valid_tm(tm);
	if (rc < 0) {
		dev_err(dev, "Invalid time read from PM8xxx\n");
		return rc;
	}

	dev_dbg(dev, "secs = %lu, h:m:s == %d:%d:%d, d/m/y = %d/%d/%d\n",
			secs, tm->tm_hour, tm->tm_min, tm->tm_sec,
			tm->tm_mday, tm->tm_mon, tm->tm_year);

	return 0;
}

static int
pm8xxx_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	int rc;
	u8 value[4], ctrl_reg;
	unsigned long secs, secs_rtc, irq_flags;
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);
	struct rtc_time rtc_tm;

	rtc_tm_to_time(&alarm->time, &secs);

	/*
	 * Read the current RTC time and verify if the alarm time is in the
	 * past. If yes, return invalid.
	 */
	rc = pm8xxx_rtc_read_time(dev, &rtc_tm);
	if (rc < 0) {
		dev_err(dev, "Unamble to read RTC time\n");
		return -EINVAL;
	}

	rtc_tm_to_time(&rtc_tm, &secs_rtc);
	if (secs < secs_rtc) {
		dev_err(dev, "Trying to set alarm in the past\n");
		return -EINVAL;
	}

	value[0] = secs & 0xFF;
	value[1] = (secs >> 8) & 0xFF;
	value[2] = (secs >> 16) & 0xFF;
	value[3] = (secs >> 24) & 0xFF;

	spin_lock_irqsave(&rtc_dd->ctrl_reg_lock, irq_flags);

	rc = pm8xxx_write_wrapper(rtc_dd, value, rtc_dd->alarm_rw_base,
							NUM_8_BIT_RTC_REGS);
	if (rc < 0) {
		dev_err(dev, "Write to RTC ALARM registers failed\n");
		goto rtc_rw_fail;
	}

	ctrl_reg = rtc_dd->ctrl_reg;
	ctrl_reg = (alarm->enabled) ? (ctrl_reg | PM8xxx_RTC_ALARM_ENABLE) :
					(ctrl_reg & ~PM8xxx_RTC_ALARM_ENABLE);

	rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base, 1);
	if (rc < 0) {
		dev_err(dev, "PM8xxx write failed\n");
		goto rtc_rw_fail;
	}

	rtc_dd->ctrl_reg = ctrl_reg;

	dev_dbg(dev, "Alarm Set for h:r:s=%d:%d:%d, d/m/y=%d/%d/%d\n",
			alarm->time.tm_hour, alarm->time.tm_min,
			alarm->time.tm_sec, alarm->time.tm_mday,
			alarm->time.tm_mon, alarm->time.tm_year);
rtc_rw_fail:
	spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);
	return rc;
}

static int
pm8xxx_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	int rc;
	u8 value[4];
	unsigned long secs;
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);

	rc = pm8xxx_read_wrapper(rtc_dd, value, rtc_dd->alarm_rw_base,
			NUM_8_BIT_RTC_REGS);
	if (rc < 0) {
		dev_err(dev, "RTC alarm time read failed\n");
		return rc;
	}

	secs = value[0] | (value[1] << 8) | (value[2] << 16) | \
						 (value[3] << 24);

	rtc_time_to_tm(secs, &alarm->time);

	rc = rtc_valid_tm(&alarm->time);
	if (rc < 0) {
		dev_err(dev, "Invalid time read from PM8xxx\n");
		return rc;
	}

	dev_dbg(dev, "Alarm set for - h:r:s=%d:%d:%d, d/m/y=%d/%d/%d\n",
		alarm->time.tm_hour, alarm->time.tm_min,
				alarm->time.tm_sec, alarm->time.tm_mday,
				alarm->time.tm_mon, alarm->time.tm_year);

	return 0;
}


//[ECID:0000]ZTE_BSP maxiaoping 20130123 modify PLATFORM 8064 RTC alarm driver,start.
/*Solve the alarm can't responsed during sleep,the reason is when calling alarm_suspend()->rtc_set_alarm()
,in second time,rtc->aie_timer.enabled will be true,then it will call rtc_timer_remove()->rtc_alarm_disable()
disable the rtc alarm irq,so the alarm is disablede.
*/
static int
pm8xxx_rtc_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
	int rc;
	unsigned long irq_flags;
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);
	u8 ctrl_reg;

	spin_lock_irqsave(&rtc_dd->ctrl_reg_lock, irq_flags);
	ctrl_reg = rtc_dd->ctrl_reg;
	ctrl_reg = (enabled) ? (ctrl_reg | PM8xxx_RTC_ALARM_ENABLE) :
				(ctrl_reg & ~PM8xxx_RTC_ALARM_ENABLE);
				
	//[ECID:0000]ZTE_BSP maxiaoping 20130123 modify PLATFORM 8064 RTC alarm driver,start.
	/*
	We don't want the wake up timer to control the rtc alarm enable/disable irq,so here we don't write the ctrl_reg
	to the pmic register. 
	*/
	//rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base, 1);//QCT default enable.
	rc = 0;
	//[ECID:0000]ZTE_BSP maxiaoping 20130123 modify PLATFORM 8064 RTC alarm driver,end.
	
	if (rc < 0) {
		dev_err(dev, "PM8xxx write failed\n");
		goto rtc_rw_fail;
	}

	rtc_dd->ctrl_reg = ctrl_reg;

rtc_rw_fail:
	spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);
	return rc;
}
//[ECID:0000]ZTE_BSP maxiaoping 20130123 modify PLATFORM 8064 RTC alarm driver,end.

static struct rtc_class_ops pm8xxx_rtc_ops = {
	.read_time	= pm8xxx_rtc_read_time,
	.set_alarm	= pm8xxx_rtc_set_alarm,
	.read_alarm	= pm8xxx_rtc_read_alarm,
	.alarm_irq_enable = pm8xxx_rtc_alarm_irq_enable,
};

static irqreturn_t pm8xxx_alarm_trigger(int irq, void *dev_id)
{
	struct pm8xxx_rtc *rtc_dd = dev_id;
	u8 ctrl_reg;
	int rc;
	unsigned long irq_flags;

	rtc_update_irq(rtc_dd->rtc, 1, RTC_IRQF | RTC_AF);

	spin_lock_irqsave(&rtc_dd->ctrl_reg_lock, irq_flags);

	/* Clear the alarm enable bit */
	ctrl_reg = rtc_dd->ctrl_reg;
	ctrl_reg &= ~PM8xxx_RTC_ALARM_ENABLE;

	rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base, 1);
	if (rc < 0) {
		spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);
		dev_err(rtc_dd->rtc_dev, "PM8xxx write failed!\n");
		goto rtc_alarm_handled;
	}

	rtc_dd->ctrl_reg = ctrl_reg;
	spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);

	/* Clear RTC alarm register */
	rc = pm8xxx_read_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base +
						PM8XXX_ALARM_CTRL_OFFSET, 1);
	if (rc < 0) {
		dev_err(rtc_dd->rtc_dev, "PM8xxx write failed!\n");
		goto rtc_alarm_handled;
	}

	ctrl_reg &= ~PM8xxx_RTC_ALARM_CLEAR;
	rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base +
						PM8XXX_ALARM_CTRL_OFFSET, 1);
	if (rc < 0)
		dev_err(rtc_dd->rtc_dev, "PM8xxx write failed!\n");

rtc_alarm_handled:
	return IRQ_HANDLED;
}

static int __devinit pm8xxx_rtc_probe(struct platform_device *pdev)
{
	int rc;
	u8 ctrl_reg;
	bool rtc_write_enable = false;
	struct pm8xxx_rtc *rtc_dd;
	struct resource *rtc_resource;
	const struct pm8xxx_rtc_platform_data *pdata =
		pdev->dev.platform_data;

	if (pdata != NULL)
		rtc_write_enable = pdata->rtc_write_enable;

	rtc_dd = kzalloc(sizeof(*rtc_dd), GFP_KERNEL);
	if (rtc_dd == NULL) {
		dev_err(&pdev->dev, "Unable to allocate memory!\n");
		return -ENOMEM;
	}

	/* Initialise spinlock to protect RTC cntrol register */
	spin_lock_init(&rtc_dd->ctrl_reg_lock);

	rtc_dd->rtc_alarm_irq = platform_get_irq(pdev, 0);
	if (rtc_dd->rtc_alarm_irq < 0) {
		dev_err(&pdev->dev, "Alarm IRQ resource absent!\n");
		rc = -ENXIO;
		goto fail_rtc_enable;
	}

	rtc_resource = platform_get_resource_byname(pdev, IORESOURCE_IO,
							"pmic_rtc_base");
	if (!(rtc_resource && rtc_resource->start)) {
		dev_err(&pdev->dev, "RTC IO resource absent!\n");
		rc = -ENXIO;
		goto fail_rtc_enable;
	}

	rtc_dd->rtc_base = rtc_resource->start;

	/* Setup RTC register addresses */
	rtc_dd->rtc_write_base = rtc_dd->rtc_base + PM8XXX_RTC_WRITE_OFFSET;
	rtc_dd->rtc_read_base = rtc_dd->rtc_base + PM8XXX_RTC_READ_OFFSET;
	rtc_dd->alarm_rw_base = rtc_dd->rtc_base + PM8XXX_ALARM_RW_OFFSET;

	rtc_dd->rtc_dev = &(pdev->dev);

	/* Check if the RTC is on, else turn it on */
	rc = pm8xxx_read_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base, 1);
	if (rc < 0) {
		dev_err(&pdev->dev, "PM8xxx read failed!\n");
		goto fail_rtc_enable;
	}

	if (!(ctrl_reg & PM8xxx_RTC_ENABLE)) {
		ctrl_reg |= PM8xxx_RTC_ENABLE;
		rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base,
									1);
		if (rc < 0) {
			dev_err(&pdev->dev, "PM8xxx write failed!\n");
			goto fail_rtc_enable;
		}
	}

	/* Enable abort enable feature */
	ctrl_reg |= PM8xxx_RTC_ABORT_ENABLE;
	rc = pm8xxx_write_wrapper(rtc_dd, &ctrl_reg, rtc_dd->rtc_base, 1);
	if (rc < 0) {
		dev_err(&pdev->dev, "PM8xxx write failed!\n");
		goto fail_rtc_enable;
	}

	rtc_dd->ctrl_reg = ctrl_reg;
	if (rtc_write_enable == true)
		pm8xxx_rtc_ops.set_time = pm8xxx_rtc_set_time;

	platform_set_drvdata(pdev, rtc_dd);

	/* Register the RTC device */
	rtc_dd->rtc = rtc_device_register("pm8xxx_rtc", &pdev->dev,
				&pm8xxx_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc_dd->rtc)) {
		dev_err(&pdev->dev, "%s: RTC registration failed (%ld)\n",
					__func__, PTR_ERR(rtc_dd->rtc));
		rc = PTR_ERR(rtc_dd->rtc);
		goto fail_rtc_enable;
	}

	/* Request the alarm IRQ */
	rc = request_any_context_irq(rtc_dd->rtc_alarm_irq,
				 pm8xxx_alarm_trigger, IRQF_TRIGGER_RISING,
				 "pm8xxx_rtc_alarm", rtc_dd);
	if (rc < 0) {
		dev_err(&pdev->dev, "Request IRQ failed (%d)\n", rc);
		goto fail_req_irq;
	}

	device_init_wakeup(&pdev->dev, 1);

	dev_dbg(&pdev->dev, "Probe success !!\n");

	return 0;

fail_req_irq:
	rtc_device_unregister(rtc_dd->rtc);
fail_rtc_enable:
	platform_set_drvdata(pdev, NULL);
	kfree(rtc_dd);
	return rc;
}
#ifdef CONFIG_ZTEMT_POWER_DEBUG
static time_t rtc_suspend_sec = 0;
static time_t rtc_resume_sec = 0;
static unsigned long all_sleep_time = 0;
static unsigned long all_wake_time = 0;

static void print_suspend_time(struct device *dev)
{
    
	int rc, diff=0;
	struct rtc_time tm;
	unsigned long now;
    rc=pm8xxx_rtc_read_time(dev,&tm);
 		if (rc) {
			printk(
				"%s: Unable to read from RTC\n", __func__);
		}
		rtc_tm_to_time(&tm, &now);
     rtc_suspend_sec=now;
     diff=rtc_suspend_sec-rtc_resume_sec;
     all_wake_time+=diff;
     printk("I have work %d seconds all_wake_time %lu seconds\n",diff,all_wake_time);
}

static void print_resume_time(struct device *dev)
{
    
	int rc, diff=0;
	struct rtc_time tm;
	unsigned long now;
    rc=pm8xxx_rtc_read_time(dev,&tm);
 		if (rc) {
			printk(
				"%s: Unable to read from RTC\n", __func__);
		}
		rtc_tm_to_time(&tm, &now);
     rtc_resume_sec=now;
     diff=rtc_resume_sec-rtc_suspend_sec;
     all_sleep_time+=diff;
     printk("I have sleep %d seconds all_sleep_time %lu seconds\n",diff,all_sleep_time);
}

#endif

#ifdef CONFIG_PM
static int pm8xxx_rtc_resume(struct device *dev)
{
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);
    #ifdef CONFIG_ZTEMT_POWER_DEBUG
    print_resume_time(dev);
    #endif
	if (device_may_wakeup(dev))
		disable_irq_wake(rtc_dd->rtc_alarm_irq);

	return 0;
}

static int pm8xxx_rtc_suspend(struct device *dev)
{
	struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);
    #ifdef CONFIG_ZTEMT_POWER_DEBUG
    print_suspend_time(dev);
    #endif
	if (device_may_wakeup(dev))
		enable_irq_wake(rtc_dd->rtc_alarm_irq);

	return 0;
}

static const struct dev_pm_ops pm8xxx_rtc_pm_ops = {
	.suspend = pm8xxx_rtc_suspend,
	.resume = pm8xxx_rtc_resume,
};
#endif
static int __devexit pm8xxx_rtc_remove(struct platform_device *pdev)
{
	struct pm8xxx_rtc *rtc_dd = platform_get_drvdata(pdev);

	device_init_wakeup(&pdev->dev, 0);
	free_irq(rtc_dd->rtc_alarm_irq, rtc_dd);
	rtc_device_unregister(rtc_dd->rtc);
	platform_set_drvdata(pdev, NULL);
	kfree(rtc_dd);

	return 0;
}

static void pm8xxx_rtc_shutdown(struct platform_device *pdev)
{
	u8 value[4] = {0, 0, 0, 0};
	u8 reg;
	int rc;
	unsigned long irq_flags;
	bool rtc_alarm_powerup = false;
	struct pm8xxx_rtc *rtc_dd = platform_get_drvdata(pdev);
	struct pm8xxx_rtc_platform_data *pdata = pdev->dev.platform_data;

	if (pdata != NULL)
		rtc_alarm_powerup =  pdata->rtc_alarm_powerup;

	if (!rtc_alarm_powerup) {

		spin_lock_irqsave(&rtc_dd->ctrl_reg_lock, irq_flags);
		dev_dbg(&pdev->dev, "Disabling alarm interrupts\n");

		/* Disable RTC alarms */
		reg = rtc_dd->ctrl_reg;
		reg &= ~PM8xxx_RTC_ALARM_ENABLE;
		rc = pm8xxx_write_wrapper(rtc_dd, &reg, rtc_dd->rtc_base, 1);
		if (rc < 0) {
			dev_err(rtc_dd->rtc_dev, "PM8xxx write failed\n");
			goto fail_alarm_disable;
		}

		/* Clear Alarm register */
		rc = pm8xxx_write_wrapper(rtc_dd, value,
				rtc_dd->alarm_rw_base, NUM_8_BIT_RTC_REGS);
		if (rc < 0)
			dev_err(rtc_dd->rtc_dev, "PM8xxx write failed\n");

fail_alarm_disable:
		spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);
	}
}

//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,start.
int msmrtc_rtc_read_alarm_time(struct device *dev, struct rtc_wkalrm *alarm)
{
	int rc;
	rc = pm8xxx_rtc_read_alarm(dev, alarm);
	return rc;
}
int msmrtc_timeremote_clear_rtc_alarm(struct device *dev)
{
      int rc = 0;
      u8 reg;
      u8 value[4] = {0, 0, 0, 0};	  
      unsigned long irq_flags;
      struct platform_device *pdev = container_of(dev, struct platform_device, dev);	  
      struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);	  	
      //struct pm8xxx_rtc *rtc_dd = platform_get_drvdata(pdev);
      struct pm8xxx_rtc_platform_data *pdata = pdev->dev.platform_data;

      //printk("PM_DEBUG_MXP: Enter msmrtc_timeremote_clear_rtc_alarm.\n");
      if (pdata == NULL)
      {
	    printk("PM_DEBUG_MXP: msmrtc_timeremote_clear_rtc_alarm: pdata == NULL.\n");
	    return -1;
      }
      spin_lock_irqsave(&rtc_dd->ctrl_reg_lock, irq_flags);
      /* Disable RTC alarms */	  
      dev_dbg(&pdev->dev, "Disabling alarm interrupts\n");
      reg = rtc_dd->ctrl_reg;	  
      reg &= ~PM8xxx_RTC_ALARM_ENABLE;
      rc = pm8xxx_write_wrapper(rtc_dd, &reg, rtc_dd->rtc_base, 1);
      if (rc < 0) 
     {
	    printk("PM_DEBUG_MXP:PM8xxx write failed,disable alarm failed.\n");
	   
      } 
      /* Clear Alarm register */
      rc = pm8xxx_write_wrapper(rtc_dd, value,rtc_dd->alarm_rw_base, NUM_8_BIT_RTC_REGS);
     if (rc < 0)
     {
	    //dev_err(rtc_dd->rtc_dev, "PM8xxx write failed\n");
	    printk("PM_DEBUG_MXP:PM8xxx write failed,clear alarm register failed.\n");
     }	 		
     spin_unlock_irqrestore(&rtc_dd->ctrl_reg_lock, irq_flags);
	 
     //printk("PM_DEBUG_MXP: Exit msmrtc_timeremote_clear_rtc_alarm.\n");	 
     return rc;
}
//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,end.

//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,start.
int msmrtc_virtual_alarm_set_to_cp(struct device *dev,unsigned long seconds)
{
      unsigned long now_time = get_seconds();
      int diff;  
      struct rtc_time tm;
      struct rtc_wkalrm alarm_tm;	  
      //printk("PM_DEBUG_MXP: alarm_seconds_to_be_set: seconds %ld\n",seconds);
      //printk("PM_DEBUG_MXP: now_time_seconds is:now_time %ld\n",now_time);
       diff = seconds-now_time;
      //printk("PM_DEBUG_MXP: msmrtc_virtual_alarm_set_to_cp diff %d\n",diff);
      if(diff>0)
      {
      	rtc_time_to_tm(seconds,&tm);
         //printk("PM_DEBUG_MXP: alarm_time_Gregorian: tm.tm_year %d, tm.mon %d, tm.day %d\n",tm.tm_year,tm.tm_mon,tm.tm_mday);
	//printk("PM_DEBUG_MXP: alarm_time_Gregorian: tm.tm_wday %d, tm.tm_yday %d.\n",tm.tm_wday,tm.tm_yday);	 
         //printk("PM_DEBUG_MXP: alarm_time_Gregorian: tm.tm_hour %d, tm.min %d, tm.sec %d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);
	
      	alarm_tm.enabled = 1;
	alarm_tm.pending = 0;
	alarm_tm.time = tm;
      	pm8xxx_rtc_set_alarm(dev,&alarm_tm);
      }
      else
     {
	return 0;
     }
     return 0;
}
//[ECID:0000]ZTE_BSP maxiaoping 20121030 modify PLATFORM 8064 RTC alarm driver for power_off alarm,end.

//[ECID:0000]ZTE_BSP maxiaoping 20121121 modify PLATFORM 8064 RTC alarm  for power_off charging,start.
int msmrtc_timeremote_get_rtc_alarm_status(struct device*dev)
{
	//int rc =0;
	int rtl =0;
	struct rtc_wkalrm   zte_rtc_alarm;
	unsigned long zte_rtc_alarm_sec;
	unsigned long temp;
	unsigned long now_time_sec = get_seconds();
	//struct pm8xxx_rtc *rtc_dd = dev_get_drvdata(dev);
	
	//printk("PM_DEBUG_MXP: Enter msmrtc_timeremote_get_rtc_alarm_status.\n");
	pr_debug("PM_DEBUG_MXP: now_seconds is:now_time_sec = %ld\n",now_time_sec);
	msmrtc_rtc_read_alarm_time(dev,&zte_rtc_alarm);
	rtc_tm_to_time(&zte_rtc_alarm.time, &zte_rtc_alarm_sec);//
	pr_debug("PM_DEBUG_MXP: zte_rtc_alarm_sec= %ld\n",zte_rtc_alarm_sec);//
	 if (zte_rtc_alarm_sec > now_time_sec)//exchage the value.
	 {
	 	temp = zte_rtc_alarm_sec;
	 	zte_rtc_alarm_sec = now_time_sec;
	 	now_time_sec = temp;
	 }
	 //As RTC has 3 seconds' error and the startup may takes some time,here we give them 5 seconds redundancy.
	  if ((now_time_sec -zte_rtc_alarm_sec)<=5)
	  {
	 	//printk("PM_DEBUG_MXP:RTC ALARM trigger detect.\r\n");
	 	rtl = 1;
	  }
	  else
	  {
		//printk("PM_DEBUG_MXP:NO RTC ALARM trigger detect.\r\n");
		rtl = 0;
	  }
	//printk("PM_DEBUG_MXP: rtc_dd->rtc_alarm_irq = %d.\n",rtc_dd->rtc_alarm_irq);
	//Here dev stand for platform device,so use its parent to correspond with pmic device.
	//but we can't get the alarm irq triggered always,so we need to read the counter to get the alarm status.
	//rc =pm8xxx_read_irq_stat(dev->parent, rtc_dd->rtc_alarm_irq);
	
	//printk("PM_DEBUG_MXP: Here get rtc alarm status -> rc  = %d.\n",rc);
	//if((rc == 1)||(rtl == 1))
	if(rtl == 1)
	{
		printk("PM_DEBUG_MXP: Get rtc alarm triggered.\n");
		return 1;
	}
	//printk("PM_DEBUG_MXP: Get rtc alarm not detected.\n");
	//printk("PM_DEBUG_MXP: Exit msmrtc_timeremote_get_rtc_alarm_status.\n");
	return 0;

}
//[ECID:0000]ZTE_BSP maxiaoping 20121121 modify PLATFORM 8064 RTC alarm  for power_off charging,end.


static struct platform_driver pm8xxx_rtc_driver = {
	.probe		= pm8xxx_rtc_probe,
	.remove		= __devexit_p(pm8xxx_rtc_remove),
	.shutdown	= pm8xxx_rtc_shutdown,
	.driver	= {
		.name	= PM8XXX_RTC_DEV_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &pm8xxx_rtc_pm_ops,
#endif
	},
};

static int __init pm8xxx_rtc_init(void)
{
	return platform_driver_register(&pm8xxx_rtc_driver);
}
module_init(pm8xxx_rtc_init);

static void __exit pm8xxx_rtc_exit(void)
{
	platform_driver_unregister(&pm8xxx_rtc_driver);
}
module_exit(pm8xxx_rtc_exit);

MODULE_ALIAS("platform:rtc-pm8xxx");
MODULE_DESCRIPTION("PMIC8xxx RTC driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Anirudh Ghayal <aghayal@codeaurora.org>");
