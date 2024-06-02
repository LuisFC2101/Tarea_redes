#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
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
#include <pthread.h> // Para la creación de hilos


//Esta funcion se ejecuta en un hilo separado para manejar la comunicación con un cliente
void *manejar_cliente(void *socket_desc) {
	int client_socket = *(int*)socket_desc;
    	char buffer[1024] = {0};
    	int valread;

std::cout << "Cliente conectado. Esperando datos..." << std::endl;

    	while ((valread = read(client_socket, buffer, 1024)) > 0) {
        // Procesa los datos recibidos del cliente
        std::cout << "Nick del oponente: " << buffer << std::endl;
        std::cout << "Hay una nueva partida en proceso. "
		 << "\n";

        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer)); // Limpia el buffer
    	}

if (valread == 0) {
	std::cout << "El cliente ha cerrado la conexión" << std::endl;
    	} else 
	{
        	std::cerr << "Error al leer desde el cliente" << std::endl;
	}

    //Cierra el socket del cliente y libera la estructura de datos
 	close(client_socket);
    	free(socket_desc);

    	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
        	std::cerr << "Uso: " << argv[0] << " <puerto>" << std::endl;
        	return 1;
    	}

int port = atoi(argv[1]);
int server_fd, new_socket;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);

// Crea el socket
if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    	std::cerr << "Error al crear el socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Se une el socket al puerto
if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Error al configurar el socket" << std::endl;
        exit(EXIT_FAILURE);
    }

	address.sin_family = AF_INET;
    	address.sin_addr.s_addr = INADDR_ANY;
    	address.sin_port = htons(port);

// Vincula el socket al puerto
if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error al vincular el socket" << std::endl;
        exit(EXIT_FAILURE);
    }

// El servidor estara esperando activamente nuevas conexiones entrantes
if (listen(server_fd, 3) < 0) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Esperando conexiones en el puerto " << port << "..." << std::endl;

    // Acepta conexiones entrantes en un bucle infinito
while (true) {
	if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            	std::cerr << "Error al aceptar la conexión" << std::endl;
            	continue; // Intenta aceptar la siguiente conexión
        }

        	std::cout << "Conexión aceptada. Creando hilo para manejar al cliente..." << std::endl;

        	// Crea un hilo para manejar al cliente
        	pthread_t thread;
        	int *client_socket = (int*)malloc(sizeof(int));
        	*client_socket = new_socket;
        if (pthread_create(&thread, NULL, manejar_cliente, (void*)client_socket) < 0) {
            std::cerr << "Error al crear el hilo" << std::endl;
            continue; // Intenta aceptar la siguiente conexión
        }

        pthread_detach(thread);
    }

    return 0;
}

