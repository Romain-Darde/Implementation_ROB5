#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <math.h>

#include "drivers/moteur.h"
#include "capteurs/codeur.h"
#include "capteurs/boussole.h"
#include "controle/pid.h"

struct RobotState {
    float angle_actuel;   // Position actuelle de l'aiguille
    float consigne;       // Où l'on veut que l'aiguille pointe (Le Nord = 0.0)
};

// Initialisation de l'état du robot à 0
struct RobotState etat_robot = {0.0, 0.0};

// Création d'un PID global pour le contrôle du moteur
PID_Controller mon_pid;

K_MUTEX_DEFINE(robot_mutex);
K_MUTEX_DEFINE(i2c_mutex);

// Thread de lecture des capteurs qui met à jour l'état du robot (angle actuel) en continu

void thread_capteurs_fn(void *arg1, void *arg2, void *arg3) {
    float angle_filtre = 0.0f;
    while (1) {
        float nouvel_angle = boussole_get_angle();

        // Filtre pour lisser les lectures de la boussole
        float erreur_angle = nouvel_angle - angle_filtre;
        if (erreur_angle > 180.0f) {
            erreur_angle -= 360.0f;
        } else if (erreur_angle < -180.0f) {
            erreur_angle += 360.0f;
        }
        angle_filtre += 0.2f * erreur_angle;
        if (angle_filtre < 0.0f) {
            angle_filtre += 360.0f;
        } else if (angle_filtre >= 360.0f) {
            angle_filtre -= 360.0f;
        }

        k_mutex_lock(&robot_mutex, K_FOREVER);
        etat_robot.angle_actuel = angle_filtre;
        k_mutex_unlock(&robot_mutex);

        // Permet que ce ne soit pas trop rapide
        k_msleep(50); 
    }
}

// Thread de contrôle PID qui lit l'état du robot, calcule la commande et l'envoie au moteur

void thread_pid_fn(void *arg1, void *arg2, void *arg3) {
    float dt = 0.01f;
    const float DEGREES_PER_TICK = 360.0f / 207.0f;

    k_msleep(200);
    k_mutex_lock(&robot_mutex, K_FOREVER);
    float angle_robot_initial = etat_robot.angle_actuel;
    k_mutex_unlock(&robot_mutex);

    float angle_aiguille_initial = (float)codeur_get_ticks() * DEGREES_PER_TICK;
    angle_aiguille_initial = fmodf(angle_aiguille_initial, 360.0f);
    if (angle_aiguille_initial < 0.0f) {
        angle_aiguille_initial += 360.0f;
    }

    float offset_nord = angle_robot_initial + angle_aiguille_initial;
    offset_nord = fmodf(offset_nord, 360.0f);
    if (offset_nord < 0.0f) {
        offset_nord += 360.0f;
    }

    while (1) {
        k_mutex_lock(&robot_mutex, K_FOREVER);
        float angle_robot = etat_robot.angle_actuel;
        k_mutex_unlock(&robot_mutex);

        // Récupération de la position actuelle de l'aiguille via le codeur
        float angle_aiguille = (float)codeur_get_ticks() * DEGREES_PER_TICK;
        angle_aiguille = fmodf(angle_aiguille, 360.0f);
        if (angle_aiguille < 0.0f) {
            angle_aiguille += 360.0f;
        }

        float cible_aiguille = offset_nord - angle_robot;

        // Le PID compare la position de l'aiguille à sa cible
        int commande = pid_calculer_commande(&mon_pid, cible_aiguille, angle_aiguille, dt);
        
        moteur_set_vitesse(commande);

        k_msleep(10); 
    }
}

// Définition des threads avec une pile de 1024 bytes, 7 correspond au niveau de priorité pour le codeur et 5 le niveau de priorité pour le PID
K_THREAD_DEFINE(thread_capteurs_id, 1024, thread_capteurs_fn, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(thread_pid_id, 1024, thread_pid_fn, NULL, NULL, NULL, 5, 0, 0);

// Ancienne version du main pour tester le codeur et le moteur séparément

static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

// // Signal A du codeur sur D2 (pin 10) pour capter les impulsions du moteur et compter les tours. Moteur connecté sur B1 et B2 du driver motor (Adresse I2C 0x14)
// // Signal B du codeur sur D3 (pin 3) pour détecter le sens de rotation du moteur (non utilisé dans ce code, mais peut être utile pour des améliorations futures)
// static const struct device *gpioa = DEVICE_DT_GET(DT_NODELABEL(gpioa));
// static const struct device *gpiob = DEVICE_DT_GET(DT_NODELABEL(gpiob));

// #define PIN_A 10 // D2 -> PA10
// #define PIN_B 3  // D3 -> PB3

// static struct gpio_callback codeur_cb;
// volatile int compte = 0;

// void codeur_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
//     compte++; 
// }

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

    // Ancien code test moteur et codeur séparément avant de passer à la phase 2

    // k_msleep(2000);
    // printk("--- TEST MOTEUR ET CODEUR---\n");

    // // Initialisation Codeur
    // if (device_is_ready(gpioa)) {
    //     gpio_pin_configure(gpioa, PIN_A, GPIO_INPUT | GPIO_PULL_UP);
    //     gpio_pin_interrupt_configure(gpioa, PIN_A, GPIO_INT_EDGE_RISING);
    //     gpio_init_callback(&codeur_cb, codeur_handler, BIT(PIN_A));
    //     gpio_add_callback(gpioa, &codeur_cb);
    //     printk("Codeur ok sur D2\n");
    // }

    // // Initialisation Moteur
    // moteur_init();
    // k_msleep(3000);

    // while (1) {
    //     printk("\n[1] Ordre: Vitesse +80. Compteur actuel: %d\r\n", compte);
    //     moteur_set_vitesse(80);
    //     k_msleep(500);
    // }
    // return 0;

    k_msleep(3000); // Laisser le temps d'ouvrir le terminal après le reset
    //printk("Démarrage\n");

    // Test debug I2C pour vérifier que la boussole répond avant de lancer le reste du code
    // printk("TEST I2C\n");
    // uint8_t data;
    // for (uint8_t i = 1; i < 127; i++) {
    //     if (i2c_reg_read_byte(i2c_dev, i, 0x00, &data) == 0) {
    //         printk("Appareil trouve a l'adresse: 0x%02x\n", i);
    //     }
    // }

    // Initialisation hardware (I2C, GPIO, Moteur, Codeur, Boussole)
    //k_msleep(3000);
    //printk("Initialisation hardware\n");
    moteur_init();
    codeur_init();
    boussole_init();

    k_msleep(500);
    k_mutex_lock(&robot_mutex, K_FOREVER);
    float angle_depart = etat_robot.angle_actuel;
    k_mutex_unlock(&robot_mutex);

    // Test de la boussole pour vérifier qu'elle répond avant de lancer le reste du code
    // if (boussole_init() != 0) {
    //     printk("ATTENTION: La boussole ne répond pas, vérifie le câblage !\n");
    // } else {
    //     printk("Boussole OK !\n");
    // }

    // Valeurs du PID ordre : Kp=1.5, Ki=0.1, Kd=0.5
    pid_init(&mon_pid, 1.5, 0.0, 0.0);

    while (1) {
        float a, c;
        k_mutex_lock(&robot_mutex, K_FOREVER);
        a = etat_robot.angle_actuel;
        c = etat_robot.consigne;
        k_mutex_unlock(&robot_mutex);

        printk("Cible: %.1f | Actuel: %.1f | Erreur: %.1f\r\n", c, a, c - a);
        k_msleep(1000);
    }

    return 0;
}