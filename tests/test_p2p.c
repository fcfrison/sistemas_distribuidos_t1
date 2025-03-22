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
// These unit tests were generated with the help of Deep Seek AI

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
    assert(pthread_create(&thread, NULL, listen_to_clt, args) == 0);
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

// Test 1: Successful connection to a valid server
void test_successful_connection() {
    int port = 9090;
    char address[] = "127.0.0.1";
    int* sockfd = NULL;

    // Start a mock server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);
    int val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_port = htons(port);

    assert(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0);
    assert(listen(server_fd, 1) == 0);

    // Test the dial function
    unsigned char result = dial(port, address, &sockfd);
    assert(result == 1); // Expected: Success
    assert(sockfd != NULL); // sockfd should be allocated
    assert(*sockfd >= 0); // Socket descriptor should be valid

    // Clean up
    close(*sockfd);
    free(sockfd);
    close(server_fd);
    printf("Test 1 Passed: Successful connection to a valid server\n");
};
// Test 2: Invalid address (connection fails)
void test_invalid_address() {
    int port = 8080;
    char address[] = "invalid.address";
    int* sockfd = NULL;

    // Test the dial function
    unsigned char result = dial(port, address, &sockfd);
    assert(result == 0); // Expected: Failure
    assert(sockfd == NULL); // sockfd should remain NULL

    printf("Test 2 Passed: Invalid address (connection fails)\n");
};
// Test 3: Invalid port (connection fails)
void test_invalid_port() {
    int port = -1; // Invalid port
    char address[] = "127.0.0.1";
    int* sockfd = NULL;

    // Test the dial function
    unsigned char result = dial(port, address, &sockfd);
    assert(result == 0); // Expected: Failure
    assert(sockfd == NULL); // sockfd should remain NULL

    printf("Test 3 Passed: Invalid port (connection fails)\n");
};

// Test 1: Valid IP and port extraction
void test_valid_ip_port_extraction() {
    char* ip = NULL;
    char* port = NULL;
    const char* to = "127.0.0.1:12345";
    const char* delim = ":";

    from_key_extract_ip_port(&ip, &port, to, delim);

    assert(ip != NULL); // IP should be allocated
    assert(port != NULL); // Port should be allocated
    assert(strcmp(ip, "127.0.0.1") == 0); // Expected IP: 127.0.0.1
    assert(strcmp(port, "12345") == 0); // Expected port: 12345

    free(ip);
    free(port);
    printf("Test 1 Passed: Valid IP and port extraction\n");
};
// Test 2: Missing port in the input string
void test_missing_port() {
    char* ip = NULL;
    char* port = NULL;
    const char* to = "127.0.0.1";
    const char* delim = ":";

    from_key_extract_ip_port(&ip, &port, to, delim);

    assert(ip == NULL);
    assert(port == NULL);
    printf("Test 2 Passed: Missing port in the input string\n");
}
// Test 3: Missing IP in the input string
void test_missing_ip() {
    char* ip = NULL;
    char* port = NULL;
    const char* to = ":12345";
    const char* delim = ":";

    from_key_extract_ip_port(&ip, &port, to, delim);

    assert(ip == NULL); // IP should be allocated
    assert(port == NULL); // Port should be allocated

    free(ip);
    free(port);
    printf("Test 3 Passed: Missing IP in the input string\n");
};
// Test 4: Empty input string
void test_empty_input_string() {
    char* ip = NULL;
    char* port = NULL;
    const char* to = "";
    const char* delim = ":";

    from_key_extract_ip_port(&ip, &port, to, delim);

    assert(ip == NULL);
    assert(port == NULL);
    printf("Test 4 Passed: Empty input string\n");
    return;
}
// Test 5: Custom delimiter
void test_custom_delimiter() {
    char* ip = NULL;
    char* port = NULL;
    const char* to = "127.0.0.1-12345";
    const char* delim = "-";

    from_key_extract_ip_port(&ip, &port, to, delim);

    assert(ip != NULL); // IP should be allocated
    assert(port != NULL); // Port should be allocated
    assert(strcmp(ip, "127.0.0.1") == 0); // Expected IP: 127.0.0.1
    assert(strcmp(port, "12345") == 0); // Expected port: 12345

    free(ip);
    free(port);
    printf("Test 5 Passed: Custom delimiter\n");
    return;
};
// Test 1: Keys are equal
void test_keys_are_equal() {
    const char* key_a = "test_key";
    const char* key_b = "test_key";

    const void* result = compare_keys(key_a, key_b);
    assert(result == key_a); // Expected: key_a (keys are equal)
    printf("Test 1 Passed: Keys are equal\n");
}

