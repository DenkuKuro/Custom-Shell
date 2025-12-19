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

using namespace std;
#define BUFF_SIZE 4000

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
      } else if (c == ' ' || c == '\n') {
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
    filesystem::path curDir = filesystem::current_path();
    string curDirString = curDir.string();
    write(STDOUT_FILENO, curDirString.c_str(), curDirString.length());
  } catch (const filesystem::filesystem_error &e) {
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
      exit(EXIT_FAILURE);
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

// cd implementation
void changeDirectory(const string &path) {
  uid_t id = getuid();
  struct passwd *pwuid = getpwuid(id);
  if (path == "~") {
    current_path(filesystem::path{pwuid->pw_dir});
  } else if (path[0] == '~') {
    current_path(filesystem::path{pwuid->pw_dir});
    current_path(filesystem::path{string{pwuid->pw_dir} + path.substr(1)});
  } else {
    current_path(filesystem::path{path});
  }
}

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
    if (tokens.front() == "exit" && tokens.size() == 1) {
      exit(EXIT_SUCCESS);
    } else if (tokens.front() == "exit" && tokens.size() > 1) {
      writeError("exit: too many arguments provided\n");
    } else if (tokens.front() == "cd" && tokens.size() == 2) {
      changeDirectory(tokens.back());
    } else if (tokens.front() == "cd" && tokens.size() > 2) {
      writeError("cd: too many arguments provided\n");
    } else {
      execute(tokens);
    }
  }
}
