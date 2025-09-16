#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Énumération pour les types d'algorithmes d'ordonnancement
typedef enum {
    HPF,  // Highest Priority First
    RM,   // Rate Monotonic
    DM    // Deadline Monotonic
} TypeOrdonnanceur;

// Structure pour représenter une tâche temps réel
typedef struct {
    int id;                    // Identifiant de la tâche
    int duree_execution;       // Durée d'exécution (C)
    int echeance;             // Échéance relative (D)
    int periode;              // Période (T)
    int priorite;             // Priorité (plus grand = haute priorité)
    
    // Variables pour la simulation
    int temps_restant;        // Temps d'exécution restant pour l'instance courante
    int prochaine_activation; // Prochaine activation de la tâche
    int prochaine_echeance;   // Prochaine échéance absolue
} Tache;

// Structure pour stocker l'état du système
typedef struct {
    Tache *taches;
    int nb_taches;
    TypeOrdonnanceur algorithme;
} Systeme;

// Fonction pour initialiser une tâche
void initialiser_tache(Tache *tache, int id, int duree, int echeance, int periode, int priorite) {
    tache->id = id;
    tache->duree_execution = duree;
    tache->echeance = echeance;
    tache->periode = periode;
    tache->priorite = priorite;
    tache->temps_restant = 0;
    tache->prochaine_activation = 0;
    tache->prochaine_echeance = echeance;
}

// Fonction pour calculer les priorités selon Rate Monotonic (plus courte période = plus haute priorité)
void calculer_priorites_rm(Systeme *systeme) {
    // Trouver les périodes pour assigner les priorités
    for (int i = 0; i < systeme->nb_taches; i++) {
        int priorite = 1000; // Priorité de base élevée
        for (int j = 0; j < systeme->nb_taches; j++) {
            if (systeme->taches[j].periode < systeme->taches[i].periode) {
                priorite--;
            }
        }
        systeme->taches[i].priorite = priorite;
    }
}

// Fonction pour calculer les priorités selon Deadline Monotonic (plus courte échéance = plus haute priorité)
void calculer_priorites_dm(Systeme *systeme) {
    // Trouver les échéances pour assigner les priorités
    for (int i = 0; i < systeme->nb_taches; i++) {
        int priorite = 1000; // Priorité de base élevée
        for (int j = 0; j < systeme->nb_taches; j++) {
            if (systeme->taches[j].echeance < systeme->taches[i].echeance) {
                priorite--;
            }
        }
        systeme->taches[i].priorite = priorite;
    }
}

// Fonction pour trouver la tâche avec la plus haute priorité prête à s'exécuter (HPF, RM, DM)
int trouver_tache_priorite_max(Systeme *systeme) {
    int tache_selectionnee = -1;
    int priorite_max = -1;
    
    for (int i = 0; i < systeme->nb_taches; i++) {
        Tache *tache = &systeme->taches[i];
        
        // Vérifier si la tâche a du temps d'exécution restant
        if (tache->temps_restant > 0) {
            if (tache->priorite > priorite_max) {
                priorite_max = tache->priorite;
                tache_selectionnee = i;
            }
        }
    }
    
    return tache_selectionnee;
}

// Fonction pour vérifier si toutes les tâches sont simultanément terminées
bool toutes_taches_terminees_simultanement(Systeme *systeme) {
    for (int i = 0; i < systeme->nb_taches; i++) {
        if (systeme->taches[i].temps_restant > 0) {
            return false;
        }
    }
    return true;
}

// Fonction pour activer les tâches à un instant donné
bool activer_taches(Systeme *systeme, int temps_actuel) {
    bool aucune_echeance_manquee = true;
    
    for (int i = 0; i < systeme->nb_taches; i++) {
        Tache *tache = &systeme->taches[i];
        
        // Vérifier si c'est le moment d'activer la tâche
        if (temps_actuel == tache->prochaine_activation) {
            // Vérifier si l'instance précédente n'est pas terminée (échéance manquée)
            if (tache->temps_restant > 0) {
                printf("  >> ECHEANCE MANQUEE pour la tache %d (temps restant: %d)\n", 
                       tache->id, tache->temps_restant);
                aucune_echeance_manquee = false;
            }
            
            tache->temps_restant = tache->duree_execution;
            printf("  -> Activation de la tache %d (echeance: %d)\n", 
                   tache->id, tache->prochaine_echeance);
            
            // Programmer la prochaine activation et échéance
            tache->prochaine_activation += tache->periode;
            tache->prochaine_echeance += tache->periode;
        }
    }
    
    return aucune_echeance_manquee;
}

