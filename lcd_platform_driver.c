#include "lcd_platform_driver.h"

/* LCD Driver private data structure */
struct lcd_drv_private_data {
	struct class *class_lcd;
};

struct lcd_drv_private_data lcd_drv_data;

static void lcd_sysfs_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	dev_info(dev, "lcd remove success\n");
}


static int lcd_sysfs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

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
	lcd_drv_data.class_lcd = class_create("bone_lcd");
	if (IS_ERR(lcd_drv_data.class_lcd)) {
		pr_err("Error in creating class\n");
		return PTR_ERR(lcd_drv_data.class_lcd);
	}

	platform_driver_register(&lcdsysfs_platform_driver);

	pr_info("lcd module load success\n");
	return 0;
}

static void __exit lcd_sysfs_exit(void)
{
	platform_driver_unregister(&lcdsysfs_platform_driver);
	class_destroy(lcd_drv_data.class_lcd);

	pr_info("lcd module exit success\n");
}

module_init(lcd_sysfs_init);
module_exit(lcd_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raju Lakkaraju");
MODULE_DESCRIPTION("A LCD gpio sysfs driver");

