#include "lcd_platform_driver.h"

/* LCD Driver private data structure */
struct lcd_drv_private_data {
        struct device *dev;
};

static struct class *lcd_class;

static ssize_t lcdcmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	dev_info(dev, "%s: count:%d \n", __func__, count);
	return count;
}

static ssize_t lcdscroll_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	dev_info(dev, "%s: count:%d \n", __func__, count);
	return count;
}

static ssize_t lcdtext_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	dev_info(dev, "%s: count:%d \n", __func__, count);
	return count;
}

static ssize_t lcdxy_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "%s: \n", __func__);
	return 0;
}

static ssize_t lcdxy_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	dev_info(dev, "%s: count:%d \n", __func__, count);
	return count;
}

static DEVICE_ATTR_WO(lcdcmd);
static DEVICE_ATTR_WO(lcdscroll);
static DEVICE_ATTR_WO(lcdtext);
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

static void lcd_sysfs_remove(struct platform_device *pdev)
{
	struct lcd_drv_private_data *drv_data = platform_get_drvdata(pdev);

	if (!drv_data)
		dev_warn(&pdev->dev, "no drv_data on remove\n");

	if(drv_data->dev)
		device_unregister(drv_data->dev);

	platform_set_drvdata(pdev, NULL);
	dev_info(&pdev->dev, "lcd remove success\n");
}


static int lcd_sysfs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct lcd_drv_private_data *drv_data;

	drv_data = devm_kzalloc(dev, sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data) {
		dev_err(dev, "Cannot allocate driver memory\n");
		return -ENOMEM;
	}

	drv_data->dev = dev;
	platform_set_drvdata(pdev, drv_data);  // attach to device

	/* Create devices under /sys/class/bone_lcd */
	drv_data->dev = device_create_with_groups(lcd_class, NULL, 0, \
			drv_data, lcd_attr_groups, "LCD16x2");
	if (IS_ERR(drv_data->dev)) {
		dev_err(dev, "Error in device create\n");
		return PTR_ERR(drv_data->dev);
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

	pr_info("lcd module exit success\n");
}

module_init(lcd_sysfs_init);
module_exit(lcd_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raju Lakkaraju");
MODULE_DESCRIPTION("A LCD gpio sysfs driver");

