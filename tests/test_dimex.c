#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/DIMEX.h"
// Test 1: Basic functionality test with normal inputs
void test_basic_functionality() {
    char* fmt_msg = NULL;
    char* entry_msg = "Request";
    char* sep = "|";
    int id = 123;
    int req_ts = 456789;
    
    format_req_entry_message(&fmt_msg, entry_msg, sep, id, req_ts);
    char expected[50];
    sprintf(expected, "%s%s%d%s%d", entry_msg, sep, id, sep, req_ts);
    
    if (strcmp(fmt_msg, expected) == 0) {
        printf("Test 1 Passed\n");
    } else {
        printf("Test 1 Failed\n");
    }
    
    free(fmt_msg);
};
int main(void){
    test_basic_functionality();
}