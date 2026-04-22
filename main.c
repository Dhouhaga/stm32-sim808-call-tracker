#include <stdint.h>
#include <stm32f407xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define MY_PHONE_NUMBER "+1234567890"
#define LED_PIN (1u << 12)
#define UART_TIMEOUT 2000000

void GSM_SendSms(char* PhoneNumber, char* text);
void GPIO_init(void) ;
void USART2_Init(void) ;
void UART2_SendChar(int ch);
void USART2_SendString(char *str);
int uart2_available(void);
unsigned char UART2_GetChar(void);
void delay(int d);
void GSM_MakeCall(char * PhoneNumber);
void configSIM808(void) ;
void GPS_parseInfo(const char* trame, char* latitude, char* longitude, char* altitude, char* date);
int GPS_readInfo(char* buffer, int buffer_size);
int checkIncomingCall(char * PhoneNumber);
void sendLocationViaSMS(char * PhoneNumber);
int main(void);

// ........................................................GPIO
void GPIO_init(void) {
    RCC->AHB1ENR |= (1u << 3); // Active horloge pour GPIOD

    GPIOD->MODER &= ~(1u << 25);
    GPIOD->MODER |= (1u << 24); // PD12 en mode sortie

    GPIOD->OSPEEDR &= ~(1u << 25);
    GPIOD->OSPEEDR &= ~(1u << 24); // Vitesse lente

    GPIOD->PUPDR &= ~(1u << 25);
    GPIOD->PUPDR |= (1u << 24); // Pull-up

    GPIOD->OTYPER &= ~(1u << 12); // Push-pull
    GPIOD->ODR &= ~LED_PIN;    // LED eteinte
}



//........................................................UART

// Configure USART2 pour la communication avec le module GSM
void USART2_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 (TX), PA3 (RX)
    GPIOA->MODER |= (1u << 5);
    GPIOA->MODER &= ~(1u << 4);
    GPIOA->MODER |= (1u << 7);
    GPIOA->MODER &= ~(1u << 6);
    GPIOA->AFR[0] = 0x7700; // AF7 pour USART2

    USART2->CR1 |= 0x200C;  // Enable USART + RX/TX
    USART2->BRR = 0x0681;   // Baudrate ~9600 pour 16 MHz
}

// Envoie un caractere via USART2
void UART2_SendChar(int ch) {
    USART2->DR = (uint32_t)ch;
    while (!(USART2->SR & (1u << 6))); // Attente fin d envoi
}

// Envoie une chaine de caracteres
void USART2_SendString(char *str) {
    while (*str) {
        UART2_SendChar(*str++);
    }
}

// Verifie si il y a un caractere recu
int uart2_available(void) {
    return ((USART2->SR) & USART_SR_RXNE) ? 1 : 0;
}

// Lit un caractere recu
unsigned char UART2_GetChar(void) {
    while (!(USART2->SR & USART_SR_RXNE));
    return (unsigned char)(USART2->DR);
}

// Boucle de delai simple (non precise, bloquante)
void delay(int d) {
    int i;
    for (; d > 0; d--) {
        for (i = 0; i < 2657; i++);
    }
}



//........................................................GSM

// Compose un numero (appel vocal)
void GSM_MakeCall(char * PhoneNumber) {
    GPIOD->ODR &= ~LED_PIN; //led eteint
    delay(3000);
    char *prefix = "ATD";
    char *suffix = ";\r\n";
    size_t len = strlen(prefix) + strlen(PhoneNumber) + strlen(suffix) + 1;

    char *full_message = malloc(len);
    if (full_message == NULL) return;

    strcpy(full_message, prefix);
    strcat(full_message, PhoneNumber);
    strcat(full_message, suffix);

    USART2_SendString(full_message);
    free(full_message);
}

// Envoie un SMS a un numero donne
void GSM_SendSms(char* PhoneNumber, char* text) {
    const char* prefix = "AT+CMGS=\"";
    const char* suffix = "\"\r";
    size_t cmd_len = strlen(prefix) + strlen(PhoneNumber) + strlen(suffix) + 1;

    char* command = malloc(cmd_len);
    if (command == NULL) return;

    strcpy(command, prefix);
    strcat(command, PhoneNumber);
    strcat(command, suffix);

    USART2_SendString("AT+CMGF=1\r");
    USART2_SendString(command);
    free(command);

    {
        char received;
        uint32_t timeout = 2000000;
        int prompt_recu = 0;

        while (timeout--) {
            if (uart2_available()) {
                received = UART2_GetChar();
                if (received == '>') { // Attente du prompt de saisie
                    prompt_recu = 1;
                    break;
                }
            }
        }

        if (prompt_recu) {
            USART2_SendString(text);
            UART2_SendChar(0x1A); // Ctrl+Z : fin du SMS
        } else {
            USART2_SendString("PROMPT non recu\r\n");
        }
    }
}

