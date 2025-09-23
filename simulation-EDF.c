#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// Structure pour représenter une tâche temps réel
typedef struct {
    int id;                    // Identifiant de la tâche
    int duree_execution;       // Durée d'exécution (C)
    int echeance;             // Échéance relative (D)
    int periode;              // Période (T)
    
    // Variables pour la simulation EDF
    int temps_restant;        // Temps d'exécution restant pour l'instance courante
    int prochaine_activation; // Prochaine activation de la tâche
    int echeance_absolue;     // Échéance absolue courante
    int instance;             // Numéro de l'instance courante
    bool active;              // Tâche actuellement active
} Tache;

// Structure pour stocker l'état du système
typedef struct {
    Tache *taches;
    int nb_taches;
} Systeme;

// Fonction pour initialiser une tâche
void initialiser_tache(Tache *tache, int id, int duree, int echeance, int periode) {
    tache->id = id;
    tache->duree_execution = duree;
    tache->echeance = echeance;
    tache->periode = periode;
    tache->temps_restant = 0;
    tache->prochaine_activation = 0;
    tache->echeance_absolue = echeance;
    tache->instance = 0;
    tache->active = false;
}

// Fonction pour trouver la tâche avec l'échéance absolue la plus proche (EDF)
int trouver_tache_echeance_plus_proche(Systeme *systeme) {
    int tache_selectionnee = -1;
    int echeance_min = INT_MAX;
    
    for (int i = 0; i < systeme->nb_taches; i++) {
        Tache *tache = &systeme->taches[i];
        
        // Vérifier si la tâche est active et a du temps d'exécution restant
        if (tache->active && tache->temps_restant > 0) {
            if (tache->echeance_absolue < echeance_min) {
                echeance_min = tache->echeance_absolue;
                tache_selectionnee = i;
            }
        }
    }
    
    return tache_selectionnee;
}

// Fonction pour gérer les événements d'ordonnancement à un instant donné
bool gerer_evenements_ordonnancement(Systeme *systeme, int temps_actuel) {
    bool evenement_ordonnancement = false;
    bool echeance_manquee = false;
    
    printf("    [Evenements d'ordonnancement a t=%d]\n", temps_actuel);
    
    // ETAPE 1: Vérifier les fins de tâches en cours (avant les nouvelles activations)
    for (int i = 0; i < systeme->nb_taches; i++) {
        Tache *tache = &systeme->taches[i];
        if (tache->active && tache->temps_restant == 0) {
            printf("    >> TERMINAISON: Tache %d (instance %d) completee\n", 
                   tache->id, tache->instance);
            tache->active = false;
            evenement_ordonnancement = true;
        }
    }
    
    // ETAPE 2: Gérer les nouvelles activations
    for (int i = 0; i < systeme->nb_taches; i++) {
        Tache *tache = &systeme->taches[i];
        
        // Vérifier si c'est le moment d'activer la tâche
        if (temps_actuel == tache->prochaine_activation) {
            // Vérifier si l'instance précédente n'est pas terminée (échéance manquée)
            if (tache->active && tache->temps_restant > 0) {
                printf("    >> ECHEC: Tache %d (instance %d, temps restant: %d) - echeance manquee\n", 
                       tache->id, tache->instance, tache->temps_restant);
                echeance_manquee = true;
            }
            
            // Lancement d'une nouvelle tâche
            tache->instance++;
            tache->temps_restant = tache->duree_execution;
            tache->active = true;
            tache->echeance_absolue = temps_actuel + tache->echeance;
            
            printf("    >> ACTIVATION: Tache %d (instance %d) - echeance absolue: %d\n", 
                   tache->id, tache->instance, tache->echeance_absolue);
            
            // Programmer la prochaine activation
            tache->prochaine_activation += tache->periode;
            evenement_ordonnancement = true;
        }
    }
    
    if (!evenement_ordonnancement) {
        printf("    (Aucun evenement d'ordonnancement)\n");
    }
    
    return !echeance_manquee;
}