// Fonction pour obtenir le nom de l'algorithme
const char* nom_algorithme(TypeOrdonnanceur algo) {
    switch (algo) {
        case HPF: return "HPF (Highest Priority First)";
        case RM:  return "RM (Rate Monotonic)";
        case DM:  return "DM (Deadline Monotonic)";
        default:  return "Inconnu";
    }
}

// Fonction principale de simulation adaptée à tous les algorithmes
bool simuler_ordonnancement(Systeme *systeme) {
    printf("+------------------------------------------------+\n");
    printf("|         SIMULATION %s               |\n", 
           systeme->algorithme == HPF ? "HPF PREEMPTIF" :
           systeme->algorithme == RM ? "RM PREEMPTIF " : "DM PREEMPTIF ");
    printf("+------------------------------------------------+");
    
    int temps = 0;
    
    while (temps < 100) { // Protection contre boucle infinie
        printf("\n[t = %d]\n", temps);
        
        // Étape 1: Activer les nouvelles instances de tâches
        if (!activer_taches(systeme, temps)) {
            printf("\n\t >> SIMULATION ARRETEE: Echeance manquee !\n");
            return false;
        }
        
        // Étape 2: Sélectionner la tâche selon l'algorithme (tous utilisent les priorités)
        int tache_courante = trouver_tache_priorite_max(systeme);
        
        if (tache_courante != -1) {
            Tache *tache = &systeme->taches[tache_courante];
            tache->temps_restant--;
            
            printf("Execution tache %d (priorite=%d, temps restant=%d)", 
                   tache->id, tache->priorite, tache->temps_restant);
            
            if (tache->temps_restant == 0) {
                printf(" [TERMINEE]");
            }
            printf("\n");
        } else {
            printf("Processeur inactif\n");
        }
        
        // Étape 3: Vérifier si toutes les tâches sont terminées simultanément
        if (toutes_taches_terminees_simultanement(systeme)) {
            printf("\n\t >> SUCCES: Toutes les taches sont terminees simultanement!\n");
            return true;
        }
        
        temps++;
    }
    
    printf("       >> TIMEOUT: Simulation arretee apres 100 unites\n");
    return false;
}

// Fonction pour saisir les paramètres des tâches
void saisir_taches(Systeme *systeme) {
    printf("\n+------------------------------------------------+\n");
    printf("|              SAISIE DES TACHES                 |\n");
    printf("+------------------------------------------------+\n");
    
    printf("Combien de taches voulez-vous saisir ? ");
    scanf("%d", &systeme->nb_taches);
    
    systeme->taches = malloc(systeme->nb_taches * sizeof(Tache));
    
    for (int i = 0; i < systeme->nb_taches; i++) {
        int c, d, t, p = 0;
        
        printf("\n--- Tache %d ---\n", i + 1);
        printf("Duree d'execution (C) : ");
        scanf("%d", &c);
        printf("Echeance relative (D) : ");
        scanf("%d", &d);
        printf("Periode (T) : ");
        scanf("%d", &t);
        
        // Pour HPF, demander la priorité
        if (systeme->algorithme == HPF) {
            printf("Priorite (P) [plus grand = plus prioritaire] : ");
            scanf("%d", &p);
        }
        
        initialiser_tache(&systeme->taches[i], i + 1, c, d, t, p);
    }
    
    // Calculer les priorités pour RM et DM
    if (systeme->algorithme == RM) {
        calculer_priorites_rm(systeme);
        printf("\n>> Priorites calculees selon Rate Monotonic (periode plus courte = priorite plus haute)\n");
    } else if (systeme->algorithme == DM) {
        calculer_priorites_dm(systeme);
        printf("\n>> Priorites calculees selon Deadline Monotonic (echeance plus courte = priorite plus haute)\n");
    }
}

