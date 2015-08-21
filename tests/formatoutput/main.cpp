/**
 * @file main.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jolla.com>
 * @date 2015
 */

#include <QTextStream>
#include <QRegExp>
#include <getopt.h>

void usage(){
  QTextStream out(stderr);
  out << "Parse QTest output on STDIN and make it shiny" << endl
      << endl
      << "Usage: formatoutput [options]" << endl
      << endl
      << "\t--skip-pass <yes|no>   \tSkip PASS lines [yes]" << endl
      << "\t--skip-debug <yes|no>  \tSkip QDEBUG lines [yes]" << endl
      << "\t--skip-warn <yes|no>   \tSkip QWARN lines [no]" << endl
      << "\t--skip-config <yes|no> \tSkip Config: lines [yes]" << endl
      << endl
      << "\t--help                 \tThis help text" << endl
      << endl;
}

bool isTrue(char *argument){
  if (!strcasecmp(argument, "yes") ||
      !strcasecmp(argument, "true") ||
      !strcmp(argument, "1"))
    return true;
  else return false;
}

int main(int argc, char **argv){
  QTextStream in(stdin);
  QTextStream out(stdout);
  int c, option_index;

  static struct option long_options[] = {
    {"skip-pass",   required_argument, 0, 0 },
    {"skip-debug",  required_argument, 0, 0 },
    {"skip-warn",   required_argument, 0, 0 },
    {"skip-config", required_argument, 0, 0 },
    {"help",        no_argument,       0, 0 }
  };

  struct {
      bool skip_pass   = true;
      bool skip_debug  = true;
      bool skip_warn   = false;
      bool skip_config = true;
  } options;

  while ((c=getopt_long_only(argc, argv, "",
                             long_options, &option_index)) != EOF){
    switch(c){
      case 0:
        if (!strcmp(long_options[option_index].name, "help")){
          usage();
          exit(0);
        } else if (!strcmp(long_options[option_index].name, "skip-pass")){
          options.skip_pass = isTrue(optarg);
        } else if (!strcmp(long_options[option_index].name, "skip-debug")){
          options.skip_debug = isTrue(optarg);
        } else if (!strcmp(long_options[option_index].name, "skip-warn")){
          options.skip_warn = isTrue(optarg);
        } else if (!strcmp(long_options[option_index].name, "skip-config")){
          options.skip_config = isTrue(optarg);
        }
        break;
      default:
        usage();
        exit(-1);
    }
  }

  while (!in.atEnd()){
    QString line = in.readLine();

    if (line.startsWith("PASS") && options.skip_pass) continue;
    if (line.startsWith("QDEBUG") && options.skip_debug) continue;
    if (line.startsWith("QWARN") && options.skip_warn) continue;
    if (line.startsWith("Config:") && options.skip_config) continue;

    line.replace(QRegExp("^(\\*{3}.*\\*{3})"), "\033[0;36m\\1\033[0;0m");
    line.replace(QRegExp("^(QDEBUG .*)"), "\033[0;90m\\1\033[0;0m");
    line.replace(QRegExp("^(QWARN .*)"), "\033[0;34m\\1\033[0;0m");
    line.replace(QRegExp("^FAIL!"), "\033[0;31mFAIL!\033[0;0m");
    line.replace(QRegExp("^PASS"), "\033[0;32mPASS\033[0;0m");
    line.replace(QRegExp("^Totals: (\\d{1,} passed), (\\d{1,} failed), (\\d{1,} skipped)"),
                 "Totals: \033[0;32m\\1\033[0;0m, \033[0;31m\\2\033[0;0m, \033[0;33m\\3\033[0;0m");
    out << line << endl;
  }

}
