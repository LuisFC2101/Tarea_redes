#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime> //LA usamos para el primer turno aleatorio

using namespace std;
const string	RESULTADO_EMPATE = "e",
		RESULTADO_GANA = "g",
		RESULTADO_PIERDE = "p";

const int COLUMNAS_DEFECTO = 7,
	  FILAS_DEFECTO = 6,
	  FICHAS_JUNTAS_NECESARIAS_PARA_GANAR = 4;

const char ESPACIO_VACIO = ' ',
	   JUGADOR_HUMANO = 'c',
	   JUGADOR_CPU = 's',
	   DELIMITADOR_RESULTADOS = ',',
	   DELIMITADOR_MOVIMIENTOS = ',';

const struct 	ConfiguracionTablero {
    		int columnas = 7;
    		int filas = 6;
		} configuracion;
struct ConteoConColumna
{
	int conteo, columna;
};

struct Movimiento
{
	char jugador;
	int columna;
};

string solicitar_nick()
{
	string nombre;
	cout << "Ingresa tu nick para comenzar el juego: ";
	cin >> nombre;
	return nombre;
}
//inicializamos un tablero de juego con las dimensiones especificadas en la estructura 
vector<vector<char> > inicializarTablero(ConfiguracionTablero configuracion)
{
	vector<vector<char> > tablero;
	int x, y;
	for (y = 0; y < configuracion.filas; y++)
	{
		vector<char> fila;
		tablero.push_back(fila);
		for (x = 0; x < configuracion.columnas; x++)
		{
			tablero.at(y).push_back(ESPACIO_VACIO);
		}
	}
	return tablero;
}
//Imprime el estado actual del tablero
void imprimir_tablero(vector<vector<char> > tablero)
{
	cout << endl;
	int y, x;
	const int filas = tablero.size(),
			  columnas = tablero.at(0).size();
	// Tablero
	for (y = 0; y < filas; y++)
	{
		for (x = 0; x < columnas; x++)
		{
			char actual = tablero[y][x];
			cout << "|" << actual;
			if (x + 1 >= columnas)
			{
				cout << "|";
			}
		}
		cout << endl;
	}

	// Imprimir línea de +--+--+--+
	for (x = 0; x < columnas; ++x)
	{
		cout << "+-";
		if (x + 1 >= columnas)
		{
			cout << "+";
		}
	}
	cout << endl;
	// Imprime los numeros
	for (x = 0; x < columnas; ++x)
	{
		cout << "|" << x;
		if (x + 1 >= columnas)
		{
			cout << "|";
		}
	}
	cout << endl;
}
//Esta funcion busca la primera fila vacia en una columna especifica del tablero
int obtener_primera_fila_vacia(int columna, vector<vector<char> > tablero)
{
	int i;
	for (i = tablero.size() - 1; i >= 0; i--)
	{
		if (tablero[i][columna] == ESPACIO_VACIO)
		{
			return i;
		}
	}
	return -1;
}
//Esta funcion solicita al usuario que seleccione una columna para realizar su movimiento
int solicitar_columna(vector<vector<char> > tablero)
{
	while (true)
	{
		cout << "Seleccione columna para su tirada: ";
		int columna;
		cin >> columna;
		if (columna >= 0 && columna <= tablero[0].size() - 1 && obtener_primera_fila_vacia(columna, tablero) != -1)
		{
			return columna;
		}
		else if (obtener_primera_fila_vacia(columna, tablero) == -1)
		{
			cout << "No hay filas vacias en esta columna" << endl;
		}
		else
		{
			cout << "Columna inválida" << endl;
		}
	}
}

//Esta funcion coloca una ficha del jugador actual en la columna seleccionada del tablero
vector<vector<char> > colocar_pieza(int columna, vector<vector<char> > tablero, char jugador)
{
	tablero[obtener_primera_fila_vacia(columna, tablero)][columna] = jugador;
	return tablero;
}

//LAs siguientes funciones se utilizan para determinar si un jugador ha ganado.----------

int contarArribaDerecha(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int xFin = (x + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR < tablero[0].size()) ? x + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR - 1 : tablero[0].size() - 1;
	int yInicio = (y - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR >= 0) ? y - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR + 1 : 0;
	int contador = 0;
	while (x <= xFin && yInicio <= y)
	{
		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
		x++;
		y--;
	}
	return contador;
}

