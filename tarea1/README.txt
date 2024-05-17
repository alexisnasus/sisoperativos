Funcionamiento
El programa se divide en varias secciones principales:

a) Inicialización de los Jugadores y los Barcos:

	-Se solicita al usuario que ingrese el número de jugadores entre 2 y 10.
	-Se generan los tableros de juego para cada jugador, con barcos colocados aleatoriamente.	
	-Cada jugador es representado por una estructura Player, que incluye su ID, los barcos y el tablero.
	-Comunicación entre Procesos y Creación de Procesos Hijos:

b) Se utilizan pipes para la comunicación entre el proceso padre y los procesos hijos (jugadores).
	-Se crean procesos hijos para cada jugador utilizando la función fork(). Cada proceso hijo inicializa su tablero y lo envía al proceso padre a través del pipe.
	-Al inicio del juego, se utilizan pipes adicionales para la comunicación entre los procesos hijos durante las rondas de ataques.
	
c) Inicio del Juego:

	-Una vez que todos los jugadores están listos, comienza el juego.
	-Los jugadores atacan por turnos en rondas.
	-El juego continúa hasta que quede un solo jugador con barcos restantes.
	
d) Ataques y Verificación de Ganador:

	-Cada jugador realiza un ataque contra todos los otros jugadores en su turno.
	-Los ataques exitosos se marcan en los tableros de los jugadores afectados.
-El juego se detiene cuando solo queda un jugador con barcos restantes.

e) Anuncio del Ganador:

 -Una vez que se determina el ganador, se imprime su tablero con los barcos restantes.
 
 
f) Anexo
	-Quiźas no se note pero hay mucho esfuerzo, humildad y desvelo detrás de este trabajo, un saludo.
