#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SIZE 256
#define MAXLINE 500

void str_echo(int sock);
void solicitarNombre(char *nombre);
void solicitarNumero(char *nombre);
int validarDigitos(const char *numero);

int main(int argc, char *argv[])
{
    int sock;
    char com[SIZE];
    struct sockaddr_in adr;
    struct hostent *hp, *gethostbyname();
    int n, error;
    char recvline[MAXLINE];
    char nombre[MAXLINE];
    char numero[MAXLINE];
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Imposible creacion del socket");
        exit(2);
    }

    if ((hp = gethostbyname(argv[1])) == NULL)
    {
        perror("Error: Nombre de la maquina desconocido");
        exit(3);
    }

    adr.sin_family = PF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));
    bcopy(hp->h_addr, &adr.sin_addr, hp->h_length);

    if (connect(sock, (struct sockaddr *)&adr, sizeof(adr)) == -1)
    {
        perror("connect");
        exit(4);
    }
    // recibir bienvenida
    n = read(sock, recvline, MAXLINE - 1);
    recvline[n] = '\0';
    printf("%s", recvline);

    solicitarNombre(nombre);
    write(sock, nombre, strlen(nombre));

    do
    {
        solicitarNumero(numero);
        write(sock, numero, strlen(numero));

        n = read(sock, recvline, MAXLINE - 1);
        recvline[n] = '\0';
        error = atoi(recvline);
        
        if (error == 0)
        {
            printf("Este numero ya fue ingresado \n");
        }
        

    } while (error == 0);

    //str_echo(sock);
    printf("\n");
    n = read(sock, recvline, MAXLINE - 1);
    recvline[n] = '\0';
    printf("%s\n",recvline);

    for (;;) {
        int error=0;
        n = read(sock, recvline, MAXLINE - 1);
        recvline[n] = '\0';
        
        if (recvline[0] == '6') {
            printf("Es su turno ");
            solicitarNumero(numero);
            write(sock, numero, strlen(numero));
        }else if(recvline[0] == '2'){
            printf("En espera\n");
        }else{
            printf("%s\n",recvline);
        }
        n=0;
        bzero(revline, MAXLINE);
    }

    return 0;
}

void str_echo(int sock)
{
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;
    while (1)
    {
        // fgets(sendline, MAXLINE, stdin);
        // write(sock, sendline, strlen(sendline));
        if (read(sock, recvline, MAXLINE) == 0)
        {
            fprintf(stderr, "Servidor terminado prematuramente\n");
            exit(5);
        }
        fputs(recvline, stdout);
    }
}

void solicitarNombre(char *nombre)
{
    printf("Por favor, ingresa tu nombre: ");
    fgets(nombre, MAXLINE, stdin);
    nombre[strcspn(nombre, "\n")] = '\0';
}

int validarDigitos(const char *numero)
{
    int digitos[10] = {0};
    int longitud = strlen(numero);
    if (longitud == 4)
    {
        // Verificar que todos los caracteres sean dígitos
        for (int i = 0; i < longitud; i++)
        {
            if (numero[i] < '0' || numero[i] > '9')
            {
                return 3; // Carácter no válido encontrado
            }
        }

        // Verificar que el primer dígito sea diferente de 0
        if (numero[0] == '0')
        {
            return 4; // El primer dígito es 0
        }

        // Contar los dígitos y verificar si hay repetidos
        for (int i = 0; i < longitud; i++)
        {
            int digito = numero[i] - '0'; // Convertir a int
            digitos[digito]++;            // Incrementar el contador del dígito
            if (digitos[digito] > 1)
            {
                return 1; // Dígito repetido encontrado
            }
        }

        return 0; // No se encontraron dígitos repetidos
    }
    else
    {
        return 2; // Cantidad de dígitos inválida
    }
}

void solicitarNumero(char *numero)
{
    int repeticion = 0;
    do
    {
        printf("Ingresa el numero: ");
        fgets(numero, MAXLINE, stdin);
        numero[strcspn(numero, "\n")] = '\0';
        repeticion = validarDigitos(numero);
        if (repeticion)
        { // el return de digitos es 1 o 2
            if (repeticion == 1)
            {
                printf("Error: Los digitos no deben repetirse. Intenta nuevamente.\n");
            }
            else if (repeticion == 2)
            {
                printf("Error: Cantidad de dígitos inválida. Intenta nuevamente.\n");
            }
            else if (repeticion == 3)
            {
                printf("Error: Se ingresaron caracteres, deben ser digitos. Intenta nuevamente.\n");
            }
            else
            {
                printf("Error: Primer digito debe ser diferente de 0. Intenta nuevamente.\n");
            }
        }
    } while (repeticion); // sale cuando el return es 0
}