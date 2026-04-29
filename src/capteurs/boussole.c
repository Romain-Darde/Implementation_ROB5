#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <math.h>
#include "boussole.h"

extern struct k_mutex i2c_mutex;

#define AK09918_ADDR 0x0C
static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

int boussole_init(void) {
    if (!device_is_ready(i2c_dev)) {
        printk("ERREUR: Boussole I2C non prête\n");
        return -1;
    }

    k_mutex_lock(&i2c_mutex, K_FOREVER);
    i2c_reg_write_byte(i2c_dev, AK09918_ADDR, 0x31, 0x08);
    k_mutex_unlock(&i2c_mutex);
    k_msleep(50);
    return 0;
}

float boussole_get_angle(void) {
    static float derniere_valeur = 0.0f; // Garde la valeur en mémoire
    uint8_t data[6];

    if (!device_is_ready(i2c_dev)) {
        return derniere_valeur;
    }

    k_mutex_lock(&i2c_mutex, K_FOREVER);
    int ret = i2c_burst_read(i2c_dev, AK09918_ADDR, 0x11, data, 6);
    k_mutex_unlock(&i2c_mutex);
    
    // Si la lecture échoue, on renvoie la dernière valeur connue
    if (ret != 0) return derniere_valeur;

    int16_t x = (int16_t)(data[1] << 8 | data[0]);
    int16_t y = (int16_t)(data[3] << 8 | data[2]);

    float angle = atan2f((float)y, (float)x) * 180.0f / 3.14159f;
    if (angle < 0.0f) {
        angle += 360.0f;
    }

    derniere_valeur = angle; // Mise à jour de la mémoire
    return angle;
}
