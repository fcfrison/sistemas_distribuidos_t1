#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <../include/p2plink.h>
// These unit test were implemented with the 
// Test 1: Valid positive number
void test_valid_positive_number() {
    char str[] = "12345";
    int result = from_str_to_int(str);
    assert(result == 12345); // Expected: 12345
    printf("Test 1 Passed: Valid positive number\n");
}

// Test 2: Valid negative number
void test_valid_negative_number() {
    char str[] = "-6789";
    int result = from_str_to_int(str);
    assert(result == -6789); // Expected: -6789
    printf("Test 2 Passed: Valid negative number\n");
}

// Test 3: Invalid input (contains non-digit characters)
void test_invalid_input_non_digit() {
    char str[] = "12a34";
    int result = from_str_to_int(str);
    assert(result == -1); // Expected: -1
    printf("Test 3 Passed: Invalid input (non-digit characters)\n");
}

// Test 4: Invalid input (empty string)
void test_invalid_input_empty_string() {
    char str[] = "";
    int result = from_str_to_int(str);
    assert(result == -1); // Expected: -1
    printf("Test 4 Passed: Invalid input (empty string)\n");
}

// Test 9: Invalid input (leading spaces)
void test_valid_input_leading_spaces() {
    char str[] = " 123";
    int result = from_str_to_int(str);
    assert(result == 123); // Expected: 123
    printf("Test 9 Passed: Valid input (leading spaces)\n");
}

// Test 10: Invalid input (trailing spaces)
void test_invalid_input_trailing_spaces() {
    char str[] = "123 ";
    int result = from_str_to_int(str);
    assert(result == -1); // Expected: -1
    printf("Test 10 Passed: Invalid input (trailing spaces)\n");
}
// Helper function to create a pair of connected sockets
int create_connected_socket_pair(const char* ip, int port, int* sockfd_client, int* sockfd_server) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }
    int option = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("setsockopt failed");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 1) < 0) {
        perror("listen failed");
        close(sockfd);
        return -1;
    }

    *sockfd_client = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd_client < 0) {
        perror("client socket creation failed");
        close(sockfd);
        return -1;
    }

    if (connect(*sockfd_client, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        close(*sockfd_client);
        return -1;
    }

    *sockfd_server = accept(sockfd, NULL, NULL);
    if (*sockfd_server < 0) {
        perror("accept failed");
        close(sockfd);
        close(*sockfd_client);
        return -1;
    }

    close(sockfd); // Close the listening socket
    return 0;
};

// Test 1: Valid IPv4 address and port
void test_valid_ipv4_and_port() {
    int sockfd_client, sockfd_server;
    int ret = create_connected_socket_pair("127.0.0.1", 10101, &sockfd_client, &sockfd_server);
    assert(ret == 0); // Ensure socket creation and connection succeeded

    char* ip_str = NULL;
    char* port_str = NULL;
    char rtn = get_peer_info_ipv4(sockfd_server, &ip_str, &port_str);

    assert(ip_str != NULL); // IP string should not be NULL
    assert(port_str != NULL); // Port string should not be NULL
    assert(strcmp(ip_str, "127.0.0.1") == 0); // Expected IP: 127.0.0.1
    assert(rtn==1);
    free(ip_str);
    free(port_str);
    close(sockfd_client);
    close(sockfd_server);
    printf("Test 1 Passed: Valid IPv4 address and port\n");
};
// Test 2: Invalid socket file descriptor
void test_invalid_socket_fd() {
    int sockfd = -1; // Invalid socket file descriptor
    char* ip_str = NULL;
    char* port_str = NULL;
    char rtn = get_peer_info_ipv4(sockfd, &ip_str, &port_str);

    assert(ip_str == NULL); // IP string should be NULL
    assert(port_str == NULL); // Port string should be NULL
    assert(rtn==-1);
    printf("Test 2 Passed: Invalid socket file descriptor\n");
};
// Test 1: Valid IP and port
void test_valid_ip_and_port() {
    const char* ip = "192.168.1.1";
    const char* port = "8080";
    char* result = format_rmt_add(ip, port);
    assert(strcmp(result, "192.168.1.1:8080") == 0); // Expected: "192.168.1.1:8080"
    free(result);
    printf("Test 1 Passed: Valid IP and port\n");
};
// Test 2: Empty IP string
void test_empty_ip_string() {
    const char* ip = "";
    const char* port = "8080";
    char* result = format_rmt_add(ip, port);
    assert(strcmp(result, ":8080") == 0); // Expected: ":8080"
    free(result);
    printf("Test 2 Passed: Empty IP string\n");
};

