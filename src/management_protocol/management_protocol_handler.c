#include "../../include/management_protocol/management_protocol_handler.h"

// Funciones privadas
static int send_get_request(int fd, uint8_t command);

static uint8_t receive_toggle_reply(int fd);
static void getUsers(int fd);
static void getPasswords(int fd);
// char* token = "somos_grupo_6";

void login(int fd, struct manage_args* args) {
    char * password = args->try_password;

    // Creamos primer mensaje
    size_t password_len = strlen(password);
    char* msg = malloc(password_len + 3);
    msg[0] = 0x00;
    msg[1] = password_len;
    strcpy((char *)(msg + 2), password);
    
    // using send due to connected state
    send(fd, msg, strlen(msg), MSG_NOSIGNAL);   // MSG_NOSIGNAL -> don't generate a SIGPIPE

    // recibir respuesta
    char res[1];
    recv(fd, res, 1, 0);

    switch(res[0]) {
        case CONN_STATUS_OK:
            printf("[INFO] AUTHORIZED\n");
            args->authorized = true;
            break;
        case CONN_STATUS_ERROR_IN_VERSION:
            fprintf(stderr, "[ERROR] VERSION NOT SUPPORTED\n");
            break;
        case CONN_STATUS_ERROR_IN_PASSLEN:
            fprintf(stderr, "[ERROR] INCORRECT TOKEN\n");
            break;
        case CONN_STATUS_ERROR_IN_PASS:
            fprintf(stderr, "[ERROR] INCORRECT PASSWORD\n");
            break;
        default:
            fprintf(stderr, "[ERROR] UNKNOWN RESPONSE\n");
    }
    
    // Liberar memoria reservada para el mensaje
    free(msg);
}

void executeCommands(int fd, struct manage_args* args) {
    if(args->list_flag) {
        switch (args->list_option)
        {
        case USERS:
            getUsers(fd);
            break;
        case PASSWORDS:
            break;
        case BUFFERSIZE:
            break;
        case AUTH_STATUS:
            break;
        case SPOOFING_STATUS:
            break;
        default:
            break;
        }
    }
    if(args->get_flag) {
        switch (args->get_option)
        {
        case SENT_BYTES:
            getSentBytes(fd);
            break;
        case RECEIVED_BYTES: 
            break;
        case HISTORIC_CONNECTIONS:
            break;
        case CONCURRENT_CONNECTIONS:
            break;
        default:
            break;
        }
    }
    if(args->add_flag) {
        
    }
    if(args->set_flag) {
        setBufferSize(fd, args->set_size);
    }
}

static void getUsers(int fd) {
    int sent_bytes = send_get_request(fd, 0x00);
    
    uint8_t status;
    char* users_list = (char*) receive_get_request(fd, &status);

    if(users_list == NULL) {
        return;
    }

    printf("USER LIST\n\n");
    printf("%s", users_list);

}

static void getPasswords(int fd) {

}

void getHistoricalConnections(int fd) {

}

void getConcurrentConections(int fd) {

}

void getSentBytes(int fd) {
    int sent_bytes = send_get_request(fd, 0x05);

    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if(reply == NULL) {
        if(status != STATUS_OK) {
            // printf de error (personalizar mensajes)
        } else {
            // UNKNOWN ERROR
        }
    }

}

void getReceivedBytes(int fd) {
    int sent_bytes = send_get_request(fd, 0x06);

    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if(reply == NULL) {
        if(status != STATUS_OK) {
            // printf de error (personalizar mensajes)
        } else {
            // UNKNOWN ERROR
        }
    }
}

static void addUser(int fd, char* username, char* password) {
    int sent_bytes = send_put_request(fd, username, password);

    
}

static void deleteUser(int fd, char* username) {

}

static void setBufferSize(int fd, unsigned int size) {
    int sent_bytes = send_set_request(fd, size);
}

// Requests y replies
static uint8_t send_delete_request(int fd, char* username) {
    // TODO preguntar
    int sent_bytes = 0;
    size_t username_len = strlen(username);
    uint8_t *request = NULL;
    // TODO por que 2 + ? y + 1?
    realloc(request, 2 + 2 * sizeof(int) + username_len + 1);
    request[0] = 0x05;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);

    sent_bytes = send(fd, request, strlen((char*) request), 0);

    free(request);
    return sent_bytes;
}

static uint8_t receive_delete_reply(int fd) {
    int rcv_bytes;
    uint8_t reply;
    rcv_bytes = recv(fd, &reply, 1, 0);

    return reply;
}

