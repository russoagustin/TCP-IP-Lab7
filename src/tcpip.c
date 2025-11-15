#include "tcpip.h"
#include "constantes.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int validarIP(const char *ip);

/**
 * Función que solicita al usuario ingresar una dirección IP (IPv4 o IPv6).
 * Valida la dirección IP ingresada.
 * Retorna 4 si es IPv4 o 6 si es IPv6
 */
int ingresarIP(char **ip) {
    size_t len = 0;
    int f;

    while (1){

        printf("Ingrese la dirección IP (IPv4 o IPv6)\n");    
        getline(ip,&len,stdin);
        (*ip)[strcspn(*ip, "\n")] = 0; //Elimina el salto de línea al final

        f = validarIP(*ip);
        if (f == IPV4 || f == IPV6){
            return f;

        }else{
            printf("Dirección IP inválida. Por favor, intente de nuevo.\n");
        }
    }
}


/**
 * Función que valida si un ip es válida.
 * Retorna 4 si es IPv4 o 6 si es IPv6
 * Retorna 0 si no es válida
 */
int validarIP(const char *ip) {
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;

    if (inet_pton(AF_INET, ip,&(sa.sin_addr))==1)
    {
        return IPV4;
    }
    
    if (inet_pton(AF_INET6, ip,&(sa6.sin6_addr))==1)
    {
        return IPV6;
    }
    
    return 0; 
}