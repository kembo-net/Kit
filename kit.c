/*    kitコマンド
  kit init
    gitディレクトリがあればそこに.kitstackを作成する
    無ければgit initを実行するか聞く
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
const char KitVersion[] = "Kit 0.0.2";
const char GitDir[]     = ".git";
const char KitFile[]    = ".kitstack";
const char GitCmd[]    = "git";

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
  const char CommandList[][8] = { "-v", "init", "commit" };
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

void cmd_init(int argc, const char * argv[], char opts[]){
  char cmd_str[256], dir_str[128];
  FILE *fp;
  //Gitディレクトリが見つからなかったらgit initする
  if (!opendir(GitDir)) {
    strcpy(cmd_str, GitCmd);
    strcat(cmd_str, opts);
    system(cmd_str);
    search_dir_opt(argv, 2, argc, dir_str);
  }
  else { strcpy(dir_str , "./"); }
  //kitファイルの作成
  strcat(dir_str, KitFile);
  fp = fopen(dir_str, "r");
  if (fp == NULL) { fp = fopen(dir_str, "w"); }
  fclose(fp);
}
void cmd_commit(char opts[]) {
  const char br[] = "\n";
  char buffer[128][128] = { {0} };
  int i, j;
  FILE *fp;
  //ファイルを開いて全部読む
  fp = fopen(KitFile, "r");
  if (fp == NULL) {
    printf("%s is not found.", KitFile);
    exit(1);
  }
  else {
    i = 0;
    while((fgets(buffer[i], 128, fp) != NULL) || (i < 127)) { i++; }
    fclose(fp);
  }
  //ファイルを一旦リセットして先頭に新しいコマンドを追加して全部書き込む
  fp = fopen(KitFile, "w");
  if (fp != NULL) {
    fputs(opts, fp);
    for (j = 0; ((buffer[j][0] != 0) && (j < i)); j++) {
      fputs(br, fp);
      fputs(buffer[j], fp);
    }
    fclose(fp);
  }
}

int main(int argc, const char * argv[]) {
  char opts[253] = "";
  int i, command_id;
  if (argc > 1) {
    command_id = detect_command(argv[1]);
    if (argc > 2) {
      for (i = 1; i < argc; i++) {
        strcat(opts, " ");
        strcat(opts, argv[i]);
      }
    }
  }
  else {
    command_id = 0;
  }
  switch(command_id) {
    case 0://Put version
      printf("%s\n", KitVersion);
      break;
    case 1://Run init
      cmd_init(argc, argv, opts);
      break;
    case 2://Run commit
      cmd_commit(opts);
      break;
    default:
      printf("unknown command!\n");
      break;
  }
  return 0;
}

