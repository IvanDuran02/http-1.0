#include "parser.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

HttpMethod HttpParser::parse_method(const std::string &token) const {
    if (token == "GET") return HttpMethod::GET;
    if (token == "POST") return HttpMethod::POST;
    if (token == "HEAD") return HttpMethod::HEAD;
    return HttpMethod::UNKNOWN;
}

HttpParser::HttpParser(HttpParserMode mode) : mode_(mode) {}

void HttpParser::feed(const std::string &data) {
    buffer_ += data;
}

bool HttpParser::parse(HttpMessage &out) {
    if (phase_ == ParsePhase::StartLine) {
        auto end_of_line = buffer_.find("\r\n");
        if (end_of_line == std::string::npos) return false;

        current_start_line_ = buffer_.substr(0, end_of_line);
        buffer_.erase(0, end_of_line + 2);
        phase_ = ParsePhase::Headers;
    }

    if (phase_ == ParsePhase::Headers) {
        while (true) {
            auto eol = buffer_.find("\r\n");
            if (eol == std::string::npos) return false;

            if (eol == 0) {
                buffer_.erase(0, 2);
                phase_ = ParsePhase::Body;
                break;
            }

            std::string line = buffer_.substr(0, eol);
            buffer_.erase(0, eol + 2);

            auto colon = line.find(":");
            if (colon == std::string::npos) throw std::runtime_error("Malformed header: " + line);

            std::string name = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            if (!value.empty() && value.front() == ' ') value.erase(0, 1);
            current_headers_[name] = value;
        }

        in_progress_.headers = current_headers_;
        in_progress_.start_line = current_start_line_;

        if (mode_ == HttpParserMode::Request) {
            std::istringstream line(current_start_line_);
            std::string method_token, uri, version;
            if (!(line >> method_token >> uri >> version)) throw std::runtime_error("Malformed request line");
            in_progress_.method = parse_method(method_token);
            if (in_progress_.method == HttpMethod::UNKNOWN) {
                throw std::runtime_error("Unsupported HTTP Method: " + method_token);
            }            
            in_progress_.request_uri = uri;
        } else {
            std::istringstream line(current_start_line_);
            std::string version, status, reason;
            if (!(line >> version >> status >> reason)) throw std::runtime_error("Malformed status line");
            in_progress_.headers["Status"] = status;
            in_progress_.headers["Reason"] = reason;
        }

        auto it = in_progress_.headers.find("Content-Length");
        body_length_ = (it != in_progress_.headers.end()) ? std::stoul(it->second) : 0;
    }

    if (phase_ == ParsePhase::Body) {
        if (buffer_.size() < body_length_) return false;
        in_progress_.body = buffer_.substr(0, body_length_);
        buffer_.erase(0, body_length_);

        out = in_progress_; // copy to output

        // reset state for next message
        phase_ = ParsePhase::StartLine;
        current_start_line_.clear();
        current_headers_.clear();
        in_progress_ = HttpMessage();
        body_length_ = 0;

        return true;
    }

    return false;
}