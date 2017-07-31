// add to dts:
//   gdma@2800 {
//       compatible = "vocore2,ws2812-i2s";
//       status = "okay";
//   };
//

#include <linux/init.h>
#include <linux/module.h>

#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/sysfs.h>

#include <linux/vmalloc.h>

// one page size is 4096, so it is for 455 LEDs (455 x 3 x 3byte = 4095)
// three pages will be enough for 1024LEDs, but fps will not reach 30, max at
// 24fps, 2.4M / (4096 * 3 * 8) = 24.4
// To make it work at 30fps, have to rewrire dma irq, fill buffer again.
#define WS2812B_BUFSIZE	(PAGE_SIZE * 3)

struct ws2812_data {
	void __iomem *gdma;
	void __iomem *i2s;

	dma_addr_t addr;
	void *vaddr;
};

irqreturn_t ws2812_irq_handler(int irq, void *dev_id)
{
	struct ws2812_data *p = (struct ws2812_data *)dev_get_drvdata(dev_id);
	u32 mask, done;

	// MASK, DONE their register type is W1C(write one to clean)
	mask = readl(p->gdma + 0x0200);
	done = readl(p->gdma + 0x0204);
	writel(mask, p->gdma + 0x0200);
	writel(done, p->gdma + 0x0204);

	// once this send done, we do not need start next.
	return IRQ_HANDLED;
}

static ssize_t ws2812_update(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t size)
{
	struct ws2812_data *p = (struct ws2812_data *)dev_get_drvdata(dev);

	memset(p->vaddr, 0, WS2812B_BUFSIZE);
	size = (size > WS2812B_BUFSIZE ? WS2812B_BUFSIZE : size);
	memcpy(p->vaddr, buf, size);

	// setup i2s speed to 2.4MHz(480M / 2 / 100)
	writel(0x80000000, p->i2s + 0x0020);
	writel(0x00000064, p->i2s + 0x0024);
	writel(0xd1004040, p->i2s + 0x0000);

	// start dma transfer
	writel(p->addr, p->gdma + 0x0020);
	writel(0x10000a10, p->gdma + 0x0024);
	writel(0x00200210, p->gdma + 0x002c);
	writel(0x30000046, p->gdma + 0x0028);

	return size;
}

static DEVICE_ATTR(update, S_IWUSR, NULL, ws2812_update);

static int ws2812_probe(struct platform_device* dev)
{
	int irq, r;
	struct resource *res;
	struct ws2812_data *p;

	p = (struct ws2812_data *)kmalloc(sizeof(struct ws2812_data), GFP_KERNEL);
	memset(p, 0, sizeof(struct ws2812_data));
	platform_set_drvdata(dev, p);

	irq = platform_get_irq(dev, 0);
	r = request_irq(irq, ws2812_irq_handler, IRQF_SHARED, "ws2812-dma", &dev->dev);
	if(r < 0) {
		printk("request_irq failed %d.\n", r);
		return 0;
	}

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	p->gdma = devm_ioremap_resource(&dev->dev, res);
	p->i2s = p->gdma - 0x00002800 + 0x00000a00;
	p->vaddr = dma_alloc_coherent(&dev->dev, WS2812B_BUFSIZE, &p->addr, GFP_DMA);

	device_create_file(&dev->dev, &dev_attr_update);
	return 0;
}

static int ws2812_remove(struct platform_device* dev)
{
	struct ws2812_data *p = (struct ws2812_data *)platform_get_drvdata(dev);
	int irq = platform_get_irq(dev, 0);

	free_irq(irq, &dev->dev);
	dma_free_coherent(&dev->dev, WS2812B_BUFSIZE, p->vaddr, p->addr);
	kfree(p);

	device_remove_file(&dev->dev, &dev_attr_update);
	return 0;
}


static const struct of_device_id ws2812_ids[] = {
    { .compatible = "vocore2,ws2812-i2s", },
    { }
};

static struct platform_driver ws2812_driver = {
	.probe = ws2812_probe,
	.remove = ws2812_remove,
	.driver = {
	    .owner = THIS_MODULE,
	    .name = "ws2812-i2s",
	    .of_match_table = ws2812_ids,
	},
};

static int ws2812_init(void)
{
	return platform_driver_register(&ws2812_driver);
}

static void ws2812_exit(void)
{
	platform_driver_unregister(&ws2812_driver);
}

module_init(ws2812_init);
module_exit(ws2812_exit);


MODULE_AUTHOR("Qin Wei <me@vonger.cn>");
MODULE_DESCRIPTION("VoCore2 WS2812 via I2S");
MODULE_LICENSE("GPL");
