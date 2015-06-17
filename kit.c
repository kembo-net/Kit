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
const char KitVersion[] = "Kit 0.11.1 beta";
const char GitDir[]     = ".git";
const char KitFile[]    = ".kitstack";
const char GitCmd[]    = "git ";
const char AddCmd[] = "git add ";
const char InitCmd[] = "git init ";
const char CommitCmd[] = "git commit -m ";
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
//与えられた文字列が整数値だったら0を返す
int is_num(const char string[]) {
  int i;
  i = 0;
  while ((string[i] != '\0') && (string[i] != '\n')) {
    if ((string[i] < '0') || ('9' < string[i])) { return -1; }
    i++;
  }
  return 0;
}
//文字列を渡すと該当するコマンドナンバーを返す
//該当するコマンドが無い場合は-1を返す
int detect_command(const char string[]) {
  const char CommandList[][8] = {
    "-v", "init", "do", "done",
    "now", "list", "remove", "edit"
  };
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
    const int argc,
    char * const argv[], 
    const int starting_point,
    char result[]) {
  int i;
  strcpy(result, "");
  if (starting_point < argc) {
    for (i = starting_point; i < argc; i++) {
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
  }
  fclose(fp);
  return i;
}
//配列の中身を指定されたindexまでkitファイルに詰め込んで保存する(空行はスキップ)
void save_kit_file(int index, char inputs[][128]) {
  char cmd_str[128];
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
  else { 
    printf("kit file open error.\n");
    exit(1);
  }
  strcpy(cmd_str, AddCmd);
  strcat(cmd_str, KitFile);
  system(cmd_str);
}
//引数の一覧を文字列にして詰め込む
void gen_arg_str(int argc, char * const argv[], int start, char result[]) {
  int i;
  strcpy(result, "");
  if (argc > start) {
    //コマンド以降のオプションを文字列に格納
    for (i = start; i < argc; i++) {
      strcat(result, " ");
      //ハイフン付のオプションか数字以外はダブルクォーテーションで囲む
      if ((argv[i][0] == '-') || (is_num(argv[i]) == 0)) {
        strcat(result, argv[i]);
      }
      else {
        strcat(result, "\"");
        strcat(result, argv[i]);
        strcat(result, "\"");
      }
    }
  }
}
//ダブルクォーテーションをエスケープする
void escape_dq(char str[]) {
  int i, cnt, len;
  len = strlen(str);
  cnt = 0;
  for (i = 0; i < len; i++) {
    if (str[i] == '"') { cnt++; }
  }
  if (cnt > 0) {
    for (i = len; i >= 0; i--) {
      str[i + cnt] = str[i];
      if (str[i] == '"') {
        cnt--;
        str[i + cnt] = '\\';
        if (cnt == 0) { break; }
      }
    }
  }
}
//kitの更新記録をコミットする
void commit_kit(const char message[]) {
  char cmd_str[128];
  strcpy(cmd_str, AddCmd);
  strcat(cmd_str, KitFile);
  system(cmd_str);
  strcpy(cmd_str, CommitCmd);
  strcat(cmd_str, "\"");
  strcat(cmd_str, message);
  strcat(cmd_str, "\"");
  system(cmd_str);
}

void cmd_init(int argc, char * const argv[]){
  char cmd_str[256], dir_str[128];
  FILE *fp;
  //Gitディレクトリが見つからなかったらgit initする
  if (!opendir(GitDir)) {
    strcpy(cmd_str, InitCmd);
    gen_arg_str(argc, argv, 2, dir_str);
    strcat(cmd_str, dir_str);
    system(cmd_str);
    search_no_hyphen(argc, argv, 2, dir_str);
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
void cmd_do(int argc, char * const argv[]) {
  const char AfterCmd[] = "after";
  const char ManyCmd[] = "many";
  const char EndMany[] = "EOF";
  const char BaseMes[] = "[kit]add plan ";
  const char ManyMes[] = "[kit]add many plans";
  char buffer[128][128] = { {0} };
  char opts[128][128];
  char mes[128];
  int i, num, pointer, n_opt = 0, many_flg = 0, opts_len = 1;
  //-nオプションの検出
  if (getopt(argc, argv, "n") != -1) {
    n_opt = 1;
  }
  //ファイルを開いて全部読む
  num = read_kit_file(127, buffer);
  //オプションの検証
  pointer = 0;
  while (optind < argc) {
    if (strcmp(argv[optind], AfterCmd) == 0) {
      if (pointer == 0) {
        pointer = num;
      }
    }
    else if (strcmp(argv[optind], ManyCmd) == 0) {
      many_flg = 1;
    }
    else if (is_num(argv[optind]) == 0) {
      pointer = atoi(argv[optind]);
    }
    else { break; }
    optind++;
  }
  if (many_flg) {
    printf("Input commit messages and press Enter. Finally input \"EOF\".\n");
    opts_len = 0;
    while (1) {
      printf(">");
      scanf("%s", mes);
      if (strlen(mes) > 0) {
        if (strcmp(mes, EndMany) == 0) {
          break;
        }
        else {
          if ((mes[0] != '"') || (mes[strlen(mes) - 1] != '"')) {
            strcpy(opts[opts_len], "\"");
            strcat(opts[opts_len], mes);
            strcat(opts[opts_len], "\"");
          }
          else {
            strcpy(opts[opts_len], mes);
          }
          opts_len++;
        }
      }
    }
    if (opts_len == 0) { exit(1); }
  }
  else { gen_arg_str(argc, argv, optind, opts[0]); }
  //指定行に挿入
  for (i = num + opts_len - 1; i > pointer; i--) {
    strcpy(buffer[i], buffer[i - opts_len]);
  }
  for (i = 0; i < opts_len; i++) {
    strcpy(buffer[i + pointer], opts[i]);
  }
  //ファイルを一旦リセットして全部書き込む
  save_kit_file(num + opts_len, buffer);
  //kitファイルの変更をコミット
  if (n_opt == 0) {
    if (opts_len == 1) {
      strcpy(mes, BaseMes);
      escape_dq(opts[0]);
      strcat(mes, opts[0]);
    }
    else {
      strcpy(mes, ManyMes);
    }
    commit_kit(mes);
  }
}
void cmd_done(int argc, char * const argv[]) {
  char buffer[128][128] = { {0} };
  char cmd_str[256];
  int i, num, pointer;
  //引数で指定のない場合には先頭の予定を実行する
  if ((optind < argc) && (is_num(argv[optind]) == 0)) {
    pointer = atoi(argv[optind]);
    optind++;
  }
  else { pointer = 0; }
  //git addする
  if (optind < argc) {
    do {
      strcpy(cmd_str, AddCmd);
      strcat(cmd_str, argv[optind]);
      system(cmd_str);
      optind++;
    } while(optind < argc);
  }
  else {
    strcpy(cmd_str, AddCmd);
    strcat(cmd_str, "-A");
    system(cmd_str);
  }
  //ファイルを開いて全部読む
  num = read_kit_file(128, buffer);
  if (num <= pointer) {
    printf("Not exist\n");
    exit(1);
  }
  //コマンドを作る
  strcpy(cmd_str, CommitCmd);
  strcat(cmd_str, buffer[pointer]);
  if (*last_char(cmd_str) == '\n') {
    *last_char(cmd_str) = '\0';
  }
  //ファイルを一旦リセットして指定の行以外を全部書き込む
  buffer[pointer][0] = '\0';
  save_kit_file(num, buffer);
  //コマンド実行
  system(cmd_str);
}
void cmd_now(int argc, char * const argv[]) {
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
void cmd_list() {
  char buffer[128];
  int i;
  FILE *fp;
  fp = fopen(KitFile, "r");
  if (fp == NULL) {
    printf("%s is not found.\n", KitFile);
    exit(1);
  }
  i = 0;
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    printf("%d %s", i, buffer);
    if (*last_char(buffer) != '\n') { printf(br); }
    i++;
  }
  fclose(fp);
}
void cmd_remove(int argc, char * const argv[]) {
  const char BaseMes[] = "[kit]remove plan ";
  const char BaseMesMany[] = "[kit]remove plans";
  const char BaseMesAll[] = "[kit]remove plans all";
  const char AllCmd[] = "all";
  char buffer[128][128] = { {0} };
  char mes[128];
  int i, num, targets[64], targ_len = 0, n_opt = 0, all_flg = 0;
  //-nオプションの検出
  if (getopt(argc, argv, "n") != -1) {
    n_opt = 1;
  }
  //引数で指定のない場合には先頭の予定を削除する
  if (optind < argc) {
    if (strcmp(argv[optind], AllCmd) == 0) {
      all_flg = 1;
    }
    else {
      while (optind < argc) {
        if (is_num(argv[optind]) == 0) {
          targets[targ_len] = atoi(argv[optind]);
          targ_len++;
        }
        optind++;
      }
    }
  }
  if (targ_len == 0) {
    targets[0] = 0;
    targ_len = 1;
  }
  //ファイルを開いて全部読む
  num = read_kit_file(128, buffer);
  //削除する予定の内容をコミットメッセージ用に保存
  if (n_opt == 0) {
    if (all_flg) {
      strcpy(mes, BaseMesAll);
    }
    else if (targ_len > 1) {
      strcpy(mes, BaseMesMany);
    }
    else {
      strcpy(mes, BaseMes);
      escape_dq(buffer[targets[0]]);
      strcat(mes, buffer[targets[0]]);
    }
  }
  //ファイルを一旦リセットして指定の行以外を全部書き込む
  if (all_flg) {
    save_kit_file(0, buffer);
  }
  else {
    for(i = 0; i < targ_len; i++) {
      if (targets[i] >= num) {
        printf("Not exist plan %d\n", targets[i]);
        exit(1);
      }
      buffer[targets[i]][0] = '\0';
    }
    save_kit_file(num, buffer);
  }
  //kitファイルの変更をコミット
  if (n_opt == 0) { commit_kit(mes); }
}
void cmd_edit(int argc, char * const argv[]) {
  char buffer[128][128] = { {0} };
  char mes[128] = "[kit]edit plan to ";
  char opt[128];
  int i, num, pointer, n_opt = 0;
  //-nオプションの検出
  if (getopt(argc, argv, "n") != -1) {
    n_opt = 1;
  }
  //ファイルを開いて全部読む
  num = read_kit_file(127, buffer);
  //番号検出
  if ((optind < argc) && (is_num(argv[optind]) ==0)) {
    pointer = atoi(argv[optind]);
    optind++;
  }
  else {
    pointer = 0;
  }
  //指定行を編集
  gen_arg_str(argc, argv, optind, opt);
  strcpy(buffer[pointer], opt);
  //ファイルを一旦リセットして全部書き込む
  save_kit_file(num, buffer);
  //kitファイルの変更をコミット
  if (n_opt == 0) {
    escape_dq(opt);
    strcat(mes, opt);
    commit_kit(mes);
  }
}
//kitに無いコマンドはgitにそのまま回す
void cmd_git(int argc, char * const argv[]) {
  char cmd_str[128], opts[128];
  strcpy(cmd_str, GitCmd);
  gen_arg_str(argc, argv, 1, opts);
  strcat(cmd_str, opts);
  system(cmd_str);
}

int main(int argc, char * const argv[]) {
  char str[128];
  int i, command_id;
  //コマンド判定
  //コマンドが何も付いていない場合はバージョン表示
  if (argc > 1) {
    command_id = detect_command(argv[1]);
  }
  else {
    command_id = 0;
  }
  optind++;
  switch(command_id) {
    case 0://Put version
      printf("%s\n", KitVersion);
      break;
    case 1://Run init
      cmd_init(argc, argv);
      break;
    case 2://Run do
      cmd_do(argc, argv);
      break;
    case 3://Run done
      cmd_done(argc, argv);
      break;
    case 4://Run now
      cmd_now(argc, argv);
      break;
    case 5://Run list
      cmd_list();
      break;
    case 6://Run remove
      cmd_remove(argc, argv);
      break;
    case 7://Run edit
      cmd_edit(argc, argv);
      break;
    default:
      cmd_git(argc, argv);
      break;
  }
  return 0;
}

