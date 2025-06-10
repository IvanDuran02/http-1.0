#include "parser/parser.h"
#include "http_connection/http_connection.h"
#include <cassert>
#include <iostream>

void test_simple_get() {
    HttpParser parser;
    HttpMessage msg;

    const std::string raw = "GET /foo HTTP/1.0\r\n"
                            "Host: example.com\r\n"
                            "\r\n";

    parser.feed(raw);
    bool ok = parser.parse(msg);
    assert(ok);
    assert(msg.method == HttpMethod::GET);
    assert(msg.request_uri == "/foo");
    assert(msg.headers["Host"] == "example.com");
    assert(msg.body.empty());
}

int main() {
    test_simple_get();
    std::cout << "Tests passed" << std::endl;
    http_connection();
}
