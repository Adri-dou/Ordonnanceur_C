#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Structure pour représenter une tâche temps réel
typedef struct {
    int id;                 // Identifiant de la tâche
    int duree_execution;       // Durée d'exécution (C)
    int echeance;            // Échéance relative (D)
    int periode;            // Période (T)
    int priorite;            // Priorité (plus grand = haute priorité)
    
    // Variables pour la simulation
    int temps_restant;      // Temps d'exécution restant pour l'instance courante
    int prochaine_activation; // Prochaine activation de la tâche
    int prochaine_echeance;  // Prochaine échéance absolue
} Tache;

// Structure pour stocker l'état du système
typedef struct {
    Tache *taches;
    int nb_taches;
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

// Fonction pour trouver la tâche avec la plus haute priorité prête à s'exécuter
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

// Fonction principale de simulation HPF simplifiée
bool simuler_hpf(Systeme *systeme) {
    printf("+------------------------------------------------+\n");
    printf("|         SIMULATION HPF PREEMPTIF               |\n");
    printf("+------------------------------------------------+");
    
    int temps = 0;
    
    while (temps < 100) { // Protection contre boucle infinie
        printf("\n[t = %d]\n", temps);
        
        // Étape 1: Activer les nouvelles instances de tâches
        if (!activer_taches(systeme, temps)) {
            printf("\n\t >> SIMULATION ARRETEE: Echeance manquee !\n");
            return false;
        }
        
        // Étape 2: Sélectionner la tâche avec la plus haute priorité
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

int main() {
    printf("=== EXERCICE 1: VERIFICATION DE FAISABILITE HPF ===\n\n");
    
    // Initialisation du système avec les données de l'exercice
    const int NB_TACHES = 3;
    Systeme systeme;
    systeme.nb_taches = NB_TACHES;
    systeme.taches = malloc(NB_TACHES * sizeof(Tache));
    
    // Données de l'exercice
    initialiser_tache(&systeme.taches[0], 1, 2, 7, 7, 20);   // Thread 1
    initialiser_tache(&systeme.taches[1], 2, 3, 11, 11, 15); // Thread 2  
    initialiser_tache(&systeme.taches[2], 3, 5, 13, 13, 10); // Thread 3
    
    // Affichage de la configuration
    printf("+------------------------------------------------+\n");
    printf("|              CONFIGURATION DU SYSTEME          |\n");
    printf("+------------------------------------------------+\n");
    for (int i = 0; i < NB_TACHES; i++) {
        printf("| Tache %-2d: C=%-2d, D=%-2d, T=%-2d, P=%-2d               |\n", 
               systeme.taches[i].id, systeme.taches[i].duree_execution, 
               systeme.taches[i].echeance, systeme.taches[i].periode, 
               systeme.taches[i].priorite);
    }
    printf("+------------------------------------------------+\n\n");

    // Lancement de la simulation
    bool resultat = simuler_hpf(&systeme);
    
    // Affichage du résultat
    printf("\n\n+------------------------------------------------+\n");
    printf("|                   RESULTAT                     |\n");
    printf("+------------------------------------------------+\n");
    if (resultat) {
        printf("| Status: FAISABLE                               |\n");
        printf("| Toutes les taches peuvent etre terminees       |\n");
        printf("| simultanement avec l'ordonnanceur HPF.         |\n");
    } else {
        printf("| Status: NON FAISABLE                           |\n");
        printf("| Impossible de terminer toutes les taches       |\n");
        printf("| simultanement avec l'ordonnanceur HPF.         |\n");
    }
    printf("+------------------------------------------------+\n\n");
    
    // Libération de la mémoire
    free(systeme.taches);
    
    return 0;
}