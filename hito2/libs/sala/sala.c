/*!
 *  @author   Chamil José Cruz Razeq
 *  
 *  @version  1.0.0.0
 *  @date     09-03-23
 *
 *  @abstract Operative Systems Fundamentals Lab 3
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

pthread_mutex_t lock;

struct {
	int* asientos;
	int n_asientos, o_asientos;
} sala;

/*!
 * @function   existe_id
 * @abstract   Checks if client ID exists.
 * @discussion Checks if client ID exists and returns 1, if not returns 0.
 *                
 *
 * @param      id       Client ID to check.     
 *
 * @return     Returns 0 if not found and 1 if found..
*/
int existe_id(int id) {
	int existe = 0;

	if (id >= 0) {
		for (int i = 0; i < sala.n_asientos; i++) {
			if (*(sala.asientos + i) == id) {
				existe = 1;
			}
		}
	}

	return existe; 
}

/*!
 * @function   reserva_asiento_concreto
 * @abstract   Books a chair for given client ID in given chair.
 * @discussion Books a chair for given client ID in given chair, if successful
 *             returns 0, if not returns -1.
 *                
 *
 * @param      id       Client ID.
 * @param      pos      Chair index.
 *
 * @return     Chair assigned or -1 if not successful.
*/
int reserva_asiento_concreto(int id, int pos) {
	int success = -1;

    // checks if id is greater than 0 and if room is not empty
	if ((!existe_id(id)) && (sala.o_asientos < sala.n_asientos) && (id >= 0)) {
	  if((*(sala.asientos + pos) == -1)) {
	    *(sala.asientos + pos) = id;
	    success = 0;
	    sala.o_asientos++;
	  }
	}

	return success;
}

/*!
 * @function   reserva_asiento
 * @abstract   Books a chair for given client ID.
 * @discussion Books a chair for given client ID, if successful returns the
               first available chair found, if not returns -1.
 *                
 *
 * @param      id       Client ID.
 *
 * @return     Chair assigned or -1 if not successful.
*/
int reserva_asiento(int id) {
	int asiento = -1;
    
	// critical section
	pthread_mutex_lock(&lock);

    // checks if id is greater than 0 and if room is not empty
	for (int i = 0; i < sala.n_asientos; i++) {
	  if(reserva_asiento_concreto(id, i) == 0) {
	    asiento = i;
	    break;
	  }
	}

	pthread_mutex_unlock(&lock);
	// critical section
	
	return asiento;
}

/*!
 * @function   libera_asiento
 * @abstract   Frees given chair.    
 * @discussion Frees given chair, if successful returns the ID of the client that
               had booked the chair, if not returns -1.
 *                
 *
 * @param      id       Client ID.
 *
 * @return     ID of client or -1 if not successful.
*/
int libera_asiento(int asiento) {
	int id = -1;
        
    // checks if chair is in the interval [chair, capacity] and if the room
    // is not empty
	
	// critical section
	pthread_mutex_lock(&lock);																   	  // mutex lock
	
	if ((asiento >= 0) && (asiento < sala.n_asientos) && (*(sala.asientos + asiento) != -1)) {	
		id = *(sala.asientos + asiento);
		*(sala.asientos + asiento) = -1;
		sala.o_asientos--;
	}

	pthread_mutex_unlock(&lock);																   // mutex unlock
	// critical section

	return id;
}

/*!
 * @function   libera_ultime_asiento
 * @abstract   Frees last chair.    
 * @discussion Frees last chair, if successful returns the ID of the client that
               had booked the chair, if not returns -1.
 *                
 *
 * @param      id       Client ID.
 *
 * @return     ID of client or -1 if not successful.
*/
int libera_ultimo_asiento() {
	int id = -1;
        
    // checks if chair is in the interval [chair, capacity] and if the room
    // is not empty
	
	// critical section
	pthread_mutex_lock(&lock);															   // mutex lock

	int asiento = sala.o_asientos - 1;
	
	if ((asiento >= 0) && (asiento < sala.n_asientos) && (*(sala.asientos + asiento) != -1)) {
		id = *(sala.asientos + asiento);
		*(sala.asientos + asiento) = -1;
		sala.o_asientos--;
	}

	pthread_mutex_unlock(&lock);														   // mutex unlock
	// critical section

	return id;
}

/*!
 * @function   estado_asiento
 * @abstract   Checks if the chair is booked and by whom.
 * @discussion Checks if the chair is booked and returns ID of client or -1 if
               unsuccessful or available.
 *                
 *
 * @param      asiento   Chair to check.
 *
 * @return     ID of client or -1 if available or unsuccessful.
*/
int estado_asiento(int asiento) {
	int id = -1;
      
        // checks if asiento is in the interval [chair, capacity]
	if ((asiento < sala.n_asientos) && (asiento >= 0)) {
		id = *(sala.asientos + asiento);
	}

	return id;
}

/*!
 * @function   asientos_libres
 * @abstract   Checks the number of available chairs.
 *
 * @return     Number of available chairs.
*/
int asientos_libres() {
	return sala.n_asientos - sala.o_asientos;
}

/*!
 * @function   asientos_ocupados
 * @abstract   Checks the number of booked chairs.
 *
 * @return     Number of booked chairs.
*/
int asientos_ocupados() {
	return sala.o_asientos;
}

/*!
 * @function   capacidad
 * @abstract   Checks the capacity of room.
 *
 * @return     Capacity of room.
*/
int capacidad() {
	return sala.n_asientos;
}

/*!
 * @function   crea_sala
 * @abstract   Creates new room.
 * 
 * @param      capacidad       The capacity of the room.
*/
void crea_sala(int capacidad) {
    // checks if capacidad is positive. if not sets to 0
	if (capacidad < 0) capacidad = 0;
	
	sala.asientos = calloc(capacidad, sizeof(int));

	sala.n_asientos = capacidad;
	sala.o_asientos = capacidad;
        
    // sets each chair to default value (-1)
	for (int i = 0; i < capacidad; i++) {
		libera_asiento(i);
	}

	pthread_mutex_init(&lock, NULL);															   // initializes mutex
}

/*!
 * @function   elimina_sala
 * @abstract   Removes room.
*/
void elimina_sala() {
	free(sala.asientos);
	sala.asientos = NULL;
	sala.n_asientos = 0;
	sala.o_asientos = 0;
	pthread_mutex_destroy(&lock);																   // frees mutex
}

/*!
 * @function   imprime_asientos
 * @abstract   Prints chairs and clients id as a table.
*/
void imprime_asientos() {
	// critical section
	pthread_mutex_lock(&lock);

	//  prints the header of the table
	printf("-------------------\n"
			"| Asie |    ID    |\n"
			"-------------------\n");
	
	for(int i = 0; i < capacidad(); i++) {
		int id = estado_asiento(i);
	
		//  checks if chair has client assigned
		if (id != -1) {
		//  prints each row of the table
		printf("| %4d | %8d |\n"
				"-------------------\n", i, id);
		} 
	}

	pthread_mutex_unlock(&lock);
}