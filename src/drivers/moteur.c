#include "moteur.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
#define MOTEUR_ADDR 0x14

#define CMD_WAKEUP  0x06
#define CMD_TRIGO   0x02
#define CMD_HORAIRE 0x03

// Moteur connécté sur le canal B du driver motor : B1 et B2
#define MOTOR_CHB 0x01 

void moteur_init(void) {
    if (!device_is_ready(i2c_dev)) {
        printk("ERREUR: I2C non pret pour le moteur\r\n");
        return;
    }
    
    uint8_t wakeup = CMD_WAKEUP;
    i2c_write(i2c_dev, &wakeup, 1, MOTEUR_ADDR);
    k_msleep(50); 

    moteur_set_vitesse(0);
    printk("Motor driver I2C (0x14) initialise (Moteur canal B)\r\n");
}

void moteur_set_vitesse(int vitesse) {
    uint8_t sens;
    
    if (vitesse > 90) vitesse = 90;
    if (vitesse < -90) vitesse = -90;

    // Arrêt si la vitesse est dans une zone de consgine trop faible pour faire tourner le moteur
    if (vitesse > -60 && vitesse < 60) {
        uint8_t stop[] = {CMD_TRIGO, MOTOR_CHB, 0x00};
        i2c_write(i2c_dev, stop, 3, MOTEUR_ADDR);
        return;
    }

    if (vitesse > 0) {
        sens = CMD_TRIGO;
    } else {
        sens = CMD_HORAIRE;
        vitesse = -vitesse; 
    }

    // Trame I2C : {Sens, Moteur B (0x01), Vitesse}
    uint8_t cmd[] = {0X02, MOTOR_CHB, (uint8_t)vitesse};
    i2c_write(i2c_dev, cmd, 3, MOTEUR_ADDR);
}