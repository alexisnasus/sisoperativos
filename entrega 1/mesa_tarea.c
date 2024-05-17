#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

int main(void)
{
    while(1) //Se ejecuta dentro de un while(true), así la mesa siempre está disponible.
    { 
        srand(time(NULL));
        int r=rand() % (21 + 1 - 10) + 10; //Acá se genera el puntaje a superar. Un máximo de 21, y un mínimo de 10. El mínimo es 10 por que el seed de c
                                           //tiende a valores muy bajos(2 o 3 o cosas así).
        printf("Esta es la mesa del casino. \n");
        int cofre=999999999; //El dinero del casino.
        int fd;//Los fifos, van hasta el 4.
        int fd2;
        int fd3;
        int fd4;
        int i;//Operador de fors.
        int decisiones1;//Cantidad de cartas a generar, dato recibido del jugador con las cartas pedidas.
        int decisiones2;//Monto de apuesta.
        int resultado;//resultado será la variable que tomará el valor de la suma de las cartas por parte del cliente.
        int cartasaux[3]={7,7,7};//Este aux está hecho para poder probar rapido condiciones de victoria. se reemplaza en la linea 55 para probar rapido.
        mkfifo("/tmp/myfifo", 0666);
        mkfifo("/tmp/myfifo2",0666);
        mkfifo("/tmp/myfifo3",0666);
        mkfifo("/tmp/myfifo4",0666);
        fd = open("/tmp/myfifo",O_WRONLY);
        fd2= open("/tmp/myfifo2",O_RDONLY);
        fd3= open("/tmp/myfifo3",O_WRONLY);
        fd4= open("/tmp/myfifo4",O_RDONLY);
        printf("El valor de la mesa es: %d \n",r);
        write(fd, &r, sizeof(int));//Acá se envía el valor de la mesa a superar.
        close(fd);
        read(fd2, &decisiones1,sizeof(int)); //acá se recibe la cantidad de cartas a generar enviado por el jugador.
        int cartas[decisiones1];//se crea el arreglo cartas de tamaño[cartas a superar], que fue recibido del jugador.
        for(i=0; i<decisiones1; i++)
        {
            srand(time(NULL)); //Acá se generan las cartas. El srand está al inicio de cada ciclo para reesedear el randomness.
            int nrc=rand() % (10 + 1 - 1) + 1;//Un valor máximo de 10 para las cartas y un mínimo de 1.
            cartas[i]=nrc;
        }
        read(fd2, &decisiones2,sizeof(int));//acá se recibe el monto de apuesta del jugador.
        close(fd2);
        printf("El jugador desea sacar: %d",decisiones1);
        printf(" cartas \n");
        printf("Y desea apostar: %d",decisiones2);
        printf(" creditos \n");
        for(i=0; i<decisiones1; i++)
        {
            write(fd3,&cartas[i],sizeof(int));//acá se envían las cartas generadas aleatoriamente.
        }
        close(fd3);
        read(fd4, &resultado, sizeof(int));//acá se lee el resultado, es decir, cuanto sumaron las cartas.
        if(resultado>r && resultado<=21)//Estos ifs simulan el pago, o cobro hecho al jugador respecto al resultado del juego.
        {
            cofre=cofre - decisiones2;
        }
        else if(resultado == 21)
        {
            cofre=cofre-decisiones2;
        }
        else
        {
            cofre=cofre + decisiones2;
        }
        close(fd4);

    }    
//Nota se hicieron 4 pipes por que surgian errores de lectura trabajando con dos nomás.
}