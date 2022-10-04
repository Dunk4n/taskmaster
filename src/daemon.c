#include "taskmaster.h"

uint8_t daemon_accept_new_connection(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_client == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_daemon == FALSE)
        return (EXIT_FAILURE);

    taskmaster->client_socket = accept(taskmaster->socket, NULL, NULL);
    if(taskmaster->client_socket == -1)
        {
        close(taskmaster->socket);
        taskmaster->socket = -1;
        taskmaster->client_socket = -1;

        //TODO error msg daemon can not accept new client
        return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }

uint8_t init_taskmaster_daemon(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_client == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_daemon == FALSE)
        return (EXIT_FAILURE);

    uint16_t port;

    port = 0;

    memset(&taskmaster->addr, 0, sizeof(taskmaster->addr));

    taskmaster->socket = socket(AF_INET, SOCK_STREAM, 0);
    if(taskmaster->socket == -1)
        {
        taskmaster->socket = -1;
        return (EXIT_FAILURE);
        }

    taskmaster->addr.sin_family = AF_INET;
    taskmaster->addr.sin_addr.s_addr = 0;
    port = 8001;
    taskmaster->addr.sin_port = port >> 8 | port << 8;
    if(bind(taskmaster->socket, (const struct sockaddr *)&taskmaster->addr, sizeof(taskmaster->addr)) < 0)
        {
        close(taskmaster->socket);
        taskmaster->socket = -1;
        return (EXIT_FAILURE);
        }
    if (listen(taskmaster->socket, 1) < 0)
        {
        close(taskmaster->socket);
        taskmaster->socket = -1;
        return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }
