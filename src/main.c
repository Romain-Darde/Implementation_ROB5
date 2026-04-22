#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include "drivers/moteur.h"

static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

// Signal A du codeur sur D2 (pin 10) pour capter les impulsions du moteur et compter les tours. Moteur connecté sur B1 et B2 du driver motor (Adresse I2C 0x14)
// Signal B du codeur sur D3 (pin 3) pour détecter le sens de rotation du moteur (non utilisé dans ce code, mais peut être utile pour des améliorations futures)
static const struct device *gpioa = DEVICE_DT_GET(DT_NODELABEL(gpioa));
static const struct device *gpiob = DEVICE_DT_GET(DT_NODELABEL(gpiob));

#define PIN_A 10 // D2 -> PA10
#define PIN_B 3  // D3 -> PB3

static struct gpio_callback codeur_cb;
volatile int compte = 0;

void codeur_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    compte++; 
}

int main(void) {
    // Test Phase 1
    // while (1) {

    //     // Test GPIO (Nouveau Codeur sur D2 / PA10)
    //     if (device_is_ready(gpioa)) {
    //         // On utilise gpioa partout au lieu de gpiob
    //         gpio_pin_configure(gpioa, PIN_A, GPIO_INPUT | GPIO_PULL_UP);
    //         gpio_pin_interrupt_configure(gpioa, PIN_A, GPIO_INT_EDGE_RISING);
    //         gpio_init_callback(&codeur_cb, codeur_handler, BIT(PIN_A));
    //         gpio_add_callback(gpioa, &codeur_cb);
            
    //         printk("OK: Codeur pret sur D2 (PA10)\n");
    //         k_msleep(2000);
    //     }

    //     // Test I2C (IMU 0x69 et Magneto 0x0C)
    //     if (device_is_ready(i2c_dev)) {
    //         uint8_t id_imu = 0, id_mag = 0;
    //         i2c_reg_read_byte(i2c_dev, 0x69, 0x75, &id_imu);
    //         i2c_reg_read_byte(i2c_dev, 0x0C, 0x00, &id_mag);
            
    //         printk("OK: IMU(0x69) ID=0x%x | Mag(0x0C) ID=0x%x\n", id_imu, id_mag);
    //         k_msleep(2000);
    //     } else {
    //         printk("ERREUR: Bus I2C non pret\n");
    //         k_msleep(2000);
    //     }

    //     // Test du codeur
    //     printk("Compteur codeur: %d\n", compte);
    //     k_msleep(2000);
    // }

    k_msleep(2000);
    printk("--- TEST MOTEUR ET CODEUR---\n");

    // Initialisation Codeur
    if (device_is_ready(gpioa)) {
        gpio_pin_configure(gpioa, PIN_A, GPIO_INPUT | GPIO_PULL_UP);
        gpio_pin_interrupt_configure(gpioa, PIN_A, GPIO_INT_EDGE_RISING);
        gpio_init_callback(&codeur_cb, codeur_handler, BIT(PIN_A));
        gpio_add_callback(gpioa, &codeur_cb);
        printk("Codeur ok sur D2\n");
    }

    // Initialisation Moteur
    moteur_init();
    k_msleep(3000);

    while (1) {
        printk("\n[1] Ordre: Vitesse +80. Compteur actuel: %d\r\n", compte);
        moteur_set_vitesse(80);
        k_msleep(500);
    }
    return 0;
}