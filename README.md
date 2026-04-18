# Implementation_ROB5

Rattrapages : Système Asservi sur STM32

Ce projet détaille la mise en œuvre d'un système robotique asservi basé sur une carte STM32 Nucleo-F103RB sous Zephyr. L'objectif est la gestion d'un driver moteur, d'un codeur incrémental et d'une unité de mesure inertielle (IMU) pour permettre le contrôle d'une aiguille se trouvant sur le moteur et devant pointer le Nord quelque soit l'orientation du robot à l'aide du magnétomètre inclus dans l'IMU.

## Architecture Matérielle

Le système est monté sur une Base Shield V2.1 de seeed studio pour la gestion des entrées/sorties :

### Composants

| Composant | Interface | Broches / Port |
| :--- | :--- | :--- |
| **STM32 Nucleo-F103RB** | - | - |
| **Codeur rotatif** | GPIO | Port Grove D4 (PB5, PB4) |
| **IMU (ICM-20600)** | I2C | Port I2C (0x69) |
| **Magnétomètre (AK09918)** | I2C | Port I2C (0x0C) |
| **Driver Moteur (TB6612FNG)** | PWM / GPIO | Port Grove D6 (PB10, PA8) |

## Validation des Composants

La phase 1 a consisté en la prise en main du matériel et la vérification des branchements. Les tests de diagnostic au démarrage confirment l'intégrité du bus I2C et la réponse correcte des composants :

- Identifiant IMU (0x69) : 0x11 (Conforme)
- Identifiant Magnétomètre (0x0C) : 0x48 (Conforme)
- Codeur : Validation de l'incrémentation via interruption externe (EXTI) sur le port D4.

## Guide d'Utilisation

### Prérequis

- Environnement Zephyr SDK configuré (west).
- Chaîne de compilation ARM GCC.

### Compilation et Déploiement

Compiler le projet :

```bash
west build
```

Flasher la carte :

```bash
west flash
```

### Accès aux Logs de Diagnostic

La communication série est configurée à 115200 bauds. Pour consulter les logs sur le terminal :

```bash
sudo screen /dev/ttyACM0 115200
```

### Sorties obtenues phase 1: 

OK: Codeur prêt sur D4
IMU(0x69) ID=0x11 | Mag(0x0C) ID=0x48
Compteur codeur: 107
