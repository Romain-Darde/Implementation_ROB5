# Rapport de Projet et liens avec les cours suivis en échange (cours disponibles dans le dossier "cours") : Asservissement d'Aiguille sous Zephyr RTOS

## 1. Introduction
Ce projet implémente un système d'asservissement sur une carte STM32 Nucleo sous Zephyr RTOS. L'objectif est de maintenir une aiguille pointée vers le Nord magnétique en compensant les rotations de la base du robot. Ce système complexe a nécessité l'application des concepts de Systèmes Temps Réel (ETS) pour garantir la réactivité, la fiabilité et la protection des ressources partagées. 

## 2. Modèle Multitâche et Planification (Cours suivi tema 3 et tema 8: Planificación y Tareas)
Le système a été modélisé autour de plusieurs tâches (threads) concurrentes afin de séparer l'acquisition des données de la logique de contrôle. 
* **Thread Capteurs :** Chargé de lire la boussole I2C (AK09918) et d'appliquer un filtre.
* **Thread PID :** Chargé du calcul d'erreur et de l'asservissement du moteur DC.

Une priorité a été assignée aux threads lors de leur création (k_thread_create). Le thread capteur possède la priorité 7, tandis que le thread PID possède la priorité 5, modèle de priorité fixe typique des systèmes en temps réel, où la tâche la plus prioritaire s'exécute dès qu'elle est prête. L'initialisation différée des threads depuis le main() a permis d'éviter les situations de course lors du démarrage matériel.

## 3. Gestion des Interruptions et "Debouncing" (Cours suivi tema 2 :Gestión de Interrupciones)
Le retour de position du moteur est assuré par un codeur incrémental. Le comptage des impulsions est géré de manière asynchrone via un service d'interruption : codeur_handler.
Conformément à la théorie, un service d'interruption doit avoir un temps d'exécution minimal pour ne pas bloquer l'ordonnanceur. Pour pallier la problématique du "rebond" mécanique créant des "tempêtes d'interruptions", j'ai implémenté un filtrage de debouncing. Le comptage utilise des opérations atomiques garantissant la cohérence des données partagées sans nécessiter de verrouillage bloquant.

## 4. Synchronisation et Exclusion Mutuelle (Cours suivi tema 6 : Sincronización / Semáforos y Mutex)
Le projet utilise des mécanismes de synchronisation pour gérer l'accès concurrent aux ressources matérielles.

### A. Exclusion Mutuelle (Mutex)
Le bus I2C est partagé entre le magnétomètre, l'IMU et le driver moteur. Pour éviter qu'un thread ne corrompe une trame I2C en cours d'envoi, un mutex (i2c_mutex) protège le bus, définissant une section critique pour assurer l'atomicité des opérations. Afin d'éviter les interblocages (Deadlocks) ou les attentes infinies en cas de défaillance matérielle (ex: Brownout), j'ai implémenté un timeout de sécurité (10 ms), respectant la gestion des surtemps (sobretiempo) préconisée dans les systèmes critiques.

### B. Synchronisation Événementielle (Semáforos)
Un sémaphore (sem_boussole) est utilisé pour orchestrer le démarrage du système. Le thread PID attend passivement via k_sem_take que le thread Capteurs valide la stabilisation du filtre via un k_sem_give. Cette approche évite l'attente active et optimise l'utilisation des ressources CPU en bloquant le thread tant que la condition n'est pas remplie.

## 5. Gestion du Temps et Temporisateurs (Cours suivi tema 5 :Temporizadores)
La précision de la boucle de contrôle PID repose sur une gestion du temps (paramètre dt). Les appels à k_msleep() garantissent le cadencement régulier des tâches par des délais relatifs et des activations périodiques, permettant à l'ordonnanceur de passer les threads en état bloqué pour libérer le processeur pour les autres activités.

## 6. Conclusion 
Ce projet a permis l'application des différents concepts vus en cours "Informatique Industriel" : Mutex avec timeouts pour la sécurité du bus, interruptions pour la capture d'événements et sémaphores pour la synchronisation entre les tâches.
