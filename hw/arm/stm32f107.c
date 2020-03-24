/*
 * STM32 f107
 *
 */

#include "hw/arm/stm32.h"
#include "hw/sysbus.h"
#include "hw/arm/arm.h"
#include "hw/devices.h"
#include "ui/console.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include <sys/time.h>

extern qemu_irq *pic;

static float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

static void led_irq_handler(void *opaque, int n, int level)
{
    /* There should only be one IRQ for the LED */
    assert(n == 0);
    static struct timeval t0;
    static struct timeval t1;
    float e;

    /* Assume that the IRQ is only triggered if the LED has changed state.
     * If this is not correct, we may get multiple LED Offs or Ons in a row.
     */
    switch (level) {
        case 0:
            gettimeofday(&t0, 0);
            e = timedifference_msec(t1, t0);
            printf("LED Off at %f\n", e);
            break;
        case 1:
            gettimeofday(&t1, 0);
            e = timedifference_msec(t0, t1);
            printf("LED On at %f\n", e);
            break;
    }

}

static void stm32_f107_init(MachineState *machine)
{
    const char* kernel_filename = machine->kernel_filename;
    qemu_irq *led_irq;

    stm32_init(/*flash_size*/0x0001ffff,
               /*ram_size*/0x00004fff,
               kernel_filename,
               25000000,
               32768);

    DeviceState *gpio_a = DEVICE(object_resolve_path("/machine/stm32/gpio[a]", NULL));
    DeviceState *gpio_c = DEVICE(object_resolve_path("/machine/stm32/gpio[c]", NULL));
    //paanth//
    DeviceState *gpio_b = DEVICE(object_resolve_path("/machine/stm32/gpio[b]", NULL));
    DeviceState *tim4 = DEVICE(object_resolve_path("/machine/stm32/timer[4]", NULL));
    /////////
    DeviceState *uart2 = DEVICE(object_resolve_path("/machine/stm32/uart[2]", NULL));
    DeviceState *uart1 = DEVICE(object_resolve_path("/machine/stm32/uart[1]", NULL));
    DeviceState *uart3 = DEVICE(object_resolve_path("/machine/stm32/uart[3]", NULL));
    assert(gpio_a);
    assert(gpio_b);
    assert(gpio_c);
    assert(uart2);
    assert(uart1);
    assert(uart3);
    assert(tim4);

    /* Connect LED to GPIO B pin 0 */
    led_irq = qemu_allocate_irqs(led_irq_handler, NULL, 1);
    qdev_connect_gpio_out(gpio_b, 0, led_irq[0]);


    /* Connect RS232 to UART 1 */
    stm32_uart_connect(
            (Stm32Uart *)uart1,
            serial_hds[0],
            STM32_USART1_NO_REMAP);

    /* These additional UARTs have not been tested yet... */
    stm32_uart_connect(
            (Stm32Uart *)uart2,
            serial_hds[1],
            STM32_USART2_NO_REMAP);

    stm32_uart_connect(
            (Stm32Uart *)uart3,
            serial_hds[2],
            STM32_USART3_NO_REMAP);
 }

static QEMUMachine stm32_f107_machine = {
    .name = "stm32f107",
    .desc = "Open 107V",
    .init = stm32_f107_init,
};


static void stm32_f107_machine_init(void)
{
    qemu_register_machine(&stm32_f107_machine);
}

machine_init(stm32_f107_machine_init);
