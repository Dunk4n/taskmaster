#include "taskmaster.h"

uint8_t send_text(struct taskmaster *taskmaster, char *line)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_daemon == FALSE && taskmaster->global_status.global_status_start_as_client == FALSE)
        return (EXIT_FAILURE);
    if(line == NULL)
        return (EXIT_FAILURE);

    ssize_t size;

    size = 0;

    if(taskmaster->global_status.global_status_start_as_daemon == TRUE)
        size = send(taskmaster->client_socket, line, strlen(line), MSG_NOSIGNAL); //MSG_DONTWAIT
    else
        size = send(taskmaster->socket, line, strlen(line), MSG_NOSIGNAL); //MSG_DONTWAIT
    if(size == -1)
        {
        return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }

uint8_t recv_text(struct taskmaster *taskmaster, uint8_t *buffer)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_daemon == FALSE && taskmaster->global_status.global_status_start_as_client == FALSE)
        return (EXIT_FAILURE);
    if(buffer == NULL)
        return (EXIT_FAILURE);

    ssize_t size;

    size = 0;

    if(taskmaster->global_status.global_status_start_as_daemon == TRUE)
        size = recv(taskmaster->client_socket, buffer, LINE_SIZE, 0); //MSG_DONTWAIT
    else
        size = recv(taskmaster->socket, buffer, LINE_SIZE, 0); //MSG_DONTWAIT
    if(size == -1)
        {
        //TODO rm errno
        perror(strerror(errno));
        return (EXIT_FAILURE);
        }
    if(size == 0)
        return (EXIT_FAILURE);

    if(size <= LINE_SIZE)
        buffer[size] = NIL;
    else
        buffer[LINE_SIZE] = NIL;

    return (EXIT_SUCCESS);
    }

uint8_t recv_text_and_display(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);

    uint8_t  buffer[LINE_SIZE + 1];
    uint32_t cnt;

    buffer[0]         = NIL;
    buffer[LINE_SIZE] = NIL;
    cnt               = 0;

    while(TRUE)
        {
        buffer[0] = NIL;
        if(recv_text(taskmaster, buffer) != EXIT_SUCCESS)
            return (EXIT_FAILURE);
        cnt = 0;
        while(buffer[cnt] != NIL)
            {
            if(strncmp((char *) (buffer + cnt), EXIT_CLIENT_MARKER, EXIT_CLIENT_MARKER_LENGTH) == 0)
                {
                buffer[cnt] = NIL;
                taskmaster->global_status.global_status_exit = TRUE;

                if(write(STDIN_FILENO, buffer, strlen((char *) buffer)) == -1)
                    return (EXIT_FAILURE);
                return (EXIT_SUCCESS);
                }
            if(strncmp((char *) (buffer + cnt), END_OF_MESSAGE_MARKER, END_OF_MESSAGE_MARKER_LENGTH) == 0)
                {
                buffer[cnt] = NIL;

                if(write(STDIN_FILENO, buffer, strlen((char *) buffer)) == -1)
                    return (EXIT_FAILURE);
                return (EXIT_SUCCESS);
                }

            cnt++;
            }

        if(write(STDIN_FILENO, buffer, strlen((char *) buffer)) == -1)
            return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }

uint8_t send_command_line_to_daemon(struct taskmaster *taskmaster, char *line)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(line == NULL)
        return (EXIT_FAILURE);

    if(send_text(taskmaster, line) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(recv_text_and_display(taskmaster) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

uint8_t init_taskmaster_client(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_daemon == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_client == FALSE)
        return (EXIT_FAILURE);

    memset(&taskmaster->addr, 0, sizeof(taskmaster->addr));

    taskmaster->socket = socket(AF_INET, SOCK_STREAM, 0);
    if(taskmaster->socket == -1)
        {
        taskmaster->socket = -1;
        return (EXIT_FAILURE);
        }

    taskmaster->addr.sin_family = AF_INET;
    taskmaster->addr.sin_addr.s_addr = 0;
    taskmaster->addr.sin_port = htons(DEAMON_PORT);

    if(connect(taskmaster->socket, (struct sockaddr *)&taskmaster->addr, sizeof(taskmaster->addr)) < 0)
        {
        close(taskmaster->socket);
        taskmaster->socket = -1;

        //TODO error msg client can not connect to the daemon
        printf("Can not connect the client to the daemon\n");
        return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }
