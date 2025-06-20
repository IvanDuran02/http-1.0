#include "../../parser/parser.h"
#include <cassert>
#include <iostream>
#include <ostream>
#include <stdexcept>

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

void test_post_in_fragments() {
    HttpParser parser;
    HttpMessage msg;

    // feed header first, without the final \r\n
    parser.feed("POST /submit HTTP/1.0\r\nContent-Length: 11\r\n\r\n");
    // at this point, parse() should still be waiting for the body
    assert(!parser.parse(msg));

    // feed the body in two parts
    parser.feed("hello");
    assert(!parser.parse(msg)); // still missing 6 bytes
    parser.feed(" world");
    assert(parser.parse(msg)); // now complete

    assert(msg.method == HttpMethod::POST);
    assert(msg.request_uri == "/submit");
    assert(msg.body == "hello world");
}

void test_unsupported_method() {
    HttpParser parser;
    HttpMessage msg;
    parser.feed("PUT /x HTTP/1.0\r\n\r\n");
    try {
        parser.parse(msg);
        assert(false && "Expected exception on unsupported method");
    } catch (const std::runtime_error &e) {
        // OK
    }
}

int main() {
    test_simple_get();
    test_post_in_fragments();
    test_unsupported_method();

    std::cout << "All tests passed" << std::endl;
    return 0;
}
