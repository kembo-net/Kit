/*    kitコマンド
  kit init
    gitディレクトリがあればそこに.kitstackを作成する
    無ければgit initを実行するか聞く
 */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
const char KitVersion[] = "Kit 0.0.2";

//一文字だけ入力を受け取る
int getch( ) {
  struct termios oldt, newt;
  int ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}
//文字列を渡すと該当するコマンドナンバーを返す
//該当するコマンドが無い場合は-1を返す
int detect_command(const char string[]) {
  const char CommandList[][5] = { "-v", "init" };
  const int CommandNum = sizeof CommandList / sizeof CommandList[0];
  int i;
  for (i=0; i < CommandNum; i++) {
    if (strcmp(string, CommandList[i]) == 0) break;
  }
  if (i == CommandNum) return -1;
  return i;
}
void search_dir_opt(
    const char *argv[], 
    const int starting_point,
    const int ending_point,
    char result[]) {
  int i;
  strcpy(result, "");
  if (starting_point < ending_point) {
    for (i = starting_point; i < ending_point; i++) {
      if (argv[i][0] != '-' && argv[i][0] != '\0') {
        strcpy(result, argv[i]);
        break;
      }
    }
  }
  if (result[0] == '\0') { result = "."; }
  strcat(result, "/");
}

int main(int argc, const char * argv[]) {
  const char GitDir[]  = ".git";
  const char KitFile[] = ".kitstack";
  char git_cmd[256] = "git";
  char opts[253], dir_str[128];
  int i, input_chr, command_id;
  FILE *fp;
  if (argc > 1) {
    command_id = detect_command(argv[1]);
  }
  else {
    command_id = 0;
  }
  switch(command_id) {
    case 0://Put version
      printf("%s\n", KitVersion);
      break;
    case 1://Run init
      if (!opendir(GitDir)) {
        opts[0] = '\0';
        for (i = 1; i < argc; i++) {
          strcat(opts, " ");
          strcat(opts, argv[i]);
        }
        strcat(git_cmd, opts);
        system(git_cmd);
        search_dir_opt(argv, 2, argc, dir_str);
      }
      else { strcpy(dir_str , "./"); }
      strcat(dir_str, KitFile);
      fp = fopen(dir_str, "r");
      if (fp == NULL) { fp = fopen(dir_str, "w"); }
      fclose(fp);
      break;
    default:
      printf("unknown command!\n");
      break;
  }
  return 0;
}