// Fonction pour choisir l'algorithme d'ordonnancement
TypeOrdonnanceur choisir_algorithme() {
    int choix;
    
    printf("\n+------------------------------------------------+\n");
    printf("|          CHOIX DE L'ALGORITHME                 |\n");
    printf("+------------------------------------------------+\n");
    printf("| 1. HPF - Highest Priority First                |\n");
    printf("| 2. RM  - Rate Monotonic                        |\n");
    printf("| 3. DM  - Deadline Monotonic                    |\n");
    printf("+------------------------------------------------+\n");
    printf("Votre choix (1-3) : ");
    
    scanf("%d", &choix);
    
    switch (choix) {
        case 1: return HPF;
        case 2: return RM;
        case 3: return DM;
        default: 
            printf("Choix invalide, utilisation de HPF par defaut.\n");
            return HPF;
    }
}

// Fonction pour utiliser les données de l'exercice 1
void utiliser_donnees_exercice1(Systeme *systeme) {
    systeme->nb_taches = 3;
    systeme->taches = malloc(3 * sizeof(Tache));
    
    // Données de l'exercice
    initialiser_tache(&systeme->taches[0], 1, 2, 7, 7, 20);   // Thread 1
    initialiser_tache(&systeme->taches[1], 2, 3, 11, 11, 15); // Thread 2  
    initialiser_tache(&systeme->taches[2], 3, 5, 13, 13, 10); // Thread 3
    
    // Recalculer les priorités selon l'algorithme choisi
    if (systeme->algorithme == RM) {
        calculer_priorites_rm(systeme);
    } else if (systeme->algorithme == DM) {
        calculer_priorites_dm(systeme);
    }
}

int main() {
    printf("=== ORDONNANCEUR TEMPS REEL ===\n");
    
    Systeme systeme;
    int mode;
    
    // Choix du mode
    printf("\n+------------------------------------------------+\n");
    printf("|                CHOIX DU MODE                   |\n");
    printf("+------------------------------------------------+\n");
    printf("| 1. Utiliser les donnees de l'exercice 1        |\n");
    printf("| 2. Saisir de nouvelles taches                  |\n");
    printf("+------------------------------------------------+\n");
    printf("Votre choix (1-2) : ");
    scanf("%d", &mode);
    
    // Choix de l'algorithme
    systeme.algorithme = choisir_algorithme();
    
    // Configuration des tâches
    if (mode == 1) {
        utiliser_donnees_exercice1(&systeme);
        printf("\n>> Utilisation des donnees de l'exercice 1\n");
    } else {
        saisir_taches(&systeme);
    }
    
    // Affichage de la configuration
    printf("\n+------------------------------------------------+\n");
    printf("|              CONFIGURATION DU SYSTEME          |\n");
    printf("+------------------------------------------------+\n");
    printf("| Algorithme: %-35s |\n", nom_algorithme(systeme.algorithme));
    printf("+------------------------------------------------+\n");
    
    for (int i = 0; i < systeme.nb_taches; i++) {
        printf("| Tache %-2d: C=%-2d, D=%-2d, T=%-2d, P=%-2d               |\n", 
               systeme.taches[i].id, systeme.taches[i].duree_execution, 
               systeme.taches[i].echeance, systeme.taches[i].periode, 
               systeme.taches[i].priorite);
    }
    printf("+------------------------------------------------+\n\n");

    // Lancement de la simulation
    bool resultat = simuler_ordonnancement(&systeme);
    
    // Affichage du résultat
    printf("\n\n+------------------------------------------------+\n");
    printf("|                   RESULTAT                     |\n");
    printf("+------------------------------------------------+\n");
    if (resultat) {
        printf("| Status: FAISABLE                               |\n");
        printf("| Toutes les taches peuvent etre terminees       |\n");
        printf("| simultanement avec l'ordonnanceur %-12s |\n", 
               systeme.algorithme == HPF ? "HPF." :
               systeme.algorithme == RM ? "RM." : "DM.");
    } else {
        printf("| Status: NON FAISABLE                           |\n");
        printf("| Impossible de terminer toutes les taches       |\n");
        printf("| simultanement avec l'ordonnanceur %-12s |\n",
               systeme.algorithme == HPF ? "HPF." :
               systeme.algorithme == RM ? "RM." : "DM.");
    }
    printf("+------------------------------------------------+\n\n");
    
    // Libération de la mémoire
    free(systeme.taches);
    
    return 0;
}