#include <xc.h>
#include "pwm.h"

#define PWM_ESPACEMENT 6

/** Valeur du canal. */
static unsigned char valeurCanal[1];

/** Valeur pour totaliser le nombre d'espacement pour le PWM.*/
/** Pour être compatible avec la norme radio contrôle.*/
static unsigned char nbEspacement = 0;

/**
 * Convertit une valeur signée générique vers une valeur directement
 * utilisable pour la génération PWM.
 * @param valeur Une valeur entre 0 et 255.
 * @return Une valeur entre 62 et 125.
 */
unsigned char pwmConversion(unsigned char valeurGenerique) {
    return (valeurGenerique / 4) + 62; // 0 à 255 devient 62 à 125.
}

/**
 * Établit la valeur du canal spécifié par {@link #pwmPrepareValeur}.
 * @param valeur La valeur du canal.
 */
void pwmEtablitValeur(unsigned char valeur) {
    
    unsigned char valeurConvertit;
    
    valeurConvertit = pwmConversion(valeur); // Conversion 0/255 => 62/125.
    // Valeur du convertisseur A/D pour le rapport cyclique PWM.*/
    valeurCanal[0] = valeurConvertit;
}

/**
 * Rend la valeur PWM correspondante au canal.
 * @param canal Le numéro de canal.
 * @return La valeur PWM correspondante au canal.
 */
unsigned char pwmValeur() {
    return valeurCanal[0]; // Rapport cyclique pour le PWM 1
    
}

/**
 * Indique si il est temps d'émettre une pulsation PWM.
 * Sert à espacer les pulsation PWM pour les rendre compatibles
 * avec la norme de radio contrôle.
 * @return 255 si il est temps d'émettre une pulse. 0 autrement.
 */
unsigned char pwmEspacement() {
    // Si le nb d'espacement est plus petit que le nb total il retourne 0.
    if (nbEspacement < PWM_ESPACEMENT) { 
        nbEspacement++;
        return 0;
    }
    // Sinon il retourne 255.
    else {
        return 255;
    }
}

/**
 * Réinitialise le système PWM.
 */
void pwmReinitialise() {
    
    valeurCanal[0] = 0;
    nbEspacement = 0;

}
