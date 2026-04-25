#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "codeur.h"

// Définitions des pins (D2 correspond à PA10)
#define PIN_A 10
static const struct device *gpioa = DEVICE_DT_GET(DT_NODELABEL(gpioa));
static struct gpio_callback codeur_cb;

// Variable volatile car modifiée dans une interruption
static volatile int compte = 0;

// La fonction appelée à chaque tick du codeur
void codeur_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    compte++;
}

void codeur_init(void) {
    if (!device_is_ready(gpioa)) return;

    gpio_pin_configure(gpioa, PIN_A, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_interrupt_configure(gpioa, PIN_A, GPIO_INT_EDGE_RISING);
    gpio_init_callback(&codeur_cb, codeur_handler, BIT(PIN_A));
    gpio_add_callback(gpioa, &codeur_cb);
}

int codeur_get_ticks(void) {
    return compte;
}

void codeur_reset(void) {
    compte = 0;
}