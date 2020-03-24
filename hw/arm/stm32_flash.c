#include "hw/arm/stm32.h"

#include <stdio.h>

#include "qemu/bitops.h"

#define FLASH_ACR_OFFSET         0x00

#define FLASH_ACR_PRFTBS_BIT      5
#define FLASH_ACR_PRFTBE_BIT      4
#define FLASH_ACR_HLFCYA_BIT      3
#define FLASH_ACR_LATENCY_START   0
#define FLASH_ACR_LATENCY_LENGTH  3
#define FLASH_ACR_LATENCY_MASK    0x00000007

struct Stm32Flash {
    /* Inherited */
    SysBusDevice busdev;

    /* Properties */

    /* Private */
    MemoryRegion iomem;

    /* Register Values */
    //uint32_t
        //FLASH_ACR; //there are more regs

    /* Register Field Values */
    uint32_t //ACR
        FLASH_ACR_LATENCY;

    /* Bit ACR register values */
    bool
        FLASH_ACR_HLFCYA,
        FLASH_ACR_PRFTBE,
        FLASH_ACR_PRFTBS;

    qemu_irq irq;
};


/* Read flash ACR register. */
static uint32_t stm32_flash_ACR_read(Stm32Flash *s)
{
    printf("acr read\n");
    return (s->FLASH_ACR_PRFTBS << FLASH_ACR_PRFTBS_BIT) |
           (s->FLASH_ACR_PRFTBE << FLASH_ACR_PRFTBE_BIT) |
           (s->FLASH_ACR_HLFCYA << FLASH_ACR_HLFCYA_BIT) |
           (s->FLASH_ACR_LATENCY << FLASH_ACR_LATENCY_START);
}

/* Write flash ACR register. */
static void stm32_flash_ACR_write(Stm32Flash *s, uint32_t new_value, bool init)
{
    bool new_HLFCYA, new_PRFTBE;
    uint32_t new_LATENCY;
    printf("acr write\n");

    /*latency*/
    new_LATENCY = extract32(new_value,
                            FLASH_ACR_LATENCY_START,
                            FLASH_ACR_LATENCY_LENGTH);
    s->FLASH_ACR_LATENCY = new_LATENCY;

    /*half cycle access enable*/
    new_HLFCYA = new_value & BIT(FLASH_ACR_HLFCYA_BIT);
    s->FLASH_ACR_HLFCYA = new_HLFCYA;

    /*prefetch buffer*/
    new_PRFTBE = new_value & BIT(FLASH_ACR_PRFTBE_BIT);
    s->FLASH_ACR_PRFTBE = new_PRFTBE;
    s->FLASH_ACR_PRFTBS = new_PRFTBE;
}

static void stm32_flash_init_acr(Stm32Flash *s)
{
    s->FLASH_ACR_LATENCY = 0;
    s->FLASH_ACR_HLFCYA = 0;
    s->FLASH_ACR_PRFTBE = 0;
    s->FLASH_ACR_PRFTBS = 1;
}

static uint64_t stm32_flash_acr_readw(void *opaque, hwaddr offset)
{
    Stm32Flash *s = (Stm32Flash *)opaque;

    switch (offset) {
        case FLASH_ACR_OFFSET:
            return stm32_flash_ACR_read(s);
        default:
            STM32_BAD_REG(offset, 4);
            return 0;
    }
    return 0;
}


static void stm32_flash_acr_writew(void *opaque, hwaddr offset,
                          uint64_t value)
{
    Stm32Flash *s = (Stm32Flash *)opaque;

    switch(offset) {
        case FLASH_ACR_OFFSET:
            stm32_flash_ACR_write(s, value, false);
            break;
        default:
            STM32_BAD_REG(offset, 4);
            break;
    }
}


static uint64_t stm32_flash_read(void *opaque, hwaddr offset,
                                     unsigned size)
{
    switch(size) {
        case 4:
            return stm32_flash_acr_readw(opaque, offset);
        default:
            STM32_NOT_IMPL_REG(offset, size);
            return 0;
    }
}

static void stm32_flash_write(void *opaque, hwaddr offset,
                                  uint64_t value, unsigned size)
{
    switch(size) {
        case 4:
            stm32_flash_acr_writew(opaque, offset, value);
            break;
        default:
            STM32_NOT_IMPL_REG(offset, size);
            break;
    }
}

static void stm32_flash_reset(DeviceState *dev)
{
    Stm32Flash *s = STM32_FLASH(dev);

    stm32_flash_ACR_write(s, 0x00000030, true);

}


static const MemoryRegionOps stm32_flash_acr_ops = {
    .read = stm32_flash_read,
    .write = stm32_flash_write,
    .endianness = DEVICE_NATIVE_ENDIAN
};


static int stm32_flash_init(SysBusDevice *dev)
{
    Stm32Flash *s = STM32_FLASH(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &stm32_flash_acr_ops, s,
                          "flash", 0x3FF);

    sysbus_init_mmio(dev, &s->iomem);

    sysbus_init_irq(dev, &s->irq);

    stm32_flash_init_acr(s);

    return 0;
}

static Property stm32_flash_properties[] = {
    DEFINE_PROP_END_OF_LIST()
};


static void stm32_flash_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);

    k->init = stm32_flash_init;
    dc->reset = stm32_flash_reset;
    dc->props = stm32_flash_properties;
}

static TypeInfo stm32_flash_info = {
    .name  = "stm32-flash",
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(Stm32Flash),
    .class_init = stm32_flash_class_init
};

static void stm32_flash_register_types(void)
{
    type_register_static(&stm32_flash_info);
}

type_init(stm32_flash_register_types)