static uint8_t* send_receive_get(int fd, uint8_t command) {
    int sent_bytes = send_get_request(fd, command);

    if(sent_bytes <= 0) {
        return NULL;
    }
    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if(reply == NULL) {
        if(status != STATUS_OK) {
            // TODO: completar los status 
            printf("GET RESPONSE STATUS: ");
        } else {
            // Fijarse como responder a un status no conocido
            fprintf(stderr, "UNKNOWN RESPONSE");
        }
    }

    return reply;
}

static int send_get_request(int fd, uint8_t command) {
    int sent_bytes = 0;
    enum get_status status;

    uint8_t request[2];

    request[0] = 0x00;      // Action GET
    request[1] = command;   // Command passed

    // Return number of sent bytes / -1 if error
    sent_bytes = send(fd, request, 2, 0);

    free(request);

    if(sent_bytes <= 0) {
        fprintf(stderr, "[ERROR] GET REQUEST NOT SENT\n");
        // goto: close(fd) ...
    }

    return sent_bytes;
}

static uint8_t* receive_get_request(int fd, uint8_t* status) {
    int recv_bytes;
    uint8_t* info[2] = malloc(2);

    recv_bytes = recv(fd, info, 2, 0);

    if(recv_bytes < 2) {
        *status = SERVER_ERROR;
        return NULL;
    }

    *status = info[0];
    // Solo nos interesa guardar la respuesta con STATUS_OK
    if(*status != STATUS_OK) {
        return NULL;
    }

    size_t rta_len = info[1];
    uint8_t* rta = malloc(rta_len + 1);

    // Chequear si hay espacio suficiente
    if(rta == NULL) {
        return NULL;
    }

    recv(fd, rta, rta_len + 1, 0);

    rta[rta_len] = '\0';

    return rta;
}

static void send_receive_put(int fd, char* username, char* password) {
    int sent_bytes = send_put_request(fd, username, password);

    // TODO: Chequear si terminamos el programa aca
    if(sent_bytes <= 0) {
        fprintf(stderr, "[ERROR] PUT REQUEST NOT SENT");
        // exit(0);
        // tendria que ser un goto
    }

    uint8_t status;
    int rcv_bytes = receive_put_reply(fd, &status);

    if(recv_bytes <= 0) {
        fprintf(stderr, "[ERROR] PUT REPLY NOT RECEIVED");
        // goto
    } else {
        printf("PUT RESPONSE STATUS: ");
    }
}

static int send_put_request(int fd, char* username, char* password) {
    int sent_bytes = 0;
    uint8_t *request = NULL;
    size_t username_len = strlen(username);
    size_t password_len = strlen(password);

    realloc(request, 2 + 2*sizeof(int) + username_len + password_len + 1);
    request[0] = 0x01;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);
    request[3 + username_len] = password_len;
    strcpy((char*) (request + 4 + username_len), password);

    sent_bytes = send(fd, request, strlen((char*) request), MSG_NOSIGNAL);

    free(request);

    return sent_bytes;
}

static int receive_put_reply(int fd, uint8_t* status) {
    int rcv_bytes;
    uint8_t reply[1];
    rcv_bytes = recv(fd, reply, 1, 0);

    return rcv_bytes;
}

static int send_configbuffsize_request(int fd, unsigned int size) {
    int sent_bytes = 0;
    uint8_t request[2];

    request[0] = 0x03;
    request[1] = size;

    sent_bytes = send(fd, request, 2, MSG_NOSIGNAL);

    free(request);
    return sent_bytes;
}

static uint8_t receive_configbuffsize_reply(int fd) {
    int rcv_bytes;
    uint8_t reply[1];
    rcv_bytes = recv(fd, reply, 1, 0);

    return reply;
}

// TOGGLE 
static uint8_t send_and_receive_configstatus(int fd, uint8_t field, uint8_t status) {

    if(send_and_receive_configstatus(fd, field, status) <= 0) {
        // ERROR
    }


}

// TOGGLE
static int send_configstatus_request(int fd, uint8_t field, uint8_t status) {
    int sent_bytes;
    uint8_t request[3];

    request[0] = 0x04;
    request[1] = field;
    request[2] = status;

    sent_bytes = send(fd, request, 3, 0);

    if(sent_bytes) {
        // server error?
    }

    return sent_bytes;
}

// TOGGLE
static uint8_t receive_configstatus_reply(int fd) {
    int rcv_bytes;
    uint8_t reply;
    rcv_bytes = recv(fd, &reply, 1, 0);

    return reply;
}