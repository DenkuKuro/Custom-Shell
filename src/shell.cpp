#include <cerrno>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <pwd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::filesystem::current_path;
using std::filesystem::filesystem_error;
using std::filesystem::path;
#define BUFF_SIZE 4000
#define HISTORY_SIZE 10

void writeError(const string &msg) {
  write(STDERR_FILENO, msg.c_str(), msg.length());
}

vector<string> tokenizer(const string &args) {
  vector<string> tokens;
  string intermediate;
  bool inQuote = false;
  char quoteChar = 0;
  for (int i = 0; i < args.size(); ++i) {
    char c = args[i];
    if (inQuote) {
      if (c == quoteChar)
        inQuote = false;
      else
        intermediate += c;
    } else {
      if (c == '"' || c == '\'') {
        inQuote = true;
        quoteChar = c;
      } else if (isspace(c)) {
        if (!intermediate.empty()) {
          tokens.push_back(intermediate);
          intermediate.clear();
        }
      } else {
        intermediate += c;
      }
    }
  }

  if (!intermediate.empty())
    tokens.push_back(intermediate);
  return tokens;
}

void printCWD() {
  try {
    path curDir = current_path();
    string curDirString = curDir.string();
    write(STDOUT_FILENO, curDirString.c_str(), curDirString.length());
  } catch (const filesystem_error &e) {
    cerr << "Filesystem error" << e.what() << endl;
  }
  write(STDOUT_FILENO, "$ ", 2);
}

void execute(vector<string> cmds) {
  pid_t id = fork();
  int wstatus;
  if (id == 0) {
    vector<char *> argv;
    for (auto &arg : cmds) {
      argv.push_back(arg.data());
    }
    argv.push_back(nullptr);
    if (execvp(argv.front(), argv.data()) == -1) {
      writeError("shell execution error");
      _exit(127);
    }
  } else if (id == -1) {
    writeError("shell fork error");
    return;
  } else {
    if (waitpid(id, &wstatus, 0) == -1) {
      writeError("shell waiting error");
    }
  }
}

static string prevPath = "";
// cd implementation
void changeDirectory(const vector<string> &args) {
  uid_t id = getuid();
  struct passwd *pwuid = getpwuid(id);
  string curPath = current_path();
  if (args.size() > 2)
    writeError("cd: too many arguments provided\n");
  string path = args.back();
  if (args.size() == 1 || path == "~") {
    current_path(std::filesystem::path{pwuid->pw_dir});
  } else if (path[0] == '~') {
    current_path(std::filesystem::path{string{pwuid->pw_dir} + path.substr(1)});
  } else if (path == "-") {
    if (prevPath != "") {
      current_path(std::filesystem::path{prevPath});
    } else
      return;
  } else {
    current_path(std::filesystem::path{path});
  }
  prevPath = curPath;
}

// Implement History (?)
static string history[HISTORY_SIZE];

int main(void) {
  char buf[BUFF_SIZE];
  size_t nbytes = sizeof(buf);
  ssize_t nbytes_read;
  while (true) {
    // Read input
    printCWD();
    while (true) {
      nbytes_read = read(STDIN_FILENO, buf, nbytes);
      if (nbytes_read == -1) {
        if (errno == EINTR)
          continue;
        else
          cerr << "read failed";
      }
      break;
    }
    buf[nbytes_read] = '\0';
    // tokenize input
    vector<string> tokens{tokenizer(buf)};
    if (tokens.empty())
      continue;
    if (tokens.front() == "exit" && tokens.size() == 1) {
      exit(EXIT_SUCCESS);
    } else if (tokens.front() == "exit" && tokens.size() > 1) {
      writeError("exit: too many arguments provided\n");
    } else if (tokens.front() == "cd") {
      changeDirectory(tokens);
    } else {
      execute(tokens);
    }
  }
}
