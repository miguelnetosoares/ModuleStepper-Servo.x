/*
 * File:   PWM_moteur.c
 * Author: Morgado
 *
 * Created on 2. juin 2016, 20:11
 */

#include <xc.h>
#include "test.h"

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

typedef enum {
    AVANT = 0b01,
    ARRIERE = 0b10
} Direction;

/**
 * Indique la direction correspondante à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return AVANT ou ARRIERE.
 */
Direction conversionDirection(unsigned char v) {
    // À implémenter...
    if (v > 127) {
        return AVANT;
    }
    else {
        return ARRIERE;
    }
}

/**
 * Indique le cycle de travail PWM correspondant à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return Cycle de travail du PWM.
 */
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

#ifndef TEST

/**
 * Initialise le hardware.
 */
static void hardwareInitialise() {
    
    // Prépare Temporisateur 2 pour PWM (compte jusqu'à 255 en 1ms):
    // À faire...
    T0CONbits.T0CKPS = 1;       // Diviseur de fréquence 1:4
    T0CONbits.T0OUTPS = 0;      // Pas de diviseur de fréquence à la sortie.
    T0CONbits.TMR0ON = 1;       // Active le temporisateur.
    INTCONbits.INT0IE = 1;        // Active les interruptions ...
    INTCON2bits.INT0IP = 0;        // ... de basse priorité ...
    INTCONbits.INT0IF = 0;        // ... pour le temporisateur 2.
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

/**
 * Point d'entrée des interruptions.
 */
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

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void main(void) {
    hardwareInitialise();

    while(1);
}
#endif

#ifdef TEST
void testConversionMagnitude() {
    testeEgaliteEntiers("CM01", conversionMagnitude(0),   254);
    testeEgaliteEntiers("CM02", conversionMagnitude(1),   252);
    testeEgaliteEntiers("CM03", conversionMagnitude(2),   250);
    
    testeEgaliteEntiers("CM04", conversionMagnitude(125),   4);
    testeEgaliteEntiers("CM05", conversionMagnitude(126),   2);
    
    testeEgaliteEntiers("CM06", conversionMagnitude(127),   0);
    testeEgaliteEntiers("CM07", conversionMagnitude(128),   0);

    testeEgaliteEntiers("CM08", conversionMagnitude(129),   2);
    testeEgaliteEntiers("CM09", conversionMagnitude(130),   4);
    
    testeEgaliteEntiers("CM10", conversionMagnitude(253), 250);
    testeEgaliteEntiers("CM11", conversionMagnitude(254), 252);
    testeEgaliteEntiers("CM12", conversionMagnitude(255), 254);
}
void testConversionDirection() {
    testeEgaliteEntiers("CD01", conversionDirection(  0), ARRIERE);    
    testeEgaliteEntiers("CD02", conversionDirection(  1), ARRIERE);    
    testeEgaliteEntiers("CD03", conversionDirection(127), ARRIERE);    
    testeEgaliteEntiers("CD04", conversionDirection(128), AVANT);
    testeEgaliteEntiers("CD05", conversionDirection(129), AVANT);
    testeEgaliteEntiers("CD06", conversionDirection(255), AVANT);    
}
void main() {
    initialiseTests();
    testConversionMagnitude();
    testConversionDirection();
    finaliseTests();
    while(1);
}
#endif