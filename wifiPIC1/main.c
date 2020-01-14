/*
 * File:   main.c
 * Author: Esteban Prieto
 * Created on 24 de septiembre de 2019, 03:50 PM
 * Ejemplo comunicacion tcpip
 */
#include "systemPIC32MX.h"
#include "m_uart.h"
#include "M_ADC.h"
#include "M_Timers.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

char getStr[100]="";
char nombre[20]= "prueba";
char contra[20]= "123456789";
char IP[20]="192.168.1.254";
char puerto[5]="200";
char cadena[50]="";
volatile char indx = 0;
volatile char a = 0;

typedef struct{
    unsigned procesar:1;//crea una bandera 
}soft_flags;
volatile soft_flags bandera;

void tcpWrite(char *dato,char ID){//ingresa el dato y el id del dispositivo
    int lonStr = strlen(dato);//Obtiene la longitud de la cadena
    strcpy(getStr,dato);//compia la variable local a una global
    char strAux[20]="";//crea una variable local
    
    Uart2Puts("AT+CIPSEND=");//envia la informaacion de la cadena que se va a enviar al modulo
    Uart2WriteData(ID);
    sprintf(strAux,",%d\r\n\0",lonStr);
    Uart2Puts(strAux);
}


void __ISR(_UART2_VECTOR,IPL7SOFT) Uart2_Rx(void){
    
    unsigned int tmp;
    
    if(IFS1bits.U2RXIF==1){
        tmp=Uart2ReadData();
        if(tmp == '>'){
            Uart2Puts(getStr);//si tmp es igual al caracter > envia la cadena
        }
        //cuando hay un enter significa que es el fin de la cadena entonces resetea el puntero
        //y activa la bandera para procesar la cadena
        if(tmp=='\n' || tmp=='\r'){
            if (indx!=0){
                cadena[indx]='\0';
                indx = 0;
                bandera.procesar=1;
            }
        }
        else{
            cadena[indx]=tmp;//Hace la cadena de caracteres
            indx++;
        }
        IFS1bits.U2RXIF=0;
    }
}


void configServerMode(char *password,char *SSID,char *IP,char *puerto){
    delay_ms(1); 
    Uart2Puts("AT\r\n\0");//Prueba de funcionamiento del modulo ESP
    delay_ms(1);  
    Uart2Puts("AT+RST\r\n\0");//Resetea el modulo
    delay_ms(260);
    Uart2Puts("ATE0\r\n\0");//Quita el eco del ESP, se cambia el 0 por el 1 para depurar
    delay_ms(6);
    Uart2Puts("AT+CWMODE=2\r\n\0");//Configura el modulo en modo soft access point
    delay_ms(6);
    Uart2Puts("AT+CIPSERVER=0\r\n\0");// Elimina el servidor y desconecta todos los dispositivos
    delay_ms(7);
    Uart2Puts("AT+CIPMUX=0\r\n\0");//Deshabilita que multiples dispositivos se conecten
    delay_ms(1);
    Uart2Puts("AT+CIPMODE=0\r\n\0");//modo de trasmisión normal
    delay_ms(1);
    
    Uart2Puts("AT+CWSAP=\"");//Se introduce el nombre de la red y la contraseñá, puede ser cualquiera.
    Uart2Puts(SSID);
    Uart2Puts("\",\"");
    Uart2Puts(password);
    Uart2Puts("\",5,3\r\n\0");
    delay_ms(1);
    
    Uart2Puts("AT+CIPAP=\"");//Se introduce la direccion ip predeterminada, puede ser cualquier direccion
    Uart2Puts(IP);
    Uart2Puts("\"\r\n\0");
    delay_ms(1);
    
    Uart2Puts("AT+CIPMUX=1\r\n\0");//habilita el modo de multiples conexiones
    delay_ms(1);
    
    Uart2Puts("AT+CIPSERVER=1,");//Crea el servidor en el puerto especificado
    Uart2Puts(puerto);
    Uart2Puts("\r\n\0");
    delay_ms(1);
    
    Uart2Puts("AT+CIPSTO=0\r\n\0");//Quita el tiempo de espera para obtener respuesta del servidor
    delay_ms(1);
}

int main(void) {
    ANSELA=0X00000000;//inicia los puertos analogicos en cero
    ANSELB=0X00000000;
    
    TRISBbits.TRISB10 = 0;//configura los pines para la trasmicion y recepbion serial
    TRISBbits.TRISB11 = 1;
    
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 0;
    
    Uart2_Config(Uart2_Rx_RPB11,Uart2_Tx_RPB10,115200);
    Uart2TxEn(1);
    Uart2RxEn(1);//configura y habilata a uart a 115200 baudios
    Uart2En(1);
    
    
    __builtin_disable_interrupts();//se configuran las interrupciones
    
    INTCONbits.MVEC=1;
    IPC9bits.U2IP=7;
    IPC9bits.U2IS=3;    
    IFS1bits.U2RXIF=0;
    IEC1bits.U2RXIE=1;
    
    __builtin_enable_interrupts(); 
    configServerMode(contra,nombre,IP,puerto);
    
    bandera.procesar=0;//pone la bandera procesar en 1
    
    while(1){
        /*Si se activa la bandera entonces la desactiva para que no se ejecute siempre
         *esa parte del codigo, en la cadena ya obtenida busca la palabra on, si esta
         *entoces enciende el led y le envia al dispositivo la informacion de que el led esta
         * encendido, si dice la palabra off apaga el led y le dice al dispositivo que el led
         * esta apagado, esa informacion se lo envia al dispositivo 0  
         */
        if (bandera.procesar==1){
            bandera.procesar=0;
            if(strstr(cadena,"on")){
                LATAbits.LATA0 = 1;
                tcpWrite("led enecendido\r\n\0",'0');
            }
            else if(strstr(cadena,"off")){
                LATAbits.LATA0 = 0;
                tcpWrite("led apagado\r\n\0",'0');
            }
            
        }
    }
}
