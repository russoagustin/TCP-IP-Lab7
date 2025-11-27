#define _POSIX_C_SOURCE 200112L

#include "tcpip.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>



int scan_host(const char *ip, int family);
int validarIP(const char *ip);
int ingresar_red(char *network, size_t net_size){

    int ip_version;

    printf("===== ESCÁNER DE SERVIDORES WEB (Puerto 80) =====\n");

    /* Entrada y normalización de subred */
    do {
        printf("Ingrese la dirección de subred (sin /):\n");
        printf("  - IPv4 /24: 3 octetos (ej: 192.168.1)\n");
        printf("  - IPv6 /64: 4 hextetos (ej: 2001:db8:1:2)\n");
        printf("> ");

        if (scanf("%45s", network) != 1) {
            fprintf(stderr, "Error leyendo entrada.\n");
            return 0;
        }

        if (strchr(network, '.')) {
            /* IPv4: exactamente 3 octetos A.B.C */
            unsigned int b1, b2, b3;
            char extra;
            int dot_count = 0;

            for (char *p = network; *p != '\0'; ++p) {
                if (*p == '.') dot_count++;
            }

            if (dot_count != 2) {
                printf("\n*** Formato IPv4 inválido: debe tener exactamente 3 octetos (ej: 192.168.1) ***\n\n");
                ip_version = 0;
                continue;
            }

            if (sscanf(network, "%u.%u.%u%c", &b1, &b2, &b3, &extra) != 3) {
                printf("\n*** Formato IPv4 inválido: use A.B.C (0–255) ***\n\n");
                ip_version = 0;
                continue;
            }

            if (b1 > 255 || b2 > 255 || b3 > 255) {
                printf("\n*** Valores de octeto fuera de rango (0–255) ***\n\n");
                ip_version = 0;
                continue;
            }

            /* Normalizar a A.B.C.0 */
            snprintf(network, net_size, "%u.%u.%u.0", b1, b2, b3);
        } else {
            /* IPv6: exactamente 4 hextetos h1:h2:h3:h4 */
            unsigned int h1, h2, h3, h4;
            char extra;
            int colon_count = 0;

            for (char *p = network; *p != '\0'; ++p) {
                if (*p == ':') colon_count++;
            }

            if (colon_count != 3) {
                printf("\n*** Formato IPv6 inválido: debe tener exactamente 4 hextetos (ej: 2001:db8:1:2) ***\n\n");
                ip_version = 0;
                continue;
            }

            if (sscanf(network, "%x:%x:%x:%x%c", &h1, &h2, &h3, &h4, &extra) != 4) {
                printf("\n*** Formato IPv6 inválido: use h1:h2:h3:h4 (0–ffff) ***\n\n");
                ip_version = 0;
                continue;
            }

            if (h1 > 0xFFFF || h2 > 0xFFFF || h3 > 0xFFFF || h4 > 0xFFFF) {
                printf("\n*** Valores de hexteto fuera de rango (0000–ffff) ***\n\n");
                ip_version = 0;
                continue;
            }

            /* Normalizar a prefijo /64: h1:h2:h3:h4:: */
            snprintf(network, net_size, "%x:%x:%x:%x::", h1, h2, h3, h4);
        }

        ip_version = validarIP(network);
        if (ip_version == 0) {
            printf("\n*** IP inválida luego de normalizar. Revise el formato. ***\n\n");
        }

    } while (ip_version == 0);

    return ip_version;
}


void escanear_red() {
   char network[MAX_IP_STR_LEN];
    int ip_version = ingresar_red(network, sizeof(network));

    printf("===== ESCÁNER DE SERVIDORES WEB (Puerto 80) =====\n");

    /* ------------------------ ESCANEO IPv4 ------------------------ */
    if (ip_version == 4) {
        struct in_addr net_addr;
        char ip_str[MAX_IP_STR_LEN];

        printf("\nEscaneando subred IPv4 /24 basada en %s (hosts .1 a .254)\n\n", network);

        if (inet_pton(AF_INET, network, &net_addr) != 1) {
            fprintf(stderr, "[ERROR] Falló inet_pton IPv4.\n");
            return;
        }

        /* Manipulación binaria correcta: usar ntohl/htonl */
        uint32_t base = ntohl(net_addr.s_addr);
        base &= 0xFFFFFF00u;  /* dejamos el último octeto en 0 */

        for (int i = 1; i <= 254; i++) {
            struct in_addr host_addr;
            host_addr.s_addr = htonl(base | (uint32_t)i);

            if (inet_ntop(AF_INET, &host_addr, ip_str, sizeof(ip_str)) == NULL) {
                perror("[ERROR] inet_ntop IPv4");
                continue;
            }

            if (scan_host(ip_str, AF_INET))
                printf("[+] %s --> SERVIDOR WEB ACTIVO\n", ip_str);
            else
                printf("[-] %s --> No responde\n", ip_str);
        }
    }
    /* ------------------------ ESCANEO IPv6 ------------------------ */
    else if (ip_version == 6) {
        struct in6_addr net_addr6;
        char ip_str[MAX_IP_STR_LEN];

        printf("\nEscaneando subred IPv6 /64 basada en %s (hosts 1–50)\n\n", network);

        if (inet_pton(AF_INET6, network, &net_addr6) != 1) {
            fprintf(stderr, "[ERROR] Falló inet_pton IPv6.\n");
            return;
        }

        for (int i = 1; i <= 50; i++) {
            struct in6_addr host_addr = net_addr6; /* copiar prefijo /64 */

            /* Último byte de los 128 bits = parte de host (simple, 50 hosts) */
            host_addr.s6_addr[15] = (unsigned char)i;

            if (inet_ntop(AF_INET6, &host_addr, ip_str, sizeof(ip_str)) == NULL) {
                perror("[ERROR] inet_ntop IPv6");
                continue;
            }

            if (scan_host(ip_str, AF_INET6))
                printf("[+] %s --> SERVIDOR WEB ACTIVO\n", ip_str);
            else
                printf("[-] %s --> No responde\n", ip_str);
        }
    }

    printf("\n===== ESCANEO FINALIZADO =====\n");
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

int scan_host(const char *ip, int family) {
    struct addrinfo hints, *res;
    int sockfd, ret;
    struct timeval tv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = family;         // AF_INET o AF_INET6
    hints.ai_flags    = AI_NUMERICHOST; // la IP viene numérica

    ret = getaddrinfo(ip, HTTP_PORT, &hints, &res);
    if (ret != 0) {
        return 0;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return 0;
    }

    tv.tv_sec  = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    ret = connect(sockfd, res->ai_addr, res->ai_addrlen);

    if (ret == 0) {

        // Consulta HTTP 
        const char *http_get = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";

        ssize_t sent = send(sockfd, http_get, strlen(http_get), 0);

        if (sent > 0) {
            char buffer[512];
            ssize_t received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
            if (received > 0) {
                buffer[received] = '\0';
                // Verifica si la respuesta contiene "HTTP/"
                if (strstr(buffer, "HTTP/") != NULL) {
                    close(sockfd);
                    freeaddrinfo(res);
                    return 1; // Es un servidor web
                }
            }
        }
    }

    close(sockfd);
    freeaddrinfo(res);

    if (ret == 0) {
        return 1; // conexión OK -> puerto 80 abierto
    } else {
        return 0; // error, timeout, refused, etc.
    }
}