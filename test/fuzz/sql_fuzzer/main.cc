#include <iostream>
#include <cstring>

#include <tarantool/tarantool.h>
#include <tarantool/tnt_net.h>
#include <tarantool/tnt_opt.h>

int main() {
    const char *query = "CREATE TABLE t (s1 SCALAR PRIMARY KEY)";
    auto args = tnt_object(NULL);
	tnt_object_format(args, "[]");

    const char *uri = "localhost:3301";
    struct tnt_stream *tnt = tnt_net(NULL); // Allocating stream
    tnt_set(tnt, TNT_OPT_URI, uri); // Setting URI
    tnt_set(tnt, TNT_OPT_SEND_BUF, 0); // Disable buffering for send
    tnt_set(tnt, TNT_OPT_RECV_BUF, 0); // Disable buffering for recv
    int connection_result = tnt_connect(tnt); // Initialize stream and connect to Tarantool
    std::cout << connection_result << std::endl;

    ssize_t execute_result = tnt_execute(tnt, query, strlen(query), args);
    std::cout << execute_result << std::endl;

    struct tnt_reply *reply = tnt_reply_init(NULL); // Initialize reply
    int reply_result = tnt->read_reply(tnt, reply); // Read reply from server
    std::cout << reply_result << std::endl;
    std::cout << reply->error << "\n" << reply->sqlinfo << "\n" << reply->metadata << "\n" 
        << reply->data << std::endl;

    tnt_reply_free(reply); // Free reply
    tnt_stream_free(args);
    tnt_close(tnt); tnt_stream_free(tnt); // Close connection and free stream object
}