// Test 2: Keys are not equal
void test_keys_are_not_equal() {
    const char* key_a = "test_key";
    const char* key_b = "different_key";

    const void* result = compare_keys(key_a, key_b);
    assert(result == NULL); // Expected: NULL (keys are not equal)
    printf("Test 2 Passed: Keys are not equal\n");
}

// Test 3: One key is NULL
void test_one_key_is_null() {
    const char* key_a = "test_key";
    const char* key_b = NULL;

    const void* result = compare_keys(key_a, key_b);
    assert(result == NULL); // Expected: NULL (one key is NULL)
    printf("Test 3 Passed: One key is NULL\n");
}

// Test 4: Both keys are NULL
void test_both_keys_are_null() {
    const char* key_a = NULL;
    const char* key_b = NULL;

    const void* result = compare_keys(key_a, key_b);
    assert(result == NULL); // Expected: NULL (both keys are NULL)
    printf("Test 4 Passed: Both keys are NULL\n");
}

// Test 5: Empty strings as keys
void test_empty_strings_as_keys() {
    const char* key_a = "";
    const char* key_b = "";

    const void* result = compare_keys(key_a, key_b);
    assert(result == key_a); // Expected: key_a (empty strings are equal)
    printf("Test 5 Passed: Empty strings as keys\n");
}

// Test 6: One key is an empty string
void test_one_key_is_empty_string() {
    char* key_a = "test_key";
    char* key_b = "";

    const void* result = compare_keys(key_a, key_b);
    assert(result == NULL); // Expected: NULL (one key is an empty string)
    printf("Test 6 Passed: One key is an empty string\n");
};

// Test 1: Successful caching of a connection
void test_successful_caching() {
    char* key = "test_key";
    int fd = 123; // Mock socket file descriptor
    struct KeyValuePair* kvp = NULL;
    struct SimpleMap* sm = create_simple_map();
    cache_connection(key, &fd, &kvp, sm);
    KeyValuePair* rtn = get(sm,key,compare_keys);
    assert(strcmp((char*)rtn->key,key)==0);
    assert(kvp != NULL); // kvp should be allocated
    free(kvp->key);
    free(kvp);
    free(sm);
    printf("Test 1 Passed: Successful caching of a connection\n");
};
// Test 2: NULL key
void test_null_key() {
    const char* key = NULL;
    int fd = 123;
    struct KeyValuePair* kvp = NULL;
    struct SimpleMap* sm = create_simple_map();

    cache_connection(key, &fd, &kvp, sm);

    assert(kvp == NULL); // kvp should remain NULL
    assert(sm->top == -1); // No entry should be added to the map
    free(sm);
    printf("Test 2 Passed: NULL key\n");
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

    test_successful_connection();
    test_invalid_address();
    test_invalid_port();

    test_valid_ip_port_extraction();
    test_missing_port();
    test_missing_ip();
    test_empty_input_string();
    test_custom_delimiter();

    test_keys_are_equal();
    test_keys_are_not_equal();
    test_one_key_is_null();
    test_both_keys_are_null();
    test_empty_strings_as_keys();
    test_one_key_is_empty_string();

    test_successful_caching();
    test_null_key();
    return 0;
}