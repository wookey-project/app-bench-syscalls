/**
 * @file main.c
 *
 * \brief Main of dummy
 *
 */
#include "api/types.h"
#include "api/syscall.h"
#include "api/print.h"

#define PERIPH_BASE                         ((uint32_t) 0x40000000)
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)

#define USART1_BASE           (APB2PERIPH_BASE + 0x1000)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000)
#define USART6_BASE           (APB2PERIPH_BASE + 0x1400)


static const struct {
  char *   name;
  uint32_t addr;
  uint8_t  port;
  uint8_t  tx_pin;
  uint8_t  rx_pin;
  uint8_t  sc_port;
  uint8_t  sc_tx_pin;
  uint8_t  sc_ck_pin;
  uint8_t  irq;
  uint8_t  af;
} usarts[] = {
 { "",       0,           0,    0,  0,   0,  0,  0, 53, 0x7 },
 { "usart1", USART1_BASE, GPIO_PB,  6,  7, GPIO_PA,  9,  8, 53, 0x7 },
 { "usart2", USART2_BASE, GPIO_PA,  2,  3, GPIO_PA,  2,  4, 54, 0x7 },
 { "usart3", USART3_BASE, GPIO_PB, 10, 11, GPIO_PB, 10, 12, 55, 0x7 },
 { "uart4",   UART4_BASE, GPIO_PA,  0,  1,   0,  0,  0, 68, 0x8 },
 { "uart5",   UART5_BASE, GPIO_PC, 12,  2,   0,  0,  0, 69, 0x8 },
 { "usart6", USART6_BASE, GPIO_PC,  6,  7, GPIO_PC,  6,  8, 89, 0x8 }
};


void usart_handler(uint8_t irq __attribute__((unused)),
                   uint32_t sr __attribute__((unused)),
                   uint32_t dr __attribute__((unused)))
{
    return;
}

/*
 * We use the local -fno-stack-protector flag for main because
 * the stack protection has not been initialized yet.
 */
