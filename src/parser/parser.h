#pragma once
#include <string>
#include <unordered_map>

enum class HttpMethod { GET, POST, HEAD, UNKNOWN };
enum class HttpParserMode { Request, Response };

struct HttpMessage {
    HttpMethod method;
    std::string start_line;
    std::string request_uri;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpParser {
  public:
    // default request type parser
    explicit HttpParser(HttpParserMode mode = HttpParserMode::Request);
    void feed(const std::string &data);
    bool parse(HttpMessage &out);

  private:
    enum class ParsePhase { StartLine, Headers, Body };
    std::string buffer_;
    HttpParserMode mode_;
    HttpMethod parse_method(const std::string &token) const;

    ParsePhase phase_ = ParsePhase::StartLine;
    std::string current_start_line_;
    std::unordered_map<std::string, std::string> current_headers_;
    size_t body_length_ = 0;
    HttpMessage in_progress_;
};
