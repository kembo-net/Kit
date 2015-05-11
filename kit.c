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
const char br[] = "\n";

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
//最後の文字へのポインタを返す
char *last_char(char string[]) {
  return string + (strlen(string) - 1);
}
//文字列を渡すと該当するコマンドナンバーを返す
//該当するコマンドが無い場合は-1を返す
int detect_command(const char string[]) {
  const char CommandList[][8] = { "-v", "init", "do", "done", "now" };
  const int CommandNum = sizeof CommandList / sizeof CommandList[0];
  int i;
  for (i=0; i < CommandNum; i++) {
    if (strcmp(string, CommandList[i]) == 0) break;
  }
  if (i == CommandNum) return -1;
  return i;
}
//オプション列からディレクトリ名(ハイフンついてない項目)を探して返す
void search_no_hyphen(
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
}
//kitファイルの内容を一行ずつ配列に詰め込んで行数と共に返す
int read_kit_file(int length, char result[][128]) {
  int i;
  char *ret;
  FILE *fp;
  fp = fopen(KitFile, "r");
  if (fp == NULL) {
    printf("%s is not found.\n", KitFile);
    exit(1);
  }
  for (i = 0; i < length; i++) {
    if (fgets(result[i], sizeof(result[i]), fp) == NULL) {
      break;
    }
    printf("%s", result[i]);
  }
  fclose(fp);
  return i;
}
//配列の中身を指定されたindexまでkitファイルに詰め込んで保存する(空行はスキップ)
void save_kit_file(int index, char inputs[][128]) {
  int i;
  FILE *fp;
  fp = fopen(KitFile, "w");
  if (fp != NULL) {
    for (i=0; i < index; i++) {
      if ((inputs[i][0] != '\0') && (inputs[i][0] != '\n')) {
        fputs(inputs[i], fp);
        if (*last_char(inputs[i]) != '\n') {
          fputs(br, fp);
        }
      }
    }
    fclose(fp);
  }
}

void cmd_init(int argc, const char * argv[], char opts[]){
  const char InitCmd[] = "git init";
  char cmd_str[256], dir_str[128];
  FILE *fp;
  //Gitディレクトリが見つからなかったらgit initする
  if (!opendir(GitDir)) {
    strcpy(cmd_str, InitCmd);
    strcat(cmd_str, opts);
    system(cmd_str);
    search_no_hyphen(argv, 2, argc, dir_str);
    if (dir_str[0] == '\0') { strcpy(dir_str, "."); }
    strcat(dir_str, "/");
  }
  else { strcpy(dir_str , "./"); }
  //kitファイルの作成
  strcat(dir_str, KitFile);
  fp = fopen(dir_str, "r");
  if (fp == NULL) { fp = fopen(dir_str, "w"); }
  fclose(fp);
}
void cmd_do(char opts[]) {
  char buffer[128][128] = { {0} };
  int num;
  strcpy(buffer[0], opts);
  //ファイルを開いて全部読む
  num = read_kit_file(127, buffer + 1);
  //ファイルを一旦リセットして先頭に新しいコメントを追加して全部書き込む
  save_kit_file(num + 1, buffer);
}
void cmd_done(int argc, const char *argv[]) {
  const char CommitCmd[] = "git commit -am ";
  char buffer[128][128] = { {0} };
  char cmd_str[256];
  int i, num, pointer;
  //引数で指定のない場合には先頭の予定を実行する
  if (argc > 2){
    pointer = atoi(argv[2]);
  }
  else { pointer = 0; }
  //ファイルを開いて全部読む
  num = read_kit_file(128, buffer);
  if (num <= pointer) {
    printf("Not exist\n");
    exit(1);
  }
  //コマンドを作る
  strcpy(cmd_str, CommitCmd);
  strcat(cmd_str, "\"");
  strcat(cmd_str, buffer[pointer]);
  if (*last_char(cmd_str) == '\n') {
    *last_char(cmd_str) = '"';
  }
  else {
    strcat(cmd_str, "\"");
  }
  //ファイルを一旦リセットして指定の行以外を全部書き込む
  buffer[pointer][0] = '\0';
  save_kit_file(num, buffer);
  //コマンド実行
  system(cmd_str);
}
void cmd_now(int argc, const char *argv[]) {
  char buffer[128];
  FILE *fp;
  //ファイルを開いて一行目だけ読んで表示
  fp = fopen(KitFile, "r");
  if (fp == NULL) {
    printf("%s is not found.\n", KitFile);
    exit(1);
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    if (strlen(buffer) > 0) {
      printf(buffer);
      if (*last_char(buffer) != '\n') { printf(br); }
    }
  }
  fclose(fp);
}

int main(int argc, const char * argv[]) {
  char opts[253] = "";
  int i, command_id;
  if (argc > 1) {
    command_id = detect_command(argv[1]);
    if (argc > 2) {
      for (i = 2; i < argc; i++) {
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
    case 2://Run do
      search_no_hyphen(argv, 2, argc, opts);
      cmd_do(opts);
      break;
    case 3://Run done
      cmd_done(argc, argv);
      break;
    case 4://Run now
      cmd_now(argc, argv);
      break;
    default:
      printf("unknown command!\n");
      break;
  }
  return 0;
}