void configSIM808(void) {
    USART2_SendString("AT+CLIP=1\r\n");      // Affichage numero appelant
    delay(200);
    USART2_SendString("AT+CNMI=2,1,0,0,0\r\n"); // Reception automatique SMS
    delay(200);
    USART2_SendString("AT+CMGF=1\r\n");      // Mode texte pour SMS
    delay(200);
    USART2_SendString("AT&W\r\n");           // Sauvegarde la config
    delay(200);
}

// Verifie si un appel a ete recu du numero donne
int checkIncomingCall(char * PhoneNumber) {
    char buffer[128];
    int idx = 0;
    char incoming;
    int i;

    for (i = 0; i < 128; i++) buffer[i] = 0;

    while (1) {
        if (uart2_available()) {
            incoming = UART2_GetChar();

            if (incoming == '\n' || incoming == '\r') {
                buffer[idx] = '\0';

                if (strstr(buffer, "+CLIP:") != NULL) {
                    char *start = strchr(buffer, '\"');
                    if (start) {
                        start++;
                        char *end = strchr(start, '\"');
                        if (end) {
                            *end = '\0';
                            if (strcmp(start, PhoneNumber) == 0) {
                                GPIOD->ODR |= LED_PIN; //led on
                                return 1; // Appel detecte
                            }
                            else {
                                GPIOD->ODR &= ~LED_PIN; //led off
                                return 0; // Appel d'un autre numero
                            }
                        }
                    }
                }

                idx = 0;
                memset(buffer, 0, sizeof(buffer));
            } else {
                if (idx < (int)(sizeof(buffer) - 1)) {
                    buffer[idx++] = incoming;
                }
            }
        }
    }
}

int GPS_readInfo(char* buffer, int buffer_size) {
    char c;
    int idx = 0;
    int found = 0;
    int timeout = 3000000;  // Timeout

    USART2_SendString("AT+CGPSINF=0\r\n");

    while (timeout--) {
        if (uart2_available()) {
            c = UART2_GetChar();

            // Construction ligne par ligne
            if (idx < buffer_size - 1) {
                buffer[idx++] = c;
            }

            // Fin de ligne detectee
            if (c == '\n') {
                buffer[idx] = '\0';

                // Verifie si la ligne contient la trame GPS
                if (strstr(buffer, "+CGPSINF:") != NULL) {
                    return 1; // Trame trouvee
                } else {
                    // Reset pour lire la ligne suivante
                    idx = 0;
                }
            }
        }
    }

    return 0; // Pas de trame recue dans le delai
}

// Extrait Latitude, Longitude, Altitude, Date et Heure depuis la trame GPS
void GPS_parseInfo(const char* trame, char* latitude, char* longitude, char* altitude, char* date) {
    char buffer[128];
    strncpy(buffer, trame, sizeof(buffer));
    buffer[sizeof(buffer)-1] = '\0'; // securite

    char* token;
    int field = 0;

    token = strtok(buffer, ",");
    while (token != NULL) {
        switch (field) {
            case 1: // Latitude
                strcpy(latitude, token);
                break;
            case 2: // Longitude
                strcpy(longitude, token);
                break;
            case 3: // Altitude
                strcpy(altitude, token);
                break;
            case 4: // Date et heure
                strcpy(date, token);
                break;
        }

        token = strtok(NULL, ",");
        field++;
    }
}

void sendLocationViaSMS(char * PhoneNumber) {
    char GPSbuffer[128];	
    char lat[64];
    char longit[64];	
    char alt[64];
    char date[64];
    
    if (GPS_readInfo(GPSbuffer, 128)) {
        GPS_parseInfo(GPSbuffer, lat, longit, alt, date);
        char link[100];
        sprintf(link, "https://maps.google.com/?q=%s,%s", lat, longit);
        GSM_SendSms(PhoneNumber, link);
    }
}

// Scenario 1 : make call to my phone number

/*
int main(void){
    GPIO_init();
    USART2_Init();
    configSIM808();
    while(1){
        GSM_MakeCall(MY_PHONE_NUMBER);
        GPIOD->ODR |= LED_PIN; //led on
        delay(30000);
    }
    return 0;
}
*/

// Scenario 2 : led on if received phone number = my phone number
/*
int main(void){
    GPIO_init();
    USART2_Init();
    configSIM808();
    while(1){
        checkIncomingCall(MY_PHONE_NUMBER);
    }
    return 0;
}
*/

// Scenario 3 : led on + send location on call receive from my phone number
int main(void){
    GPIO_init();
    USART2_Init();
    configSIM808();
    while(1){
        if (checkIncomingCall(MY_PHONE_NUMBER)) {
            sendLocationViaSMS(MY_PHONE_NUMBER);
        }
    }
    return 0;
}
