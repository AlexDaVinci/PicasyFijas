#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define PORT 2222
#define MAXLINE 4096
#define TRUE 1
#define MAX_JUGADORES 4
#define MAX_NOMBRE 20
#define MAX_CODIGO 4

pthread_mutex_t semaforo, semaforo2;

int hilos = 0;
int validar = 1;

struct Jugador
{
    char nombre[MAX_NOMBRE];
    char codigo[MAX_CODIGO];
};

struct Jugador jugadores[MAX_JUGADORES];

int sock_servicio[10];

int crearsocket(int *port, int type)
{
    int sockfd;
    struct sockaddr_in adr;
    int longitud;

    if ((sockfd = socket(PF_INET, type, 0)) == -1)
    {
        perror("Error: Imposible crear socket");
        exit(2);
    }

    bzero((char *)&adr, sizeof(adr));
    adr.sin_port = htons(*port);
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_family = PF_INET;

    if (bind(sockfd, (struct sockaddr *)&adr, sizeof(adr)) == -1)
    {
        perror("Error: bind");
        exit(3);
    }

    longitud = sizeof(adr);

    if (getsockname(sockfd, (struct sockaddr *)&adr, &longitud))
    {
        perror("Error: Obtencion del nombre del sock");
        exit(4);
    }

    *port = ntohs(adr.sin_port);
    return sockfd;
}

void sigchld()
{
    pid_t pid;
    int stat;

    pid = wait(&stat);
    fprintf(stderr, "Proceso hijo: %d terminado\n", pid);
    return;
}

void validarRepetido(const char *codigo)
{
    int i;
    int codigoJugador;
    int codigoEntrante = atoi(codigo);

    for (i = 0; i < MAX_JUGADORES; i++)
    {
        codigoJugador = atoi(jugadores[i].codigo);

        if (codigoJugador == codigoEntrante)
        {
            validar = 0;
            return;
        }
    }
    validar = 1;
    return;
}

void *servicio(void *sock)
{
    pthread_mutex_lock(&semaforo);
    int client_sock = *(int *)sock;
    ssize_t n, m;
    char line[MAXLINE];

    // Bienvenida
    const char *mensaje_Bienvenida = "Bienvenido a picas y fijas\n";
    write(client_sock, mensaje_Bienvenida, strlen(mensaje_Bienvenida));

    // Leer nombre del cliente
    n = read(client_sock, line, MAXLINE - 1);
    line[n] = '\0';
    printf("Nombre del cliente: %s\n", line);

    strncpy(jugadores[hilos - 1].nombre, line, MAX_NOMBRE); // Guardar nombre del jugador

    // Leer código del cliente
    if (hilos == 1)
    {
        m = read(client_sock, line, MAXLINE - 1);
        line[m] = '\0';
        const char *mensaje_Error = "1";
        write(client_sock, mensaje_Error, strlen(mensaje_Error));
        printf("Código del cliente: %s\n", line);
        strncpy(jugadores[hilos - 1].codigo, line, MAX_CODIGO); // Guardar código del jugador
    }
    else
    {
        do
        {

            m = read(client_sock, line, MAXLINE - 1);
            line[m] = '\0';
            validarRepetido(line);

            if (validar == 0)
            {
                const char *mensaje_Error = "0";
                write(client_sock, mensaje_Error, strlen(mensaje_Error));
            }

        } while (validar == 0);
        const char *mensaje_Error = "1";
        write(client_sock, mensaje_Error, strlen(mensaje_Error));
        printf("Código del cliente: %s\n", line);
        strncpy(jugadores[hilos - 1].codigo, line, MAX_CODIGO); // Guardar código del jugador
    }

    // imprimir arreglo
    for (int i = 0; i < 4; i++)
    {
        printf("%s", jugadores[i].nombre);
        printf("|");
        printf("%s", jugadores[i].codigo);
        printf("\n");
    }
    pthread_mutex_unlock(&semaforo);
}

int main(int argc, char *argv[])
{
    int sock_escucha;
    struct sockaddr_in adr;
    int lgadr = sizeof(adr);
    int port = PORT;
    pthread_t t1[10];
    int clientes_conectados = 0;
    int max_clientes = 4;
    int finalizar = 0;

    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s [port]\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    if ((sock_escucha = crearsocket(&port, SOCK_STREAM)) == -1)
    {
        fprintf(stderr, "Error: No se pudo crear/conectar socket\n");
        exit(2);
    }

    signal(SIGCHLD, sigchld);

    listen(sock_escucha, 1024);

    fprintf(stdout, "Inicio servidor en el puerto %d\n", port);

    while (TRUE)
    {
        if (finalizar == 1)
            break;

        lgadr = sizeof(adr);
        sock_servicio[hilos] = accept(sock_escucha, (struct sockaddr *)&adr, &lgadr);

        if (clientes_conectados >= max_clientes)
        {
            const char *mensaje = "Cantidad de jugadores máxima conectada\n";
            write(sock_servicio[hilos], mensaje, strlen(mensaje));
            close(sock_servicio[hilos]);
        }
        else
        {
            fprintf(stdout, "Servicio aceptado: %d\n", hilos);
            pthread_create(&t1[hilos], NULL, servicio, &sock_servicio[hilos]);
            pthread_mutex_lock(&semaforo);
            hilos++;
            pthread_mutex_unlock(&semaforo);
            clientes_conectados++;
        }

        if (clientes_conectados >= max_clientes)
        {
            const char *mensaje = "Cantidad de jugadores máxima conectada\n";
            write(sock_servicio[hilos], mensaje, strlen(mensaje));
        }
    }

    close(sock_escucha); // Cerrar el socket de escucha después del bucle

    return 0;
}
