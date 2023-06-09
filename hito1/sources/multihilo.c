/*!
 *  @author   Chamil José Cruz Razeq
 *  
 *  @version  0.0.0.0
 *  @date     10-05-23
 *
 *  @abstract Operative Systems Fundamentals Lab 3
*/

/** ----------------------------------------------------------------------------
 *  LIBRARIES
 *  --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "../headers/retardo.h"
#include "../headers/sala.h"


/** ----------------------------------------------------------------------------
 *  AUXILIARY METHODS
 *  --------------------------------------------------------------------------*/

/*!
 * @function   print_error
 * @abstract   Prints error.
 * @discussion If there is a error passed by errno, prints given message and
 +             perror message.
 *                
 *
 * @param      err_msg  Error message.     
 *
 * @return     Returns 0 if an error was caught and prints message and perror,
 *             if there are no errors returns -1.
*/
int print_error(const char* err_msg) {
  if (errno != 0) {
    fprintf(stderr, "[!] ERROR %d: ", errno);
    perror(err_msg);
    errno=0;
    
    return 0;
  }
  
  return -1;
}

/*!
 * @function   set_err
 * @abstract   Sets error.
 * @discussion If state is 1, sets given error and prints given message.
 *                
 *
 * @param      state      Result of condition evaluated.
 * @param      err_msg    Error message.
 * @param      err_number Error number.
 *
 * @return     Returns -1 if an error was caught, sets errno and prints message
 *             and perror, if there are no errors returns 0.
*/
int set_err(const int state, const char* err_msg, const int err_number) {
  if (state) {
    errno = err_number;
    print_error(err_msg);
    
    return -1;
  }
  
  return 0;
} 

/** ----------------------------------------------------------------------------
 *  ROUTINES
 *  --------------------------------------------------------------------------*/

/*!
 * @function   manage_room
 * @abstract   Books and frees three rooms.
 * @discussion Books  three rooms with randomized ids and frees three last rooms.
 *                
 *
 * @param      arg        Argument required by specification.
*/
void* manage_room (void* arg) {
    int* chairs = calloc(3, sizeof(int));
    
    // books three rooms
    for (int i = 0; i < 3; i++) {

        if (set_err(asientos_libres() == 0, "Not enough chairs", 1) == -1) pthread_exit(NULL);

        *(chairs + i) = reserva_asiento(rand() % 1000);

        if (set_err(*(chairs + i) == -1, "Booking error", 1) == -1) pthread_exit(NULL);

        pausa_aleatoria(2);
    }
    
    // frees three rooms
    for (int i = 0; i < 3; i++) {

        if (set_err(asientos_ocupados() == 0, "No chairs to free", 1) == -1) pthread_exit(NULL);

        if (set_err(libera_asiento(*(chairs + i)) == -1, "Freeing error", 1) == -1) pthread_exit(NULL);

        pausa_aleatoria(2);
    }

    free(chairs);

    pthread_exit(NULL);
}

/*!
 * @function   print_chairs
 * @abstract   Prints a table of contents of the room.
 *
 * 
 * @param      arg        Argument required by specification. Used to get number of booking threads from main.
*/
void* print_chairs(void* arg) {
    while(1) {
        pausa_aleatoria(2);
        
        imprime_asientos();
    }

    pthread_exit(NULL);
}

/** ----------------------------------------------------------------------------
 *  MAIN
 *  --------------------------------------------------------------------------*/

int main(int argc, char* argv[]) {
    printf("[!] Program Starts.\n");
    srand(getpid());

    int arg, n, c;
    arg = n = c = 0;
    
    if (set_err((argc != 2) && (argc != 3), "Wrong number of arguments", 1) == -1) exit(1);

    n = atoi(argv[1]);

    if (argc == 3) {
        c = atoi(argv[2]);   
    } else {
        c = n * 3;
    }

    if (set_err((n < 1) || (c < 0), "Wrong number of threads.", 1) == -1) exit(1);

    pthread_t* manages_room = calloc(n, sizeof(pthread_t));
    pthread_t prints_chairs;

    crea_sala(c);

    for (int i = 0; i < n; i++) {
        set_err(pthread_create(manages_room + i, NULL, manage_room, NULL), "Create thread 1 failed",1);
    }

    set_err(pthread_create(&prints_chairs, NULL, print_chairs, NULL), "Create thread 2 failed",1);

    void* dummy;
    for (int i = 0; i < n; i++) {
        set_err(pthread_join(*(manages_room + i), &dummy), "Join thread 2 failed.",1);
    }

    printf("[!] Program Ends with %d reservations.\n", asientos_ocupados());
    elimina_sala();
    free(manages_room);
    exit(0);
}