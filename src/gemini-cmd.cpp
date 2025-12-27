#include "../include/gemini-cmd.h"
#include <curl/curl.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <string>

const std::string contextPrompt = R"(
  You are an expert Linux command-line assistant. Your job is to translate natural language requests into precise, executable shell commands.
  
  Rules:
  1. Output ONLY the command(s) needed - no markdown, no code blocks, no backticks
  2. If multiple commands are needed, separate them with newlines
  3. Use safe, standard Linux commands
  4. Assume the user is in a bash shell
  5. If the request is ambiguous, choose the most common interpretation
  6. If user request is unrelated to Linux commands provide this message: "Unrelated prompt given..."
  
  User Request: 
)";

// Callback function to handle response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  userp->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string callGeminiAPI(const std::string &apiKey,
                          const std::string &prompt) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    // Construct the URL with your API key
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/"
                      "gemini-3-flash-preview:generateContent?key=" +
                      apiKey;

    // Build JSON request body
    Json::Value root;
    Json::Value content;
    Json::Value parts;
    Json::Value textPart;

    textPart["text"] = contextPrompt + prompt;
    parts.append(textPart);
    content["parts"] = parts;

    Json::Value contents;
    contents.append(content);
    root["contents"] = contents;

    Json::StreamWriterBuilder writer;
    std::string jsonData = Json::writeString(writer, root);

    // Set up curl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

    // Set headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // Perform the request
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                << std::endl;
    }

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return readBuffer;
}

std::vector<std::string> tokenizeCommands(std::string &cmds) {
  std::vector<std::string> tokens;
  std::string token;
  std::stringstream ss{cmds};

  while (ss >> token)
    tokens.push_back(token);

  return tokens;
}

std::string parseResponse(std::string response) {
  // Parse and display the response
  Json::CharReaderBuilder reader;
  Json::Value jsonResponse;
  std::string errs;
  std::istringstream s(response);
  std::string text;
  if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
    // Extract the text from the response
    if (jsonResponse.isMember("candidates") &&
        jsonResponse["candidates"].size() > 0) {
      text = jsonResponse["candidates"][0]["content"]["parts"][0]["text"]
                 .asString();
    }
  } else {
    // Handle the error better
    std::cerr << "Failed to parse JSON: " << errs << std::endl;
    std::cout << "Raw response:\n" << response << std::endl;
  }
  return text;
}

// To compile on Linux/Mac:
// g++ gemini_api.cpp -lcurl -ljsoncpp -o gemini_api
//
// To compile on Windows (with vcpkg):
// Install dependencies: vcpkg install curl jsoncpp
// Then compile with appropriate linking flags
