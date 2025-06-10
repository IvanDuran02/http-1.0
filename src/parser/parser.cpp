#include "parser.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

HttpMethod HttpParser::parse_method(const std::string &token) const {
    if (token == "GET")
        return HttpMethod::GET;
    if (token == "POST")
        return HttpMethod::POST;
    if (token == "HEAD")
        return HttpMethod::HEAD;

    return HttpMethod::UNKNOWN;
}

// Feed will push new bytes into parser
void HttpParser::feed(const std::string &data) { buffer_ += data; }

bool HttpParser::parse(HttpMessage &out) {
    // Extract start of line
    auto end_of_line = buffer_.find("\r\n");

    // find method returns npos if not found
    if (end_of_line == std::string::npos)
        return false; // don't have a complete start-line
    std::string start = buffer_.substr(0, end_of_line);
    buffer_.erase(0, end_of_line + 2);

    // Tokenize the start line
    std::istringstream line(start);
    std::string method_token, uri, version;
    if (!(line >> method_token >> uri >> version)) {
        throw std::runtime_error("Malformed start-line");
    }

    // Populate the enum and URI
    out.method = parse_method(method_token);
    out.request_uri = std::move(uri);

    // reject unsupported methods
    if (out.method == HttpMethod::UNKNOWN) {
        throw std::runtime_error("Unsupported HTTP Method: " + method_token);
    }

    // Parse Headers
    out.headers.clear();
    while (true) {
        // Look for the next "\r\n"
        auto eol = buffer_.find("\r\n");
        if (eol == std::string::npos) {
            return false; // not enough data
        }

        // Empty line => end of headers
        if (eol == 0) {
            // consume the blank line and break
            buffer_.erase(0, 2);
            break;
        }

        // Grab the header line
        std::string line = buffer_.substr(0, eol);
        buffer_.erase(0, eol + 2);

        // Split at the first colon
        auto colon = line.find(":");
        if (colon == std::string::npos) {
            throw std::runtime_error("Malformed header: " + line);
        }
        std::string name = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // trim optional leading space on the value
        if (!value.empty() && value.front() == ' ') {
            value.erase(0, 1);
        }

        out.headers[name] = value;
    }

    // Parse Body (HTTP/1.0)
    out.body.clear();
    auto it = out.headers.find("Content-Length");
    if (it != out.headers.end()) {
        // Explicit length found
        size_t len = std::stoul(it->second);
        if (buffer_.size() < len) {
            // Wait for more data
            return false;
        }

        // Extract exactly len bytes
        out.body = buffer_.substr(0, len);
        buffer_.erase(0, len);
    }

    return true;
}
