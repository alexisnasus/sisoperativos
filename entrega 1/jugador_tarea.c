#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

//Este corresponde al código del jugador. Para ejecutar, primero compilar en un programa(ctrl + shift + b) y abrir dos terminales una para este y la
//otra para el proceso de la mesa. Una vez abierta dos terminales, ejecutar primero mesa y luego jugador.
int main(void)
{
    //Acá se inician variables que serán utilizadas por el programa. los fds corresponden a las pipes utilizadas, en este caso fifos. Dolares serán los
    //creditos iniciales que se podran apostar. r será el valor a superar de la mesa, que será entregado por la mesa mediante fd. input1 y 2 serán los 
    //valores a enviar en lo que respecta la cantidad de cartas a pedir y el número de creditos a apostar respectivamente. 
    //lastinput se usa para decidir si se sigue jugando o no luego de perder(Y aun tener creditos) o después de ganar.
    int dolares=10;
    int fd;
    int fd2;
    int fd3;
    int fd4;
    int r;
    int input;
    int input2;
    int lastinput;
    while(1) //Se hace dentro de un while(true), Esto para que se pueda seguir jugando en caso de desearlo.
    { 
        pid_t pid = getpid(); //El pid será usado principalmente para kills.
        printf("Buenas, Bienvenido al casino. \n");
        int suma=0;
        fd = open("/tmp/myfifo",O_RDONLY); //En estas lineas se abren todas las fifos. Se puede apreciar el ritmo que seguirán también, primero leyendo
        fd2= open("/tmp/myfifo2",O_WRONLY);//de la mesa y luego escribiendo, para luego leer y finalmente mandar un ultimo mensaje.
        fd3= open("/tmp/myfifo3",O_RDONLY);
        fd4= open("/tmp/myfifo4",O_WRONLY);
        read(fd,&r,sizeof(int)); //Acá se lee el primer fifo y se recibe la cantidad de puntos a superar de la mesa.
        close(fd);
        printf("El valor a superar es: %d \n", r);
        printf("En base al puntaje a vencer, cuantas cartas desea sacar? \n");
        scanf("%d", &input); 
        printf("acutalmente tienes: %d",dolares);
        printf(" creditos. \n");
        printf(" Cuanto desea apostar: \n");
        scanf("%d",&input2); //tratar de no colocar numeros negativos, o mayores a 10 para observar el funcionamiento pedido por la tarea.
                             //no hice un algoritmo de if para el input de apuesta o de pedido de cartas, ya que según entendí eso no se evalúa.
        write(fd2, &input,sizeof(int));//Se envía a la mesa el número de cartas que se desea sacar.
        write(fd2, &input2,sizeof(int));//Se envía a la mesa el monto de la apuesta.
        close(fd2);
        int carta[input];//este arreglo de tamaño input(número cartas pedidas) guardará las cartas enviadas por la mesa central.
        for(int i=0; i<input; i++)
        {
            //En este ciclo, se recibe el número de cartas que se solicito. Las cartas se guardan en el arreglo carta. luego se suman y se guarda el 
            //valor en suma.
            read(fd3, &carta[i],sizeof(int));
            suma=suma + carta[i];
        }
        close(fd3);
        printf("Las cartas suman: ");
        printf("%d",suma);
        printf("\n");
        if(suma>r && suma<=21) //Acá se decide si ganó o perdió el jugador.
        {
            printf("Gano!!!! ahora tiene:  ");
            dolares=dolares + input2;
            printf("%d",dolares);
            printf("\n");
        }
        else if(suma == 21)
        {
            printf("gano!!!! ahora tiene: ");
            dolares=dolares + input2;
            printf("%d",dolares);
            printf("\n");
        }
        else
        {
            if((dolares-input2)==0) //Nuevamente, recordar usar valores de apuestas entre 1 y 10
            {
                printf("Perdio toodo!!! hora de morir \n");
                kill(pid,SIGKILL);
            }
            else
            {
                printf("Perdió, pero aun esta con creditos/vida \n"); //No sabía que hacer para este caso, asi que decidí que se podría seguir jugando
                                                                      //mientras se tuvieran creditos.
            }
        }
        write(fd4, &suma, sizeof(int)); //Acá se envía el resultado de la suma de las cartas a la mesa central.
        close(fd4);
        printf("continuar? 1=si, 2=no:\n");
        fflush(stdout);
        scanf("%d",&lastinput);
        if(lastinput==1) 
        {
            printf("Id actual: %d\n",getpid()); //En caso de querer continuar, se pasa a este punto. acá se logrará que el proceso "tome otro lugar".
            int reemplazo=fork();//Y para lograr el reemplazo, se usa fork.
            if(reemplazo==0)//solo se ejecuta el código del juego sobre el hijo.
            {
                printf("Nuevo id: %d\n",getpid());
                
            }
            else
            {
                wait(NULL); //Mientras tanto, el proceso padre se queda esperando hasta que acaben las ejecuciones de cuantos hijos se creen.
                            //Es muy importante este wait(NULL), sin el no es posible lograr ejecutar sobre el hijo inputs y ese tipo de cosas.
                return 0;
            }
        }
        else
        {
            printf("Eligo rendirme, y morir\n"); //Condición creada para poder salir del proceso una vez hecho un turno.
            kill(pid,SIGKILL);
            break;
        }
    }

}