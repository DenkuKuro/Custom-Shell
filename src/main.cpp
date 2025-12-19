#include <cerrno>
#include <csignal>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <pwd.h>
#include <sstream>
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
  stringstream ss{args};
  string intermediate;

  while (ss >> intermediate) {
    tokens.push_back(intermediate);
  }

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
    if (tokens.front() == "exit")
      exit(EXIT_SUCCESS);
    execute(tokens);
  }
}
