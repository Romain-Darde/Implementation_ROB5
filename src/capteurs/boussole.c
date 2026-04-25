#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <math.h>
#include "boussole.h"

#define AK09918_ADDR 0x0C
static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

int boussole_init(void) {
    uint8_t id = 0;
    i2c_reg_read_byte(i2c_dev, AK09918_ADDR, 0x00, &id); 
    
    printk("DEBUG: ID lu sur AK09918 = 0x%02x\n", id);

    if (id == 0x09) {
        printk("Boussole détectée avec succès !\n");
        return 0;
    } else {
        printk("ERREUR: ID boussole incorrect (différent de 0X09)\n");
        return -1;
    }
}

float boussole_get_angle(void) {
    uint8_t data[6];
    // On lit les 6 registres (X, Y, Z) en mode "Burst"
    i2c_burst_read(i2c_dev, AK09918_ADDR, 0x11, data, 6);

    // Recomposition des octets (Little Endian)
    int16_t x = (int16_t)(data[1] << 8 | data[0]);
    int16_t y = (int16_t)(data[3] << 8 | data[2]);

    // Calcul de l'angle en radians puis conversion en degrés
    float angle = atan2((float)y, (float)x) * 180.0f / 3.14159f;
    return angle;
}