int contarArribaIzquierda(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int xFin = (x - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR >= 0) ? x - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR + 1 : 0;
	int yInicio = (y - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR >= 0) ? y - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR + 1 : 0;
	int contador = 0;
	while (x >= xFin && yInicio <= y)
	{
		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
		x--;
		y--;
	}
	return contador;
}

int contarArriba(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int yInicio = (y - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR >= 0) ? y - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR + 1 : 0;
	int contador = 0;
	for (; yInicio <= y; yInicio++)
	{
		if (tablero[yInicio][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
	}
	return contador;
}

int contarAbajo(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int yFin = (y + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR <= tablero.size() - 1) ? y + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR - 1 : tablero.size() - 1;
	int contador = 0;
	for (; y <= yFin; y++)
	{
		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
	}
	return contador;
}

int contarDerecha(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int xFin = (x + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR < tablero[0].size()) ? x + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR - 1 : tablero[0].size() - 1;
	int contador = 0;
	for (; x <= xFin; x++)
	{
		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
	}
	return contador;
}

int contarAbajoDerecha(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int xFin = (x + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR < tablero[0].size()) ? x + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR - 1 : tablero[0].size() - 1;
	int yFin = (y + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR < tablero.size()) ? y + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR - 1 : tablero.size() - 1;
	int contador = 0;
	while (x <= xFin && y <= yFin)
	{
		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
		x++;
		y++;
	}
	return contador;
}
int contarIzquierda(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int xFin = (x - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR >= 0) ? x - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR + 1 : 0;
	int contador = 0;
	for (; x >= xFin; x--)
	{
		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
	}
	return contador;
}
int contarAbajoIzquierda(int x, int y, char jugador, vector<vector<char> > tablero)
{
	int xFin = (x - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR >= 0) ? x - FICHAS_JUNTAS_NECESARIAS_PARA_GANAR + 1 : 0;
	int yFin = (y + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR < tablero.size()) ? y + FICHAS_JUNTAS_NECESARIAS_PARA_GANAR - 1 : tablero.size() - 1;
	int contador = 0;
	while (x >= xFin && y <= yFin)
	{

		if (tablero[y][x] == jugador)
		{
			contador++;
		}
		else
		{
			return contador;
		}
		x--;
		y++;
	}
	return contador;
}

//Aqui se verifica si hay algun ganador
bool jugador_gana(char jugador, vector<vector<char> > tablero)
{
	/*
 * Solo necesitamos
 * Arriba
 * Derecha
 * Arriba derecha
 * Abajo derecha
 * Las demás direcciones son útiles para cuando se elige a la columna ganadora
 *
  */
	int y;
	for (y = 0; y < tablero.size(); y++)
	{
		int x;
		for (x = 0; x < tablero[y].size(); x++)
		{
			if (contarAbajo(x, y, jugador, tablero) >= FICHAS_JUNTAS_NECESARIAS_PARA_GANAR)
			{
				return true;
			}

			if (contarDerecha(x, y, jugador, tablero) >= FICHAS_JUNTAS_NECESARIAS_PARA_GANAR)
			{
				return true;
			}

			if (contarAbajoIzquierda(x, y, jugador, tablero) >= FICHAS_JUNTAS_NECESARIAS_PARA_GANAR)
			{
				return true;
			}
			if (contarAbajoDerecha(x, y, jugador, tablero) >= FICHAS_JUNTAS_NECESARIAS_PARA_GANAR)
			{
				return true;
			}
		}
	}
	return false;
}

//Las siguientes funciones se utilizan para determinar la mejor columna para el proximo movimiento de la CPU
int obtener_columna_ganadora(char jugador, vector<vector<char> > tablero)
{
	vector<vector<char> > tablero_original;
	tablero_original = tablero;
	int i;
	for (i = 0; i < tablero[0].size(); i++)
	{
		tablero = tablero_original;
		if (obtener_primera_fila_vacia(i, tablero) != -1)
		{
			tablero = colocar_pieza(i, tablero, jugador);
			if (jugador_gana(jugador, tablero))
			{
				return i;
			}
		}
	}
	return -1;
}

int obtener_primera_fila_llena(int columna, vector<vector<char> > tablero)
{
	int i;
	for (i = 0; i < tablero.size(); ++i)
	{
		if (tablero[i][columna] != ESPACIO_VACIO)
		{
			return i;
		}
	}
	return -1;
}

ConteoConColumna obtener_columna_en_la_que_se_obtiene_mayor_puntaje(char jugador, vector<vector<char> > tableroOriginal)
{
	ConteoConColumna conteoConCoordenada;
	conteoConCoordenada.conteo = 0;
	conteoConCoordenada.columna = -1;
	vector<vector<char> > tablero = tableroOriginal;
	int x, conteo;
	for (x = 0; x < tablero[0].size(); ++x)
	{
		tablero = tableroOriginal;

		if (obtener_primera_fila_vacia(x, tablero) != -1)
		{
			tablero = colocar_pieza(x, tablero, jugador);
			int fila_pieza_recien_colocada = obtener_primera_fila_llena(x, tablero);
			if (fila_pieza_recien_colocada != -1)
			{
				conteo = contarArriba(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarAbajo(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarDerecha(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarIzquierda(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarAbajoIzquierda(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarArribaDerecha(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarAbajoDerecha(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
				conteo = contarArribaIzquierda(x, fila_pieza_recien_colocada, jugador, tablero);
				if (conteo > conteoConCoordenada.conteo)
				{
					conteoConCoordenada.conteo = conteo;
					conteoConCoordenada.columna = x;
				}
			}
		}
	}
	return conteoConCoordenada;
}

int aleatorio_en_rango(int minimo, int maximo)
{
	// Asegurarnos de invocar a srand una vez
	static bool primeraVez = true;
	if (primeraVez)
	{
		srand(getpid());
		primeraVez = false;
	}
	return minimo + rand() / (RAND_MAX / (maximo - minimo + 1) + 1);
}

int obtener_columna_aleatoria(char jugador, vector<vector<char> > tableroOriginal)
{
	vector<vector<char> > tablero;
	while (1)
	{
		tablero = tableroOriginal;
		int columna = aleatorio_en_rango(0, tablero[0].size() - 1);
		if (obtener_primera_fila_vacia(columna, tablero) != -1)
		{
			return columna;
		}
	}
}

int obtener_columna_central(char jugador, vector<vector<char> > tableroOriginal)
{
	int mitad = (tableroOriginal[0].size() - 1) / 2;
	if (obtener_primera_fila_vacia(mitad, tableroOriginal) != -1)
	{
		return mitad;
	}
	return -1;
}

char obtener_oponente(char jugador)
{
	if (jugador == JUGADOR_HUMANO)
	{
		return JUGADOR_CPU;
	}
	return JUGADOR_HUMANO;
}

int elegir_mejor_columna(char jugador, vector<vector<char> > tablero)
{
	// Voy a comprobar si puedo ganar...
	int posibleColumnaGanadora = obtener_columna_ganadora(jugador, tablero);
	if (posibleColumnaGanadora != -1)
	{
		cout << "*Creo que estoy cerca de ganar*\n";
		return posibleColumnaGanadora;
	}
	// Si no, voy a comprobar si mi oponente gana con el siguiente movimiento, para evitarlo
	char oponente = obtener_oponente(jugador);
	int posibleColumnaGanadoraDeOponente = obtener_columna_ganadora(oponente, tablero);
	if (posibleColumnaGanadoraDeOponente != -1)
	{
		cout << "*Creo que no puedes ganar, aun...*\n";
		return posibleColumnaGanadoraDeOponente;
	}
	// En caso de que nadie pueda ganar en el siguiente movimiento, buscaré en dónde se obtiene el mayor
	// puntaje al colocar la pieza
	ConteoConColumna conteoConColumnaJugador = obtener_columna_en_la_que_se_obtiene_mayor_puntaje(jugador, tablero);
	ConteoConColumna conteoConColumnaOponente = obtener_columna_en_la_que_se_obtiene_mayor_puntaje(oponente, tablero);
	if (conteoConColumnaOponente.conteo > conteoConColumnaJugador.conteo)
	{
		cout << "*Interesante...*\n";
		return conteoConColumnaOponente.columna;
	}
	else if (conteoConColumnaJugador.conteo > 1)
	{
		cout << "*Creo que estoy tomando la delantera...*\n";
		return conteoConColumnaJugador.columna;
	}
	// Si no, regresar la central por si está desocupada

	int columnaCentral = obtener_columna_central(jugador, tablero);
	if (columnaCentral != -1)
	{
		cout << "*Creo que esta es la mejor posicion...*\n";
		return columnaCentral;
	}
	// Finalmente, devolver la primera disponible de manera aleatoria
	int columna = obtener_columna_aleatoria(jugador, tablero);
	if (columna != -1)
	{
		cout << "*Hagamoslo mas divertido...*\n";
		return columna;
	}
	cout << "Esto no debería suceder\n";
	return 0;
}

bool esEmpate(vector<vector<char> > tablero)
{
	int i;
	for (i = 0; i < tablero[0].size(); ++i)
	{
		if (obtener_primera_fila_vacia(i, tablero) != -1)
		{
			return false;
		}
	}
	return true;
}



void anunciar_victoria(char jugador, string nick)
{
	if (jugador == JUGADOR_HUMANO)
	{
		cout << nick << " gana la partida. Felicidades"
			 << "\n";
		cout << "Fin del juego."
			 << "\n"; 
	}
	else
	{
		cout << "El CPU gana la partida. Felicidades"
			 << "\n";
			 cout << "Fin del juego."
			 << "\n"; 
	}
}

void anunciar_empate()
{
	cout << "El juego termina en empate "
		 << "\n";
}


//Esta funcion es la que maneja todo respecto al juego
void jugar(string nick)
{
	srand(time(nullptr));
	
	vector<vector<char> > tablero = inicializarTablero(configuracion);
    	// Elección aleatoria del primer jugador
    	int jugadorActual = (rand() % 2 == 0) ? JUGADOR_HUMANO : JUGADOR_CPU;
	int columna;
	int conteo_movimientos = 0;
	vector<Movimiento> movimientos;
	Movimiento movimiento;
	while (true)
	{
		imprimir_tablero(tablero);
		if (jugadorActual == JUGADOR_HUMANO)
		{
			cout << nick << ", elige: " << endl;
			columna = solicitar_columna(tablero);
			cout << nick << " elige la columna " << columna << endl;
		}
		else
		{
			cout << "CPU, elige:" << endl;
			columna = elegir_mejor_columna(jugadorActual, tablero);
		}
		tablero = colocar_pieza(columna, tablero, jugadorActual);
		movimiento.columna = columna;
		movimiento.jugador = jugadorActual;
		movimientos.push_back(movimiento);
		if (jugador_gana(jugadorActual, tablero))
		{
			imprimir_tablero(tablero);
			anunciar_victoria(jugadorActual, nick);
			break;
		}
		else if (esEmpate(tablero))
		{
			imprimir_tablero(tablero);
			anunciar_empate();
			break;
		}
		jugadorActual = obtener_oponente(jugadorActual);
	}
}



int main(int argc, char *argv[]) {
	if (argc != 3) {
        cerr << "Formato correcto: " << argv[0] << " <dirección IP del servidor> <puerto>" << endl;
        return 1;
    }

int PORT = atoi(argv[2]);
int sock = 0, valread;
struct sockaddr_in serv_addr;
char buffer[1024] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Error al crear el socket" << endl;
        return 1;
    	}
	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons(PORT);

    	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        std::cerr << "Dirección no válida o no soportada" << std::endl;
        return 1;
    	}

    	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Conexión fallida" << std::endl;
        return 1;
    	}

string nick = solicitar_nick(); // Solicitar nick después de la conexión
send(sock, nick.c_str(), nick.length(), 0); // Enviar el nick al servidor

// Leer el estado del tablero desde el servidor
valread = read(sock, buffer, 1024);
    	if (valread <= 0) {
        std::cerr << "Error al leer desde el socket" << std::endl;
        return 1;
    	}

    	// Mostrar el estado del tablero
    	std::cout << "Mensaje del servidor:\n" << buffer << std::endl;

    	// Limpiar el buffer
    	std::memset(buffer, 0, 1024);

    	jugar(nick); // Llamar a la función jugar después de recibir el estado inicial del tablero

    	return 0;
}
