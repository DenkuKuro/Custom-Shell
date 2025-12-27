#ifndef GEMINI_CMD
#define GEMINI_CMD

#include <curl/curl.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <string>

size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp);

std::string callGeminiAPI(const std::string &apiKey, const std::string &prompt);

std::vector<std::string> tokenizeCommands(std::string &cmds);

std::string parseResponse(std::string response);

#endif
