#include <xc.h>
#include "file.h"
#include "i2c.h"

#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.


typedef enum {
    AVANT = 0b01,
    ARRIERE = 0b10,
    ARRET = 0b00
} Direction;


static void esclaveInitialiseHardware() {
   // Active le MSSP1 en mode Esclave I2C:
    TRISCbits.RC3 = 1;          // RC3 comme entrée...
    ANSELCbits.ANSC3 = 0;       // ... digitale.
    TRISCbits.RC4 = 1;          // RC4 comme entrée...
    ANSELCbits.ANSC4 = 0;       // ... digitale.

    SSP1CON1bits.SSPEN = 1;     // Active le module SSP.    
    
    SSP1ADD = ECRITURE_SERVO_0;   // Adresse de l'esclave.
    SSP1MSK = I2C_MASQUE_ADRESSES_ESCLAVES;
    SSP1CON1bits.SSPM = 0b1110; // SSP1 en mode esclave I2C avec adresse de 7 bits et interruptions STOP et START.
        
    SSP1CON3bits.PCIE = 0;      // Désactive l'interruption en cas STOP.
    SSP1CON3bits.SCIE = 0;      // Désactive l'interruption en cas de START.
    SSP1CON3bits.SBCDE = 0;     // Désactive l'interruption en cas de collision.

    PIE1bits.SSP1IE = 1;        // Interruption en cas de transmission I2C...
    IPR1bits.SSP1IP = 0;        // ... de basse priorité.
    
    // Active les interruptions générales:
    // À faire...
    RCONbits.IPEN = 1; // Active les niveaux de priorité pour les interruptions
    INTCONbits.GIEL = 1; // Les interruptions de basse priorité sont activées
    INTCONbits.GIEH = 1; // Toutes les interruptions sont activées
      
 }

static void stepperInitialiseHardware() {
    // Initialise 
    T2CONbits.T2CKPS = 1;       // Diviseur de fréquence 1:4
    T2CONbits.T2OUTPS = 0;      // Pas de diviseur de fréquence à la sortie.
    T2CONbits.TMR2ON = 1;       // Active le temporisateur.
    PIE1bits.TMR2IE = 1;        // Active les interruptions ...
    IPR1bits.TMR2IP = 0;        // ... de basse priorité ...
    PIR1bits.TMR2IF = 0;        // ... pour le temporisateur 2.
    PR2 = 138;                   // Période du tmr2.
    
    // Configure PWM 1 pour émettre un signal de 1KHz:
    // À faire...
    ANSELCbits.ANSC2 = 0;   // Désactive A/D
    TRISCbits.RC2 = 0;      // Active la sortie RC2 en digital   
    CCPTMRS0bits.C1TSEL = 0;// CCP1 sur tmr2
    CCP1CONbits.P1M = 0;    // Simple PWM sur la sortie P1A
    CCP1CONbits.CCP1M = 12; // Active le CCP1
    PR2 = 255;              // Période est 1ms plus une marge de sécurité.
    CCPR2H=138;             // Limitation puissance à 56%
                            // (Proteus n'aime pas que CCPRxL dépasse PRx)
    
    // Configure RC0 et RC1 pour gérer la direction du moteur:
    // À faire...
    TRISCbits.RC0 = 0; // Le bit 0 du port C est configuré en sortie
    TRISCbits.RC1 = 0; // Le bit 1 du port C est configuré en sortie
    
    // Active les interruptions générales:
    // À faire...
    RCONbits.IPEN = 1; // Active les niveaux de priorité pour les interruptions
    INTCONbits.GIEL = 1; // Les interruptions de basse priorité sont activées
    INTCONbits.GIEH = 1; // Toutes les interruptions sont activées
}


void stepper( unsigned char valeur){
    
    stepperInitialiseHardware();
    calculDistance(valeur);
    
}

void servo(unsigned char valeur) {
    // Rien...
}

Direction conversionDirection(unsigned char v) {
    
    if (v > 0) {
        return AVANT;
    }
    else {
        return ARRIERE;
    }
}

void receptionI2C(unsigned char adresse, unsigned char valeur){
    if (adresse==13){
        stepper(valeur);
    }
    if (adresse==14){
        servo(valeur);
    }  
}

void low_priority interrupt interruptionsBassePriorite(){
    
// interuption pour i2c
 if (PIR1bits.SSP1IF) {
        i2cEsclave();
    }
 
 // interuption pour stepper
 
 if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        ADCON0bits.GO = 1;
    }
 if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        PORTC = conversionDirection(ADRESH);
        CCPR1L = ADRESH;
    }
 
}

void main(void) {
    
    esclaveInitialiseHardware();
    i2cReinitialise();
    i2cRappelCommande(receptionI2C);
         
    TRISBbits.RB4 = 1;      // RB4 (AN11) configuré en entrée.  
    ANSELBbits.ANSB4 = 0;   // RB4 (AN11) configuré en digitale.
    
    servoMain();
    
    while(1);
}