// Fonction principale de simulation EDF avec traçage détaillé
bool simuler_edf_avec_tracage(Systeme *systeme, int duree_simulation) {
    printf("\n");
    printf("==========================================\n");
    printf("          SIMULATION EDF                  \n");
    printf("     (Earliest Deadline First)           \n");
    printf("==========================================\n");
    
    printf("\nPRINCIPE EDF:\n");
    printf("A chaque evenement d'ordonnancement, selectionner la tache\n");
    printf("avec l'echeance absolue la plus proche.\n\n");
    
    int temps = 0;
    bool simulation_reussie = true;
    int tache_precedente = -1;
    
    while (temps < duree_simulation) {
        printf("\n--- TEMPS t = %d ---\n", temps);
        
        // Étape 1: Gérer les événements d'ordonnancement
        if (!gerer_evenements_ordonnancement(systeme, temps)) {
            simulation_reussie = false;
        }
        
        // Étape 2: Appliquer l'algorithme EDF
        int tache_courante = trouver_tache_echeance_plus_proche(systeme);
        
        // Détection de préemption
        if (tache_precedente != -1 && tache_courante != tache_precedente && tache_courante != -1) {
            printf("    >> PREEMPTION: Tache %d interrompue par Tache %d\n", 
                   systeme->taches[tache_precedente].id, systeme->taches[tache_courante].id);
        }
        
        if (tache_courante != -1) {
            Tache *tache = &systeme->taches[tache_courante];
            tache->temps_restant--;
            
            printf("    >> EXECUTION: Tache %d (instance %d)\n", tache->id, tache->instance);
            printf("       - Echeance absolue: %d\n", tache->echeance_absolue);
            printf("       - Temps restant: %d\n", tache->temps_restant);
            
            if (tache->temps_restant == 0) {
                printf("       - TERMINEE!\n");
            }
        } else {
            printf("    >> PROCESSEUR INACTIF\n");
        }
        
        // Affichage de l'état de la file d'attente
        printf("    >> ETAT DE LA FILE D'ATTENTE:\n");
        bool file_vide = true;
        for (int i = 0; i < systeme->nb_taches; i++) {
            if (systeme->taches[i].active && systeme->taches[i].temps_restant > 0) {
                printf("       - Tache %d: echeance=%d, restant=%d\n", 
                       systeme->taches[i].id, systeme->taches[i].echeance_absolue, 
                       systeme->taches[i].temps_restant);
                file_vide = false;
            }
        }
        if (file_vide) {
            printf("       - (File vide)\n");
        }
        
        tache_precedente = tache_courante;
        temps++;
    }
    
    printf("\n--- FIN DE SIMULATION a t=%d ---\n", duree_simulation);
    return simulation_reussie;
}

// Fonction pour définir l'exemple de tâches de démonstration
void definir_exemple_taches(Systeme *systeme) {
    printf("\nEXEMPLE DE TACHES POUR DEMONSTRATION:\n");
    printf("+--------+-----+---------+---------+\n");
    printf("| Tache  |  C  |    D    |    T    |\n");
    printf("+--------+-----+---------+---------+\n");
    printf("|   T1   |  2  |    5    |    5    |\n");
    printf("|   T2   |  1  |    3    |    6    |\n");
    printf("|   T3   |  1  |    7    |    8    |\n");
    printf("+--------+-----+---------+---------+\n");
    
    systeme->nb_taches = 3;
    systeme->taches = malloc(3 * sizeof(Tache));
    
    // Tâche 1: C=2, D=5, T=5
    initialiser_tache(&systeme->taches[0], 1, 2, 5, 5);
    
    // Tâche 2: C=1, D=3, T=6  
    initialiser_tache(&systeme->taches[1], 2, 1, 3, 6);
    
    // Tâche 3: C=1, D=7, T=8
    initialiser_tache(&systeme->taches[2], 3, 1, 7, 8);
    
    printf("\nFACTEUR D'UTILISATION:\n");
    double utilisation = 0.0;
    for (int i = 0; i < systeme->nb_taches; i++) {
        double u_i = (double)systeme->taches[i].duree_execution / systeme->taches[i].periode;
        printf("   U%d = %d/%d = %.3f\n", systeme->taches[i].id, 
               systeme->taches[i].duree_execution, systeme->taches[i].periode, u_i);
        utilisation += u_i;
    }
    printf("   U_total = %.3f\n", utilisation);
    printf("   Condition EDF: U <= 1.0 => %s\n", 
           utilisation <= 1.0 ? "RESPECTEE" : "VIOLEE");
}

// Fonction pour afficher le résultat final
void afficher_resultat_final(bool simulation_reussie) {
    printf("\n");
    printf("==========================================\n");
    printf("           RESULTAT FINAL                 \n");
    printf("==========================================\n");
    
    if (simulation_reussie) {
        printf("STATUT: SIMULATION REUSSIE\n");
        printf("Toutes les echeances ont ete respectees.\n");
        printf("L'ordonnancement EDF a permis de satisfaire\n");
        printf("toutes les contraintes temporelles.\n");
    } else {
        printf("STATUT: SIMULATION ECHOUEE\n");
        printf("Au moins une echeance n'a pas ete respectee.\n");
        printf("Le systeme de taches n'est pas ordonnancable\n");
        printf("avec l'algorithme EDF dans cette configuration.\n");
    }
    
    printf("==========================================\n");
}

int main() {
    printf("==========================================\n");
    printf("          EXERCICE 2 - EDF                \n");
    printf("      Simulateur d'Ordonnancement         \n");
    printf("      Earliest Deadline First             \n");
    printf("==========================================\n");
    
    Systeme systeme;
    
    // 1) Développer un simulateur proposant EDF comme ordonnanceur
    printf("\nOBJECTIF: Developper un simulateur EDF\n");
    
    // Définir l'exemple de tâches
    definir_exemple_taches(&systeme);
    
    // 2) Vérifier l'application en traçant l'exécution
    printf("\nVERIFICATION: Tracage de l'execution\n");
    
    // Demander la durée de simulation
    int duree_simulation;
    printf("\nDuree de simulation souhaitee (recommande: 24) : ");
    scanf("%d", &duree_simulation);
    
    // Lancer la simulation avec traçage complet
    bool resultat = simuler_edf_avec_tracage(&systeme, duree_simulation);
    
    // Afficher le résultat final
    afficher_resultat_final(resultat);
    
    // Libération de la mémoire
    free(systeme.taches);
    
    return 0;
}