int _main(uint32_t task_id)
{
    e_syscall_ret ret = 0;
    uint64_t ts[2] = { 0 };
    device_t devs[3] = { 0 };
    int      dev_descriptor[3] = { 0 };
    const char * devs_name[] = {
        "usart2", "usart3", "usart4"
    };

    // test getcycles
    uint64_t val[10];

    // declaring 3 devices in dynamic map mode
    for (int i = 0; i < 3; ++i) {
       strncpy(devs[i].name, devs_name[i], strlen(devs[i].name));
       devs[i].address = usarts[i + 2].addr;
       devs[i].irqs[0].handler = usart_handler;

       devs[i].irqs[0].posthook.status = 0x0000; /* SR register */
       devs[i].irqs[0].posthook.data   = 0x0004; /* DR register */

       devs[i].irqs[0].posthook.action[0].instr = IRQ_PH_READ;
       devs[i].irqs[0].posthook.action[0].read.offset = 0x0000; /* SR register */

       devs[i].irqs[0].posthook.action[1].instr = IRQ_PH_READ;
       devs[i].irqs[0].posthook.action[1].read.offset = 0x0004; /* DR register */

       devs[i].irqs[0].posthook.action[2].instr = IRQ_PH_WRITE;
       devs[i].irqs[0].posthook.action[2].write.offset = 0x0000;
       devs[i].irqs[0].posthook.action[2].write.value  = 0x00;
       devs[i].irqs[0].posthook.action[2].write.mask   = 0x3 << 6; /* clear TC & Tx status */

       devs[i].irqs[0].mode = IRQ_ISR_STANDARD;

       devs[i].size = 0x400;
       devs[i].irq_num = 1;
       devs[i].gpio_num = 2;
       devs[i].map_mode = DEV_MAP_VOLUNTARY;
       devs[i].irqs[0].irq = usarts[i + 2].irq;

    /*
     * GPIOs
     */

       devs[i].gpios[0].mask =
          GPIO_MASK_SET_MODE | GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
          GPIO_MASK_SET_PUPD | GPIO_MASK_SET_AFR;
       devs[i].gpios[0].kref.port = usarts[i + 2].port;
       devs[i].gpios[0].kref.pin = usarts[i + 2].tx_pin;
       devs[i].gpios[0].mode = GPIO_PIN_ALTERNATE_MODE;
       devs[i].gpios[0].speed = GPIO_PIN_VERY_HIGH_SPEED;
       devs[i].gpios[0].afr = usarts[i + 2].af;
       devs[i].gpios[0].type = GPIO_PIN_OTYPER_PP;
       devs[i].gpios[0].pupd = GPIO_NOPULL;

       devs[i].gpios[1].mask =
          GPIO_MASK_SET_MODE | GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
          GPIO_MASK_SET_PUPD | GPIO_MASK_SET_AFR;
       devs[i].gpios[1].kref.port = usarts[i + 2].port;
       devs[i].gpios[1].kref.pin = usarts[i + 2].rx_pin;
       devs[i].gpios[1].afr = usarts[i + 2].af;
       devs[i].gpios[1].mode = GPIO_PIN_ALTERNATE_MODE;
       devs[i].gpios[1].speed = GPIO_PIN_VERY_HIGH_SPEED;
       devs[i].gpios[1].type = GPIO_PIN_OTYPER_PP;
       devs[i].gpios[1].pupd = GPIO_NOPULL;

       ret = sys_init(INIT_DEVACCESS, &devs[i], &dev_descriptor[i]);
    }

    printf("Hello ! I'm benchsyscall, my id is %x\n", task_id);


    ret = sys_init(INIT_DONE);
//    ret = sys_ipc(IPC_SEND_SYNC, 1, 15, buffer_out); // to sdio
    printf("sys_init DONE returns %x !\n", ret);


    printf("----------------------------------\n");
    printf("gettick tests...\n");
    printf("----------------------------------\n");

    for (uint8_t i = 0; i < 10; ++i) {
      sys_get_systick(&(val[i]), PREC_CYCLE);
    }
    for (uint8_t i = 0; i < 10; ++i) {
      printf("tick: %x\n", (uint32_t)val[i]);
    }
    uint32_t max_tickdiff = (uint32_t)val[1] - (uint32_t)val[0];
    uint32_t min_tickdiff = (uint32_t)val[1] - (uint32_t)val[0];
    uint64_t avg_tickdiff = (uint32_t)val[1] - (uint32_t)val[0];
    for (uint8_t i = 2; i < 10; ++i) {
        if (((uint32_t)val[i] - (uint32_t)val[i - 1]) > max_tickdiff) {
            max_tickdiff = (uint32_t)val[i] - (uint32_t)val[i - 1];
        }
        if (((uint32_t)val[i] - (uint32_t)val[i - 1]) < min_tickdiff) {
            min_tickdiff = (uint32_t)val[i] - (uint32_t)val[i - 1];
        }
        avg_tickdiff += ((uint32_t)val[i] - (uint32_t)val[i - 1]);
    }
    avg_tickdiff /= 9;
    printf("-----------\n");
    printf("max tickdiff: %x (%d) cycles\n", max_tickdiff, max_tickdiff);
    printf("min tickdiff: %x (%d) cycles\n", min_tickdiff, min_tickdiff);
    printf("avg tickdiff: %x (%d) cycles\n", (uint32_t)avg_tickdiff, (uint32_t)avg_tickdiff);



    printf("----------------------------------\n");
    printf("sleep tests...\n");
    printf("----------------------------------\n");

    uint32_t sleep_tab[] = { 200, 20, 3000, 50, 250, 100, 800, 400, 10, 70 };
    
    for (uint8_t i = 0; i < 10; ++i) {
      sys_get_systick(&ts[0], PREC_MICRO);
      sys_sleep(sleep_tab[i], SLEEP_MODE_DEEP);
      sys_get_systick(&ts[1], PREC_MICRO);
      printf("starting at %x us\n", (uint32_t)ts[0]);
      printf("awake at %x us\n", (uint32_t)ts[1]);
      printf("sleep duration: %d\n", (((uint32_t)ts[1] - (uint32_t)ts[0]) / 1000));
      printf("sleep wanted: %d\n", sleep_tab[i]);
      printf("-----------\n");
    }

    printf("----------------------------------\n");
    printf("dynamic mapping tests...\n");
    printf("----------------------------------\n");


    printf("mapping our own voluntary devices\n");
    printf("-----------\n");

    printf("mapping 1rst device (usart2)\n");
    ret = sys_cfg(CFG_DEV_MAP, dev_descriptor[0]);
    printf("sys_cfg(CFG_DEV_MAP returns %x !\n", ret);

    printf("mapping 2nd device (usart3)\n");
    ret = sys_cfg(CFG_DEV_MAP, dev_descriptor[1]);
    printf("sys_cfg(CFG_DEV_MAP returns %x !\n", ret);

    printf("mapping 3rd device (usart4)\n");
    ret = sys_cfg(CFG_DEV_MAP, dev_descriptor[2]);
    printf("sys_cfg(CFG_DEV_MAP returns %x !\n", ret);


    printf("unmapping 2nd device (usart3)\n");
    ret = sys_cfg(CFG_DEV_UNMAP, dev_descriptor[1]);
    printf("sys_cfg(CFG_DEV_UNMAP returns %x !\n", ret);

    printf("mapping 3rd device (usart4)\n");
    ret = sys_cfg(CFG_DEV_MAP, dev_descriptor[2]);
    printf("sys_cfg(CFG_DEV_MAP returns %x !\n", ret);

    printf("-----------\n");
    printf("try to map another device\n");
    ret = sys_cfg(CFG_DEV_MAP, 12); /* as lonely task, there isn't 12 devices mapped*/
    printf("sys_cfg(CFG_DEV_MAP returns %x !\n", ret);
    printf("-----------\n");
    printf("try to unmap another device\n");
    ret = sys_cfg(CFG_DEV_UNMAP, 12); /* as lonely task, there isn't 12 devices mapped*/
    printf("sys_cfg(CFG_DEV_MAP returns %x !\n", ret);
    printf("-----------\n");

    while (1) {
        sys_yield();
    }
    /* should return to do_endoftask() */
    return 0;
}
