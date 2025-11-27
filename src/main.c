#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"tcpip.h"


int main(int argc, char *argv[]){
    
    int op=0;

    while (op != 2)
    {
        system("clear");
        printf("------Bienvenido------\n\n");
        printf("Presione una ENTER para continuar...");
        while (getchar() != '\n');
        printf("Selecciones una opción:\n");
        printf("\t1. Buscar servidores web\n");
        printf("\t2. Salir\n");
        scanf("%d",&op);
        while (getchar() != '\n');

        switch (op)
        {
            case 1:
                escanear_red();
                while (getchar() != '\n');
                break;
            case 2:
                printf("Saliendo del programa...\n");
                return 0;
                break;
            default:
        }
    }


    return 0;
}