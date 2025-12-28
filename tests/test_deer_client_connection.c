/**
 * @file test_deer_client_connection.c
 * @brief Connectivity probe test for the Deer client.
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "deer/client.h"

typedef struct {
    int server_fd;
    uint16_t port;
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t ready;
    int ready_flag;
} MockServer;

static void *mock_server_run(void *data) {
    MockServer *server = (MockServer *)data;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server->server_fd >= 0);
    assert(bind(server->server_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0);
    assert(listen(server->server_fd, 1) == 0);

    socklen_t len = sizeof(addr);
    assert(getsockname(server->server_fd, (struct sockaddr *)&addr, &len) == 0);
    server->port = ntohs(addr.sin_port);

    pthread_mutex_lock(&server->lock);
    server->ready_flag = 1;
    pthread_cond_signal(&server->ready);
    pthread_mutex_unlock(&server->lock);

    int client_fd = accept(server->server_fd, NULL, NULL);
    if (client_fd >= 0) {
        char buffer[64] = {0};
        recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        const char pong[] = "PONG\n";
        send(client_fd, pong, sizeof(pong) - 1, 0);
        close(client_fd);
    }

    close(server->server_fd);
    return NULL;
}

static void mock_server_start(MockServer *server) {
    memset(server, 0, sizeof(*server));
    pthread_mutex_init(&server->lock, NULL);
    pthread_cond_init(&server->ready, NULL);
    pthread_create(&server->thread, NULL, mock_server_run, server);
}

static void mock_server_wait_ready(MockServer *server) {
    pthread_mutex_lock(&server->lock);
    while (!server->ready_flag) {
        pthread_cond_wait(&server->ready, &server->lock);
    }
    pthread_mutex_unlock(&server->lock);
}

static void mock_server_join(MockServer *server) {
    pthread_join(server->thread, NULL);
    pthread_cond_destroy(&server->ready);
    pthread_mutex_destroy(&server->lock);
}

int main(void) {
    MockServer server;
    mock_server_start(&server);
    mock_server_wait_ready(&server);

    DeerClient client;
    DeerClientConfig config = {"127.0.0.1", server.port};
    deer_init(&client, &config);

    char status[128] = {0};
    assert(deer_client_test_connection(&client, status, sizeof(status)));
    assert(strstr(status, "Connected") != NULL);

    mock_server_join(&server);
    deer_client_disconnect(&client);
    return 0;
}
