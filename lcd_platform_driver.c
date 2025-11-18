#include "lcd_platform_driver.h"
#include "lcd_api.h"

static struct class *lcd_class;

static ssize_t lcdcmd_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	long value;
	int status;

	status = kstrtol(buf, 0, &value);
	if (!status)
		lcd_send_command(dev, (u8)value);

	return status ? : count;
}
static DEVICE_ATTR_WO(lcdcmd);

static ssize_t lcdtext_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	if(buf) {
		dev_info(dev, "lcd test: %s\n", buf);
		lcd_print_string(dev, (char *)buf);
	} else {
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR_WO(lcdtext);

static ssize_t lcdscroll_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	int ret;

	if (lcd_data->lcd_scroll)
		ret = sprintf(buf, "%s\n", "on");
	else
		ret = sprintf(buf, "%s\n", "off");

	return ret;
}

static ssize_t lcdscroll_store(struct device *dev, struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	int status = 0;

	if (sysfs_streq(buf, "on")) {
		lcd_data->lcd_scroll = 1;
		/* Display shift left */
		lcd_send_command(dev, 0x18);
	} else if (sysfs_streq(buf, "off")) {
		lcd_data->lcd_scroll = 0;
		/* Return home */
		lcd_send_command(dev, 0x2);
		/* Turn off display shift */
		lcd_send_command(dev, 0x10);
	} else {
		status = -EINVAL;
	}

	return status ? : count;
}
static DEVICE_ATTR_RW(lcdscroll);

static ssize_t lcdxy_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);

	return sprintf(buf, "%s\n", lcd_data->lcdxy);
}

static ssize_t lcdxy_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	long value;
	int status;
	int x, y;

	status = kstrtol(buf, 10, &value);
	if (status)
		return status;

	x = value / 10;
	y = value % 10;
	/* lcd 16x2 - x should be either 1 or 2 only */
	if (x < 1 || x > 2) {
		dev_warn(dev, "invalid input(%d, %d), range x:1-2, y:1-16\n", x, y);
		return count;
	}

	status = sprintf(lcd_data->lcdxy, "(%d, %d)", x, y);
	lcd_set_cursor(dev, x, y);

	return count;
}
static DEVICE_ATTR_RW(lcdxy);

static struct attribute *lcd_attrs[] = {
	&dev_attr_lcdcmd.attr,
	&dev_attr_lcdscroll.attr,
	&dev_attr_lcdtext.attr,
	&dev_attr_lcdxy.attr,
	NULL
};

static struct attribute_group lcd_attr_group = {
	.attrs = lcd_attrs,
};

static const struct attribute_group *lcd_attr_groups[] = {
	&lcd_attr_group,
	NULL,
};

static int lcd_sysfs_data_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct lcd_private_data *lcd_data = platform_get_drvdata(pdev);

	lcd_data->lcd_scroll = 0;
        sprintf(lcd_data->lcdxy, "(1, 1)");

	/* lcd gpio desc initialization */
	lcd_data->desc[LCD_RS] = devm_gpiod_get(dev, "rs", GPIOD_OUT_LOW);
	lcd_data->desc[LCD_RW] = devm_gpiod_get(dev, "rw", GPIOD_OUT_LOW);
	lcd_data->desc[LCD_EN] = devm_gpiod_get(dev, "en", GPIOD_OUT_LOW);
	lcd_data->desc[LCD_D4] = devm_gpiod_get(dev, "d4", GPIOD_OUT_LOW);
	lcd_data->desc[LCD_D5] = devm_gpiod_get(dev, "d5", GPIOD_OUT_LOW);
	lcd_data->desc[LCD_D6] = devm_gpiod_get(dev, "d6", GPIOD_OUT_LOW);
	lcd_data->desc[LCD_D7] = devm_gpiod_get(dev, "d7", GPIOD_OUT_LOW);
	if(IS_ERR(lcd_data->desc[LCD_RS]) || \
	   IS_ERR(lcd_data->desc[LCD_RW]) || \
	   IS_ERR(lcd_data->desc[LCD_EN]) || \
	   IS_ERR(lcd_data->desc[LCD_D4]) || \
	   IS_ERR(lcd_data->desc[LCD_D5]) || \
	   IS_ERR(lcd_data->desc[LCD_D6]) || \
	   IS_ERR(lcd_data->desc[LCD_D7]))   {
		dev_err(dev,"gpio init error\n");
		return -EINVAL;
	}

	return 0;
}

static void lcd_sysfs_remove(struct platform_device *pdev)
{
	struct lcd_private_data *lcd_data = platform_get_drvdata(pdev);

	if (!lcd_data)
		dev_warn(&pdev->dev, "no device data on remove\n");

	lcd_deinit(&pdev->dev);

	if(lcd_data->sysfs_dev)
		device_unregister(lcd_data->sysfs_dev);

	platform_set_drvdata(pdev, NULL);
	dev_info(&pdev->dev, "lcd remove success\n");
}

static int lcd_sysfs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct lcd_private_data *lcd_data;
	int ret;

	lcd_data = devm_kzalloc(dev, sizeof(*lcd_data), GFP_KERNEL);
	if (!lcd_data) {
		dev_err(dev, "Cannot allocate device memory\n");
		return -ENOMEM;
	}

	dev_set_drvdata(dev, lcd_data);  // attach to device

	/* Create devices under /sys/class/bone_lcd */
	lcd_data->sysfs_dev = device_create_with_groups(lcd_class, NULL, 0, \
				lcd_data, lcd_attr_groups, "LCD16x2");
	if (IS_ERR(lcd_data->sysfs_dev)) {
		dev_err(dev, "Error in device create\n");
		return PTR_ERR(lcd_data->sysfs_dev);
	}

	ret = lcd_sysfs_data_init(pdev);
	if (ret) {
		dev_err(dev, "lcd private data initialization failed\n");
		return ret;
	}

	ret = lcd_init(dev);
	if (ret) {
		dev_err(dev, "lcd initialization failed\n");
		return ret;
	}

	dev_info(dev, "lcd probe success\n");
	return 0;
}

struct of_device_id lcd_device_match[] = {
	{.compatible = "org,lcd16x2"},
	{ }
};

struct platform_driver lcdsysfs_platform_driver = {
	.probe = lcd_sysfs_probe,
	.remove = lcd_sysfs_remove,
	.driver = {
		.name = "bone-lcd-sysfs",
		.of_match_table = of_match_ptr(lcd_device_match)
	}
};

static  int __init lcd_sysfs_init(void)
{
	lcd_class = class_create("bone_lcd");
	if (IS_ERR(lcd_class)) {
		pr_err("Error in creating class\n");
		return PTR_ERR(lcd_class);
	}

	platform_driver_register(&lcdsysfs_platform_driver);

	pr_info("lcd module load success\n");
	return 0;
}

static void __exit lcd_sysfs_exit(void)
{
	platform_driver_unregister(&lcdsysfs_platform_driver);
	class_destroy(lcd_class);
}

module_init(lcd_sysfs_init);
module_exit(lcd_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raju Lakkaraju");
MODULE_DESCRIPTION("A LCD sysfs gpio driver");

