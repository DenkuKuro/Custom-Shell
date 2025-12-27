#include <curl/curl.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <string>

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
                      "gemini-2.5-flash:generateContent?key=" +
                      apiKey;

    // Build JSON request body
    Json::Value root;
    Json::Value content;
    Json::Value parts;
    Json::Value textPart;

    textPart["text"] = prompt;
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

int main() {
  // Replace with your actual API key
  std::string apiKey = std::getenv("GEMINI_API_KEY");
  std::string prompt = R"(
    You are a Linux command-line assistant. Generate ONLY the exact shell commands needed, without explanations unless asked.

User request: Create 3 files that start with 'test'

Response format:
- Provide only the executable commands
- Provide series of commands in a single line
- No markdown formatting
- No explanations

Command(s):
  )";

  std::cout << "Sending request to Gemini API..." << std::endl;

  std::string response = callGeminiAPI(apiKey, prompt);

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
      std::cout << "\nGemini Response:\n" << text << std::endl;
    } else {
      std::cout << "Raw response:\n" << response << std::endl;
    }
  } else {
    std::cerr << "Failed to parse JSON: " << errs << std::endl;
    std::cout << "Raw response:\n" << response << std::endl;
  }

  std::vector<std::string> tokens = tokenizeCommands(text);
  for (std::string token : tokens)
    std::cout << token << " ";
  std::cout << std::endl;
  return 0;
}

// To compile on Linux/Mac:
// g++ gemini_api.cpp -lcurl -ljsoncpp -o gemini_api
//
// To compile on Windows (with vcpkg):
// Install dependencies: vcpkg install curl jsoncpp
// Then compile with appropriate linking flags
