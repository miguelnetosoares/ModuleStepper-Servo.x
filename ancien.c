
/*
 * File:   main.c
 * Author: Miguel
 *
 * Created on 2. juin 2016, 19:52
 */

#include <htc.h>
#include <xc.h>

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

// La position du servo-moteur va de 0 à 250
#define SERVO_MAX 250

/**
* Les 2 bits moins signifiants du rapport cyclique.
* Sont copiés dans CCP5CON.DC5B
*/
char PWM_ccpr2LSB;

/**
* Les 8 bits plus signifants du rapport cyclique.
* Sont copiés dans CCPR5L.
*/
char PWM_ccpr8MSB;


/**
* Calcule la configuration du PWM pour la position indiquée.
* @param position Valeur entre 0 et 250, la position du
* servo-moteur.
*/
void PWM_configure(char position)
{
    int pwm;
    // Calcule le rapport cyclique selon la position:
    pwm = position + 250;
    // Sépare le rapport cyclique en 2 + 6 bits:
    PWM_ccpr2LSB = pwm & 0x03;
    PWM_ccpr8MSB = pwm >> 2;
    // Configure le générateur PWM:
    CCPR5L = PWM_ccpr8MSB;
    CCP5CONbits.DC5B = PWM_ccpr2LSB;
}

/**
* Position du servo-moteur.
* Entre 0 et SERVO_MAX.
*/
unsigned char SERVO_position;

/**
* Gère la position du servo.
* @param deplacement Nombre d'unités que le servomoteur doit
* se déplacer.
*/
void SERVO_deplace(char deplacement)
{
    // Déplace le servo:
    SERVO_position += deplacement;
    // Vérifie le dépassement:
    if (SERVO_position > SERVO_MAX)
    {
        SERVO_position = SERVO_MAX;
    }
    // Configure le PWM:
    PWM_configure(SERVO_position);
}

/**
* Routine de gestion des interruptions de basse priorité.
*/
void interrupt low_priority interruptionsBP()
{
    // Detecte de quel type d'interruption il s'agit:
    // Test si le drapeau a été activer sur la ligne externe INT2
    if (INTCON3bits.INT2IF)
    {
        INTCON3bits.INT2IF=0; // Drapeau remit à zéro
        SERVO_deplace(-1);
    }
    // Test si le drapeau a été activer sur la ligne externe INT1
    if (INTCON3bits.INT1IF)
    {
        INTCON3bits.INT1IF = 0; // Drapeau remit à zéro
        SERVO_deplace(+1);
    }
}

/**
* Établit la position du servo-moteur.
* @param position
*/
void SERVO_place(char position)
{
    // Vérifie le dépassement:
    if (position > SERVO_MAX)
    {
        SERVO_position = SERVO_MAX;
    }
    else
    {
        SERVO_position = position;
    }
    // Configure le PWM:
    PWM_configure(position);
}

/**
* Gére la séquence de 10 étapes de 2ms pour générer le signal
* complet de 20ms.
*/
void PWM_gereSequence()
{
    static char n = 0;
    
    // Établit le rapport cyclique selon la séquence:
    switch(n)
    {
        case 0:
            CCPR5L = PWM_ccpr8MSB;
            CCP5CONbits.DC5B = PWM_ccpr2LSB;
            break;
        default:
            CCPR5L = 0;
            CCP5CONbits.DC5B = 0;
    }
    // Suivante étape dans la séquence:
    n++;
    if (n > 9)
    {
        n = 0;
    }
}

/**
* Routine de gestion des interruptions de haute priorité.
*/
void interrupt interruptionsHP()
{
    // Detecte de quel type d'interruption il s'agit:
    // Test si le drapeau a été activer sur le timer 2
    if (PIR1bits.TMR2IF)
    {
        PIR1bits.TMR2IF = 0; // Drapeau remit à zéro
        PWM_gereSequence();
    }
}

/**
* Point d'entrée du programme.
* Configure le tmr 2 pour produire des interruptions et un PWM de
* 2ms de période. Configure également les interruptions
* INT1 et INT2 en basse priorité.
*/
void main()
{
    ANSELA = 0x00; // Désactive les convertisseurs A/D.
    ANSELB = 0x00; // Désactive les convertisseurs A/D.
    ANSELC = 0x00; // Désactive les convertisseurs A/D.
    // Active le PWM sur CCP5:
    CCPTMRS1bits.C5TSEL = 0; // CCP5 branché sur tmr2
    T2CONbits.T2CKPS = 1; // Diviseur de fréquence tmr2 sur 32
    T2CONbits.TMR2ON = 1; // Active le tmr2
    PR2 = 125; // Période du tmr2.
    CCP5CONbits.CCP5M = 0xF; // Active le CCP5.
    TRISAbits.RA4 = 0; // Active la sortie du CCP5.
    // Prépare les interruptions de haute priorité tmr2:
    PIE1bits.TMR2IE = 1; // Active les interruptions.
    IPR1bits.TMR2IP = 1; // En haute priorité.
    PIR1bits.TMR2IF = 0; // Baisse le drapeau.
    // Prépare les interruptions de basse priorité INT1 et INT2:
    TRISBbits.RB2 = 1; // INT2 comme entrée digitale.
    TRISBbits.RB1 = 1; // INT1 comme entrée digitale.
    INTCON2bits.RBPU=0; // Active les résistances de tirage...
    WPUBbits.WPUB2 = 1; // ... sur INT2 ...
    WPUBbits.WPUB1 = 1; // ... et INT1.
    INTCON2bits.INTEDG2 = 0; // Int. de INT2 sur flanc descendant.
    INTCON2bits.INTEDG1 = 0; // Int. de INT1 sur flanc descendant.
    INTCON3bits.INT2IE = 1; // Active les interruptions pour INT2...
    INTCON3bits.INT2IP = 0; // ... en basse priorité.
    INTCON3bits.INT1IE = 1; // Active les interriptions pour INT1...
    INTCON3bits.INT1IP = 0; // ... en basse priorité.
    // Active les interruptions de haute et de basse priorité:
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
    // Initialise le PWM au point central:
    SERVO_place(SERVO_MAX / 2);
    // Dodo
    while(1);
}
