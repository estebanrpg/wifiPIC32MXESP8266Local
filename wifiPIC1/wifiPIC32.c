#include "wifiPIC32.h"

void configClientMode(char *password,char *SSID){
    delay_ms(1000);
    Uart2Puts("AT\r\n\0");
    delay_ms(1000);  
    Uart2Puts("AT+RST\r\n\0");
    delay_ms(15000);
    Uart2Puts("AT+CWMODE=1\r\n\0");
    delay_ms(5000);
    Uart2Puts("AT+CWJAP=\"");
    Uart2Puts(SSID);
    Uart2Puts("\",\"");
    Uart2Puts(password);
    Uart2Puts("\"\r\n\0");
    delay_ms(15000);
    
}
void thingSpeakWrite(double data, char *getStr, char *apiKey){
    Uart2Puts("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n\0");
    delay_ms(300);
    char strAux[100]="";
    sprintf(strAux,"&field1=%0.2f\r\n\r\n\0",data);
    strcpy(getStr,"GET /update?api_key=");
    strcat(getStr,apiKey);
    strcat(getStr,strAux);
    int lonStr=strlen(getStr);
    sprintf(strAux,"AT+CIPSEND=%d\r\n\0",lonStr);
    Uart2Puts(strAux);
}
