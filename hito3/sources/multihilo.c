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

extern pthread_mutex_t lock;
pthread_cond_t cond_book;
pthread_cond_t cond_free;
int count = 0;
int end = 0;

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
 * @function   book_room
 * @abstract   Books three rooms.
 * @discussion Books three rooms with randomized IDs between 0 and 999.
 *                
 *
 * @param      arg        Argument required by specification.
*/
void* book_room (void* arg) {
    pthread_mutex_lock(&lock);
    count++;
    pthread_mutex_unlock(&lock);

    for (int i = 0; (i < 3) && !end; i++) {
        pthread_mutex_lock(&lock);
        // loops until there is space available
        while ((asientos_ocupados() == capacidad()) && !end) {
            pthread_cond_wait(&cond_free, &lock);
        }
        pthread_mutex_unlock(&lock);

        // check if book fails
        if (reserva_asiento(rand() % 10000) == -1) {
            i--;
        }

        pthread_mutex_lock(&lock);
        pthread_cond_signal(&cond_book);
        pthread_mutex_unlock(&lock);

        pausa_aleatoria(2);
    }

    pthread_mutex_lock(&lock);
    count--;
    pthread_cond_signal(&cond_book);
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

/*!
 * @function   free_room
 * @abstract   Frees three rooms.
 * @discussion Frees three last rooms.
 *                
 *
 * @param      arg        Argument required by specification.
*/
void* free_room (void* arg) {
    pthread_mutex_lock(&lock);
    count++;
    pthread_mutex_unlock(&lock);

    for (int i = 0; (i < 3) && !end; i++) {
        pthread_mutex_lock(&lock);
        // loops until there is at least one book
        while ((asientos_ocupados() == 0) && !end) {
            pthread_cond_wait(&cond_book, &lock);
        }
        pthread_mutex_unlock(&lock);

        // check if free fails
        if (libera_ultimo_asiento() == -1) {
            i--;
        }

        pthread_mutex_lock(&lock);
        pthread_cond_signal(&cond_free);
        pthread_mutex_unlock(&lock);

        pausa_aleatoria(2);
    }

    pthread_mutex_lock(&lock);
    count--;
    pthread_mutex_unlock(&lock);

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
    while(!end) {
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
    
    int n, m, c;
    n = m = c = 0;
    
    if (set_err((argc != 4) && (argc != 3), "Wrong number of arguments", 1) == -1) exit(1);

    n = atoi(argv[1]);   

    m = atoi(argv[2]);

    if (argc == 4) {
        c = atoi(argv[3]);
    } else {
        c = n * 3;
    }

    if (set_err((n < 1) || (m < 1) || (c < 0), "Wrong number of threads.", 1) == -1) exit(1);

    pthread_t* booking = calloc(n, sizeof(pthread_t));
    pthread_t* freeing = calloc(m, sizeof(pthread_t));
    pthread_t prints_chairs;
    
    crea_sala(c);

    if (set_err(pthread_cond_init(&cond_book, NULL), "Booking condition declaration failed.", 1) == -1) exit(1);     // initializes book condition
    if (set_err(pthread_cond_init(&cond_free, NULL), "Freeing condition declaration failed.", 1) == -1) exit(1);     // initializes free condition
    
    // creates threads
    int n_aux, m_aux, t_aux;
    n_aux = m_aux = 0;
    for (int i = 0; i < (n + m); i++) {
        if ((n_aux < n) && (m_aux < m)) {
            t_aux = rand() % 2;
        } else if (n_aux == n) {
            t_aux = 0;
        } else {
            t_aux = 1;
        }
        if (t_aux) {
            set_err(pthread_create(booking + n_aux, NULL, book_room, NULL), "Create thread for booking failed",1);
            n_aux++;
        } else {
            set_err(pthread_create(freeing + m_aux, NULL, free_room, NULL), "Create thread for freeing failed",1);
            m_aux++;
        }
    }

    set_err(pthread_create(&prints_chairs, NULL, print_chairs, NULL), "Create thread for printing failed",1);

    // join threads
    void* dummy;
    int mark = capacidad();

    if ((n - m) != 0) {
        mark = (n - m) * 3;
    }

    if (((n - m)*3) > capacidad()) {
        mark = capacidad();
    }

    if (n > m) {
        
        for (int i = 0; i < m; i++) {
            set_err(pthread_join(*(freeing + i), &dummy), "Join thread for freeing failed.", 1);
        }
        
        while((mark > asientos_ocupados())) {
            pthread_mutex_lock(&lock);
            pthread_cond_signal(&cond_free);
            pthread_mutex_unlock(&lock);
        }

        end = 1;    // flag for stopping threads
        pthread_mutex_lock(&lock);
        pthread_cond_broadcast(&cond_free);
        pthread_mutex_unlock(&lock);
        // waits for booking to end
        for (int i = 0; i < n; i++) {
            set_err(pthread_join(*(booking + i), &dummy), "Join thread for booking failed.", 1);
        }
    } else if (n < m) {
        // waits for booking to end
        for (int i = 0; i < n; i++) {
            set_err(pthread_join(*(booking + i), &dummy), "Join thread for booking failed.", 1);
        }
        
        // waits until room is empty
        while(asientos_ocupados() > 0) {
            pthread_mutex_lock(&lock);
            pthread_cond_signal(&cond_book);
            pthread_mutex_unlock(&lock);
        }

        end = 1;    // flag for stopping threads
        pthread_mutex_lock(&lock);
        pthread_cond_broadcast(&cond_book);
        pthread_mutex_unlock(&lock);

        // waits for freeing to end
        for (int i = 0; i < m; i++) {
            set_err(pthread_join(*(freeing + i), &dummy), "Join thread for freeing failed.", 1);
        }
    } else {
        // waits for booking to end
        for (int i = 0; i < n; i++) {
            set_err(pthread_join(*(booking + i), &dummy), "Join thread for booking failed.", 1);
        }

        // waits for freeing to end
        for (int i = 0; i < m; i++) {
            set_err(pthread_join(*(freeing + i), &dummy), "Join thread for freeing failed.", 1);
        }
    }
    //set_err(pthread_join(prints_chairs, &dummy), "Join thread for printing failed.",1);
    // end of program
    printf("[!] Program Ends with %d reservations.\n", asientos_ocupados());
    pthread_cond_destroy(&cond_book);													           // frees condition
    pthread_cond_destroy(&cond_free);													           // frees condition
    elimina_sala();

    free(freeing);
    free(booking);
    exit(0);
}