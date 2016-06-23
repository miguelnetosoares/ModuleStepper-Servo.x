/*
 * File:   stepper.c
 * Author: Morgado
 *
 * Created on 9. juin 2016, 19:56
 */


#include <xc.h>

typedef enum {
    AVANT = 0b01,
    ARRIERE = 0b10
} Direction;



Direction conversionDirection(unsigned char v) {
    // À implémenter...
    if (v > 127) {
        return AVANT;
    }
    else {
        return ARRIERE;
    }
}

unsigned char conversionMagnitude(unsigned char v) {
    // À implémenter...
    unsigned char PWM;
    
    if (v < 127) {
        PWM = 254 - (v * 2);
    }
    else if (v == 127 || v == 128) {
        PWM = 0;
    }
    else {
        PWM = (v - 128) * 2;
    }
    return PWM;
}


static void hardwareInitialise() {
  T2CONbits.T2CKPS = 1;       // Diviseur de fréquence 1:4
    T2CONbits.T2OUTPS = 0;      // Pas de diviseur de fréquence à la sortie.
    T2CONbits.TMR2ON = 1;       // Active le temporisateur.
    PIE1bits.TMR2IE = 1;        // Active les interruptions ...
    IPR1bits.TMR2IP = 0;        // ... de basse priorité ...
    PIR1bits.TMR2IF = 0;        // ... pour le temporisateur 2.
    PR2 = 62;                   // Période du tmr2.
    
    // Configure PWM 1 pour émettre un signal de 1KHz:
    // À faire...
    ANSELCbits.ANSC2 = 0;   // Désactive A/D
    TRISCbits.RC2 = 0;      // Active la sortie RC2 en digital   
    CCPTMRS0bits.C1TSEL = 0;// CCP1 sur tmr2
    CCP1CONbits.P1M = 0;    // Simple PWM sur la sortie P1A
    CCP1CONbits.CCP1M = 12; // Active le CCP1
    PR2 = 255;              // Période est 1ms plus une marge de sécurité.
                            // (Proteus n'aime pas que CCPRxL dépasse PRx)
    
    // Configure RC0 et RC1 pour gérer la direction du moteur:
    // À faire...
    TRISCbits.RC0 = 0; // Le bit 0 du port C est configuré en sortie
    TRISCbits.RC1 = 0; // Le bit 1 du port C est configuré en sortie
    
    // Active le module de conversion A/D:
    // À faire...
    TRISBbits.RB3 = 1;      // Active RB4 comme entrée.
    ANSELBbits.ANSB3 = 1;   // Active AN11 comme entrée analogique.
    ADCON0bits.ADON = 1;    // Allume le module A/D.
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN09
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.

    PIE1bits.ADIE = 1;      // Active les interruptions A/D
    IPR1bits.ADIP = 0;      // Interruptions A/D sont de basse priorité.
    
    // Active les interruptions générales:
    // À faire...
    RCONbits.IPEN = 1; // Active les niveaux de priorité pour les interruptions
    INTCONbits.GIEL = 1; // Les interruptions de basse priorité sont activées
    INTCONbits.GIEH = 1; // Toutes les interruptions sont activées
}




void low_priority interrupt interruptionsBassePriorite() {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        ADCON0bits.GO = 1;
    }
    
    if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        PORTC = conversionDirection(ADRESH);
        CCPR1L = conversionMagnitude(ADRESH);
    }
}







void stepper(void) {
    
     hardwareInitialise();

    return;
}