// Test 3: Empty port string
void test_empty_port_string() {
    const char* ip = "192.168.1.1";
    const char* port = "";
    char* result = format_rmt_add(ip, port);
    assert(strcmp(result, "192.168.1.1:") == 0); // Expected: "192.168.1.1:"
    free(result);
    printf("Test 3 Passed: Empty port string\n");
};
// Test 4: Both IP and port strings empty
void test_both_ip_and_port_empty() {
    const char* ip = "";
    const char* port = "";
    char* result = format_rmt_add(ip, port);
    assert(strcmp(result, ":") == 0); // Expected: ":"
    free(result);
    printf("Test 4 Passed: Both IP and port strings empty\n");
};
// Test 5: IP with leading/trailing spaces
void test_ip_with_spaces() {
    const char* ip = "  192.168.1.1  ";
    const char* port = "8080";
    char* result = format_rmt_add(ip, port);
    assert(strcmp(result, "  192.168.1.1  :8080") == 0); // Expected: "  192.168.1.1  :8080"
    free(result);
    printf("Test 5 Passed: IP with leading/trailing spaces\n");
};
// Test 6: Port with leading/trailing spaces
void test_port_with_spaces() {
    const char* ip = "192.168.1.1";
    const char* port = "  8080  ";
    char* result = format_rmt_add(ip, port);
    assert(strcmp(result, "192.168.1.1:  8080  ") == 0); // Expected: "192.168.1.1:  8080  "
    free(result);
    printf("Test 6 Passed: Port with leading/trailing spaces\n");
}
// Test 1: Valid client-server interaction
void test_valid_client_server_interaction(){
    int sockfd[2];//[0]-server [1]-client
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == 0);//using AF_UNIX is a good call
    PP2PLink* p2p = new_p2p_link(1);
    pthread_t thread;
    ListenSockArgs* args = init_listen_sock_args(sockfd[0], p2p);
    assert(pthread_create(&thread, NULL, listen_to_clnt, args) == 0);
    // Simulate a client sending a message
    const char* message = "Hello, server!";
    char msg_len[5];
    snprintf(msg_len, sizeof(msg_len), "%04d", (int)strlen(message));
    write(sockfd[1], msg_len, 4); // Send message length
    write(sockfd[1], message, strlen(message)); // Send message
    PP2PLink_Ind_Message* msg = receive_data(p2p->ind);
    assert(strcmp("Hello, server!",msg->message)==0);
    assert(strcmp("0.0.0.0:0",msg->from)==0);
    // Clean up
    close(sockfd[0]);
    close(sockfd[1]);
    pthread_cancel(thread); // Stop the listener thread
    pthread_join(thread, NULL);
    printf("\nTest 1 Passed: Valid client-server interaction\n");
};
int main() {
    test_valid_positive_number();
    test_valid_negative_number();
    test_invalid_input_non_digit();
    test_invalid_input_empty_string();
    test_valid_input_leading_spaces();
    test_invalid_input_trailing_spaces();

    test_valid_ipv4_and_port();
    test_invalid_socket_fd();

    test_valid_ip_and_port();
    test_empty_ip_string();
    test_empty_port_string();
    test_both_ip_and_port_empty();
    test_ip_with_spaces();
    test_port_with_spaces();


    test_valid_client_server_interaction();
    printf("All tests passed!\n");
    return 0;
}