#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

/* Configuration pour port Grove D4 (PB5=D4, PB4=D5) */
static const struct device *gpiob = DEVICE_DT_GET(DT_NODELABEL(gpiob));
static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

#define PIN_A 5 
#define PIN_B 4 

static struct gpio_callback codeur_cb;
volatile int compte = 0;

void codeur_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    compte++; // Vérification simple, on incrémente à chaque front montant sur D4
}

int main(void) {

    while (1) {

        // Test GPIO (Codeur D4)
        if (device_is_ready(gpiob)) {
            gpio_pin_configure(gpiob, PIN_A, GPIO_INPUT | GPIO_PULL_UP);
            gpio_pin_interrupt_configure(gpiob, PIN_A, GPIO_INT_EDGE_RISING);
            gpio_init_callback(&codeur_cb, codeur_handler, BIT(PIN_A));
            gpio_add_callback(gpiob, &codeur_cb);
            printk("OK: Codeur pret sur D4\n");
            k_msleep(2000);
        }

        // Test I2C (IMU 0x69 et Magneto 0x0C)
        if (device_is_ready(i2c_dev)) {
            uint8_t id_imu = 0, id_mag = 0;
            i2c_reg_read_byte(i2c_dev, 0x69, 0x75, &id_imu);
            i2c_reg_read_byte(i2c_dev, 0x0C, 0x00, &id_mag);
            
            printk("OK: IMU(0x69) ID=0x%x | Mag(0x0C) ID=0x%x\n", id_imu, id_mag);
            k_msleep(2000);
        } else {
            printk("ERREUR: Bus I2C non pret\n");
            k_msleep(2000);
        }

        // Test du codeur sur le moteur 
        printk("Compteur codeur: %d\n", compte);
        k_msleep(2000);
    }
    return 0;
}