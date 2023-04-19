#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <errno.h>
#include <netdb.h>
#include <sys/wait.h>

#define MAX_FILHOS 1000

int total_portas = 0;
int portas_abertas = 0;
int portas_fechadas = 0;

void scan_port(int porta) {
    int meusocket;
    struct sockaddr_in host;
    struct hostent *he;
    struct servent *servico;

    he = gethostbyname("127.0.0.1");
    if (he == NULL) {
        printf("Erro: Host Desconhecido!!\n");
        exit(-1);
    }
    meusocket = socket(AF_INET, SOCK_STREAM, 0);
    if (meusocket < 0) {
        perror("Socket");
        exit(1);
    }
    host.sin_family = he->h_addrtype;
    host.sin_port = htons(porta);
    host.sin_addr = *((struct in_addr *) he->h_addr);
    bzero(&(host.sin_zero), 8);
    if (connect(meusocket, (struct sockaddr *) &host, sizeof(host)) == -1) {
        printf("A porta %d fechada\n", porta);
        portas_fechadas++;
    } else {
        servico = getservbyport(htons(porta), "tcp");
        printf("A porta %d aberta\tO Servico e [%s]\n", porta, (servico == NULL) ? "Desconhecido" : servico->s_name);
        portas_abertas++;
    }
    close(meusocket);
}

int main(int argc, char *argv[]) {
    int porta_inicial = 1, porta_final = 65535, porta, i;
    pid_t pid;

    for (porta = porta_inicial; porta <= porta_final; porta++) {
        if (total_portas >= MAX_FILHOS) {
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                    portas_abertas++;
                } else {
                    portas_fechadas++;
                }
            }
            total_portas--;
        }
        pid = fork();
        if (pid == 0) {
            scan_port(porta);
            exit(0);
        } else if (pid < 0) {
            perror("Fork");
            exit(1);
        }
        total_portas++;
    }

    int status;
    while (total_portas > 0) {
        wait(&status);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                portas_abertas++;
            } else {
                portas_fechadas++;
            }
        }
        total_portas--;
    }

    printf("\nTotal de Portas Escaneadas: %d\n", porta_final - porta_inicial + 1);
    printf("Portas Abertas: %d\n", portas_abertas);
    printf("Portas Fechadas: %d\n", portas_fechadas);
    return 0;
}