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

void tcpWrite(char *dato,char ID){
    int lonStr = strlen(dato);
    strcpy(getStr,dato);
    char strAux[20]="";
    Uart2Puts("AT+CIPSEND=");
    Uart2WriteData(ID);
    sprintf(strAux,",%d\r\n\0",lonStr);
    Uart2Puts(strAux);
}


void __ISR(_UART2_VECTOR,IPL7SOFT) Uart2_Rx(void){
    
    unsigned int tmp;
    
    if(IFS1bits.U2RXIF==1){
        tmp=Uart2ReadData();
        if(tmp == '>'){
            Uart2Puts(getStr);
        }
        if(tmp=='\n' || tmp=='\r'){
            if (indx!=0){
                cadena[indx]='\0';
                indx = 0;
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
        else{
            cadena[indx]=tmp;
            indx++;
        }
        IFS1bits.U2RXIF=0;
    }
}


void configServerMode(char *password,char *SSID,char *IP,char *puerto){
    delay_ms(1); 
    Uart2Puts("AT\r\n\0");
    delay_ms(1);  
    Uart2Puts("AT+RST\r\n\0");
    delay_ms(260);
    Uart2Puts("ATE0\r\n\0");
    delay_ms(6);
    Uart2Puts("AT+CWMODE=2\r\n\0");
    delay_ms(6);
    Uart2Puts("AT+CIPSERVER=0\r\n\0");
    delay_ms(7);
    Uart2Puts("AT+CIPMUX=0\r\n\0");
    delay_ms(1);
    Uart2Puts("AT+CIPMODE=0\r\n\0");
    delay_ms(1);
    Uart2Puts("AT+CWSAP=\"");
    Uart2Puts(SSID);
    Uart2Puts("\",\"");
    Uart2Puts(password);
    Uart2Puts("\",5,3\r\n\0");
    delay_ms(1);
    Uart2Puts("AT+CIPAP=\"");
    Uart2Puts(IP);
    Uart2Puts("\"\r\n\0");
    delay_ms(1);
    Uart2Puts("AT+CIPMUX=1\r\n\0");
    delay_ms(1);
    Uart2Puts("AT+CIPSERVER=1,");
    Uart2Puts(puerto);
    Uart2Puts("\r\n\0");
    delay_ms(1);
    Uart2Puts("AT+CIPSTO=0\r\n\0");
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
    
    while(1){
    }
}
