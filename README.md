# Ordonnanceur Temps Réel

Ce projet implémente un simulateur d'ordonnancement temps réel en langage C, permettant de tester et comparer différents algorithmes d'ordonnancement sur des jeux de tâches périodiques.

## Objectifs

- **Ex 1** : Vérification de faisabilité avec l'algorithme HPF (Highest Priority First)
- **Ex 2** : Interface de saisie pour définir des jeux de tâches personnalisés
- **Ex 3** : Comparaison d'algorithmes d'ordonnancement (HPF, RM, DM)

## Architecture du Programme

### Structure des Données

```c
typedef struct {
    int id;                    // Identifiant de la tâche
    int duree_execution;       // Durée d'exécution (C)
    int echeance;             // Échéance relative (D)
    int periode;              // Période (T)
    int priorite;             // Priorité calculée ou saisie
    
    // Variables de simulation
    int temps_restant;        // Temps d'exécution restant
    int prochaine_activation; // Prochaine activation
    int prochaine_echeance;   // Prochaine échéance absolue
} Tache;
```

### Algorithmes Implémentés

| Algorithme | Description            | Priorité                                   |
|------------|------------------------|--------------------------------------------|
| **HPF**    | Highest Priority First | Priorités saisies manuellement             |
| **RM**     | Rate Monotonic         | Période plus courte = priorité plus haute  |
| **DM**     | Deadline Monotonic     | Échéance plus courte = priorité plus haute |


## Ex 1 : Etude de Faisabilité

### Données

Le programme utilise par défaut les tâches suivantes :

| Tâche        | C (exec) | D (échéance) | T (période) | P (priorité) |
|--------------|----------|--------------|-------------|--------------|
| **Thread 1** | 2        | 7            | 7           | 20           |
| **Thread 2** | 3        | 11           | 11          | 15           |
| **Thread 3** | 5        | 13           | 13          | 10           |

### Fonctionnement de la Simulation

1. **Activation périodique** : Les tâches s'activent selon leur période
2. **Ordonnancement HPF** : Sélection de la tâche avec la plus haute priorité
3. **Préemption** : Une tâche plus prioritaire interrompt l'exécution courante
4. **Détection d'échéances manquées** : Vérification à chaque activation
5. **Critère de succès** : Toutes les tâches doivent se terminer simultanément

### Résultat Attendu

Avec les données de l'ex 1, quel que soit l'odonnanceur le système est **non faisable** : La Tâche 3 est interrompue par les Tâches 1 et 2 plus prioritaires

## Utilisation du Programme

### Compilation

```bash
gcc -o faisabilite-ordonnanceur faisabilite-ordonnanceur.c
```

### Exécution

```bash
./faisabilite-ordonnanceur
```

### Interface Utilisateur

#### 1. Choix du Mode

```
+------------------------------------------------+
|                CHOIX DU MODE                   |
+------------------------------------------------+
| 1. Utiliser les donnees de l'exercice 1        |
| 2. Saisir de nouvelles taches                  |
+------------------------------------------------+
```

#### 2. Sélection de l'Algorithme

```
+------------------------------------------------+
|          CHOIX DE L'ALGORITHME                 |
+------------------------------------------------+
| 1. HPF - Highest Priority First                |
| 2. RM  - Rate Monotonic                        |
| 3. DM  - Deadline Monotonic                    |
+------------------------------------------------+
```

#### 3. Saisie des Tâches (Mode 2)

Pour chaque tâche, le programme demande :
- **C** : Durée d'exécution
- **D** : Échéance relative  
- **T** : Période
- **P** : Priorité (uniquement pour HPF)


## 🔍 Analyse des Résultats

### Interprétation HPF (Ex 1)

- **t=0-1** : Tâche 1 s'exécute (priorité 20)
- **t=2-4** : Tâche 2 s'exécute (priorité 15)  
- **t=5-6** : Tâche 3 commence (priorité 10)
- **t=7-8** : Tâche 1 preempte Tâche 3 (nouvelle instance)
- **t=9-10** : Tâche 3 reprend
- **t=11-12** : Tâche 2 preempte Tâche 3 (nouvelle instance)
- **t=13** : **Échéance manquée** pour Tâche 3

### Comparaison des Algorithmes

| Algorithme | Résultat     | Observation                         |
|------------|--------------|-------------------------------------|
| **HPF**    | Non faisable | Priorités manuelles inadaptées      |
| **RM**     | Non faisable | Même ordre que HPF pour ces données |
| **DM**     | Non faisable | Même ordre que HPF pour ces données |

> **Note** : Pour les données de l'ex 1, T=D pour toutes les tâches, donc RM et DM produisent le même ordonnancement, avec le même ordre de priorités que HPF.

