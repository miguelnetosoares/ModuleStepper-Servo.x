#include <xc.h>
#include "pwm.h"

/**
 * Initialise le hardware pour le servo.
 */
static void servoInitialiseHardware() {
    
    // Interruptions INT2:
    TRISBbits.RB2 = 1;      // Port RB2 comme entrée...
    ANSELBbits.ANSB2 = 0;   // ... digitale.    
    INTCON2bits.RBPU = 0;   // Active les résistances de tirage...
    WPUBbits.WPUB2 = 1;     // ... et INT2.    
    INTCON3bits.INT2E = 1;  // INT2
    INTCON2bits.INTEDG2 = 0;// Flanc descendant.

    // Prépare Temporisateur 6 pour PWM (compte jusqu'à 125 en 2ms):
    T6CONbits.T6CKPS = 1;       // Diviseur de fréquence 1:4
    T6CONbits.T6OUTPS = 0;      // Pas de diviseur de fréquence à la sortie.
    T6CONbits.TMR6ON = 1;       // Active le temporisateur.
    PIE5bits.TMR6IE = 1;        // Active les interruptions ...
    IPR5bits.TMR6IP = 0;        // ... de basse priorité ...
    PIR5bits.TMR6IF = 0;        // ... pour le temporisateur 6.


    // Configure PWM 1 pour émettre le signal de radio-contrôle:
    ANSELCbits.ANSC2 = 0;   // AN14 (RC2) configuré en entrée/sortie digitale
    TRISCbits.RC2 = 0;      // RC2 en sortie (EUSART peut prendre le contrôle)    
    CCP1CONbits.P1M = 0;    // Simple PWM sur la sortie P1A
    CCP1CONbits.CCP1M = 12; // Active le CCP1 comme PWM.
    CCPTMRS0bits.C1TSEL = 2;    // Branche le CCP1 sur le temporisateur 6.
    PR2 = 200;// Période 2ms plus une marge.(Proteus n'aime pas CCPRxL >> PRx)

    // Active le module de conversion A/D:
    TRISBbits.RB3 = 1;      // Active RB3 comme entrée.
    ANSELBbits.ANSB3 = 1;   // Active AN09 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.
    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.

    // Active les interruptions générales:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

/**
 * Point d'entrée des interruptions pour le servo.
 */
void servoInterruptions() {

    if (PIR5bits.TMR6IF) {// Débordement du timer 6 (Interruption : drapeau =1)
        PIR5bits.TMR6IF = 0;  // On remet le drapeau à 0
        if (pwmEspacement()) {
            CCPR1L = pwmValeur();
        } else {
            CCPR1L = 0;
        }
    }

    if (INTCON3bits.INT2F) {    // Interruption sur la ligne externe INT2
        INTCON3bits.INT2F = 0;  // On remet le drapeau à 0
        ADCON0bits.GO = 1;      // Lance une conversion A/D
    }
    
    if (PIR1bits.ADIF) { // Conversion A/D terminé (Interruption : drapeau =1)
        PIR1bits.ADIF = 0;      // On remet le drapeau à 0
        pwmEtablitValeur(ADRESH);
    }
}

/**
 * Point d'entrée pour le servo de radio contrôle.
 */
void servoMain(void) {
    servoInitialiseHardware();
    pwmReinitialise();

    while(1);
}
