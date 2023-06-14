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
#define MAX_CODIGO 5

pthread_mutex_t semaforo, semaforo2;

int hilos = 0;
int validar = 1;
int rturno=0;
int pos=-1;
struct Jugador
{
    char nombre[MAX_NOMBRE];
    char codigo[MAX_CODIGO];
    int perdio;
    int posiperdio;
    int picas;
    int fijas;
    int turno;
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

int validarNumJugadores() {

const char *mensaje_Inicio = "Jugadores completos, el juego ha iniciado\n";
    int contador = 0;
    int numero=0;
    int aux;
    struct Jugador j;
    for (int i = 0; i < MAX_JUGADORES; i++)
    {
        if (strlen(jugadores[i].codigo)==4)
        {
            contador++;
        }
    }

    if (contador == 4)
    {
        for (int i=0;i<MAX_JUGADORES;i++)
        {
            numero = rand() % 4;
            aux=sock_servicio[i];
            sock_servicio[i]=sock_servicio[numero];
            sock_servicio[numero]=aux;
            j=jugadores[i];
            jugadores[i]=jugadores[numero];
            jugadores[numero]=j;

        }
        for(int o=0; o<MAX_JUGADORES; o++){

            write(sock_servicio[o], mensaje_Inicio, strlen(mensaje_Inicio));
        }
        return 1;
    }

}


void asignarTurno(int posicion)
{
    srand(time(0)); // Semilla para generar números aleatorios

    for (int i = 0; i < MAX_JUGADORES; i++) {
        int numero;
        int repetido;

        do {
            repetido = 0;
            numero = rand() % 4 + 1; // Genera un número aleatorio entre 1 y 4

            for (int j = 0; j < i; j++) {
                if (jugadores[j].turno == numero) {
                    repetido = 1;
                    break;
                }
            }
        } while (repetido); // Repite el proceso si el número está repetido
        jugadores[posicion].turno=numero;
    }
}

int validarTurno() {
    //static int rturno = 0;
    int i;
    do{
    pos++;
    pos=pos%4;
    }
    while (jugadores[pos].perdio==1);
    return pos;
/*    if (rturno == 4) {
        rturno = 1;
    } else {
        rturno++;
    }

    while (1) {
        if (jugadores[i].turno == rturno) {
            return i;
            break;
        } else if (jugadores[i].turno == 0) {
            if (rturno == 4) {
                rturno = 1;
            } else {
                rturno++;
            }
        }
        if (i < 4) {
            i++;
        } else {
            i = 0;
        }
    }*/
}

int calcularPicas(const char *codigoJugador, const char *codigoOponente)
{
    int picas = 0;
    int longitud = strlen(codigoJugador);

    for (int i = 0; i < longitud; i++)
    {
        for (int j = 0; j < longitud; j++)
        {
            if (codigoJugador[i] == codigoOponente[j] && i != j)
            {
                picas++;
            }
        }
    }

    return picas;
}

int calcularFijas(const char *codigoJugador, const char *codigoOponente)
{
    int fijas = 0;
    int longitud = strlen(codigoJugador);

    for (int i = 0; i < longitud; i++)
    {
        if (codigoJugador[i] == codigoOponente[i])
        {
            fijas++;
        }
    }

    return fijas;
}

void enviar(int socket, const char *mensaje){
    write(sock_servicio[socket], mensaje, strlen(mensaje));
}
char mensajeeli[8094];
char mensajeran[8094];
char mensajecode[8094];
char mensajepf[8094];
int contadorfinal=0;
int ranking=4;
int bandera=1;
int picasyfijas(){
    int validado=0; 
    validado=validarNumJugadores();
    int mturno=0,m;
    int turno_desbloqueado=1;
    if (validado == 1) {
        while (1) {
            if (turno_desbloqueado) {
                turno_desbloqueado=0;
                for(int i=0;i<4;i++){
                    if(jugadores[i].perdio==1){
                        jugadores[i].turno=0;
                    }
                }
                mturno = validarTurno();

                for (m=0;m<4;m++){                
                    if(m!=mturno){
                        const char *mensaje_turno = "2";
                        enviar(m, mensaje_turno);
                    }else{
                        const char *mensaje_turno = "6";
                        enviar(mturno, mensaje_turno);
                    } 
                }                
                //write(sock_servicio[mturno], mensaje_turno, strlen(mensaje_turno));
                // Leer el código del cliente
                char line[MAXLINE];
                int m = read(sock_servicio[mturno], line, MAXLINE - 1);
                line[m] = '\0';
                printf("El jugador %s ingresó el código: %s\n", jugadores[mturno].nombre, line);

                                
                sprintf(mensajecode, "el jugador %s puso un codigo", jugadores[mturno].nombre);
                for (m=0;m<4;m++){
                    if(m!=mturno){
                        write(sock_servicio[m], mensajecode, strlen(mensajecode));
                    }
                }

                for (m=0;m<4;m++){
                    if(m!=mturno){
                        if(jugadores[m].turno!=0){
                            jugadores[m].picas=calcularPicas(line, jugadores[m].codigo);
                            jugadores[m].fijas=calcularFijas(line, jugadores[m].codigo);
                            printf("%s ha tenido %d picas y %d fijas con el jugador %s\n",jugadores[mturno].nombre,jugadores[m].picas,jugadores[m].fijas,jugadores[m].nombre);
                            sprintf(mensajepf, "\n%s ha tenido %d picas y %d fijas con el jugador %s\n",jugadores[mturno].nombre,jugadores[m].picas,jugadores[m].fijas,jugadores[m].nombre);
                            write(sock_servicio[m], mensajepf, strlen(mensajepf));
                            write(sock_servicio[mturno], mensajepf, strlen(mensajepf));
                            if(jugadores[m].fijas==4){
                                jugadores[m].perdio=1;
                                jugadores[m].posiperdio=ranking;
                                contadorfinal++;
                                ranking--;
                                printf("El jugador %s perdio por tanto perdio=%d\n",jugadores[m].nombre,jugadores[m].perdio);
                                sprintf(mensajeeli, "El jugador %s a eliminado a %s",jugadores[mturno].nombre,jugadores[m].nombre);
                                write(sock_servicio[m], mensajeeli, strlen(mensajeeli));
                                write(sock_servicio[mturno], mensajeeli, strlen(mensajeeli));
                            }                           
                        }                 
                    }                
                }
                if(contadorfinal==3){
                    int i=0;
                    for(int i=0;i<4;i++){
                        if(jugadores[i].perdio!=1){
                            jugadores[i].posiperdio=ranking;
                        }
                        const char *mensajeacabo="EL JUEGO HA FINALIZADO";
                        write(sock_servicio[i], mensajeacabo, strlen(mensajeacabo));
                        const char *mensajeranki="\n----------RANKING--------";                     
                        write(sock_servicio[i], mensajeranki, strlen(mensajeranki));
                    }
                    while(bandera<5){
                        if(jugadores[i].posiperdio==bandera){
                            sprintf(mensajeran, "* %s", jugadores[i].nombre);  
                            bandera++;    
                            for(int m=0;m<4;m++){
                                write(sock_servicio[m], mensajeran, strlen(mensajeran)); 
                            }                             
                        }
                        if(i<4){
                            i++;
                        }else{
                            i=0;
                        }   
                    }
                    while(1){ 

                    }
                }

            }
            turno_desbloqueado=1;
        }
    }

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
        asignarTurno(hilos-1);
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
        asignarTurno(hilos-1);
    }

    // imprimir arreglo
    for (int i = 0; i < 4; i++)
    {
        printf("%s", jugadores[i].nombre);
        printf("|");
        printf("%s", jugadores[i].codigo);
        printf("|");
        printf("%d", jugadores[i].turno);
        printf("\n");
    }
    pthread_mutex_unlock(&semaforo); 
    picasyfijas();
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