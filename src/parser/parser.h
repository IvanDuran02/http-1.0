#pragma once
#include <string>
#include <unordered_map>

enum class HttpMethod { GET, POST, HEAD, UNKNOWN };

struct HttpMessage {
    HttpMethod method;
    std::string start_line;
    std::string request_uri;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpParser {
  public:
    void feed(const std::string &data);
    bool parse(HttpMessage &out);

  private:
    std::string buffer_;

    HttpMethod parse_method(const std::string &token) const;
};
