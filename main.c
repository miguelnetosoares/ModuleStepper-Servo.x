#include <xc.h>
#include "servo.h"
#include "pwm.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.

// Sorties des PWM:
#pragma config CCP3MX = PORTC6  // Sortie PA3 connectée sur PORTC6.

/**
 * Point d'entrée des interruptions basse priorité.
 */
void low_priority interrupt interruptionsBassePriorite() {
        servoInterruptions();  
}

/**
 * Point d'entrée.
 * Lance le programme du servo.
 */
void main(void) {
    
    TRISBbits.RB4 = 1;      // RB4 (AN11) configuré en entrée.  
    ANSELBbits.ANSB4 = 0;   // RB4 (AN11) configuré en digitale.
    
    servoMain();
    
    while(1);
}