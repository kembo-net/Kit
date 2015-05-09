/*    kitコマンド
  kit init
    gitディレクトリがあればそこに.kitstackを作成する
    無ければgit initを実行するか聞く
 */
#include <stdio.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
const char KitVersion[] = "Kit 0.0.1";

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
int detect_command(char string[]) {
  const char CommandList[][5] = { "-v", "init" };
  const int CommandNum = sizeof CommandList / sizeof CommandList[0];
  int i;
  for (i=0; i < CommandNum; i++) {
    if (strcmp(string, CommandList[i]) == 0) break;
  }
  if (i == CommandNum) return -1;
  return i;
}

int main(int argc, const char * argv[]) {
  const char GitDir[] = ".git";
  char init_cmd[256] = "git init";
  char pname[248];
  int input_chr;
  int command_id;
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
        //この辺の英文おかしくないだろうか
        //ものすごく不安だ
        printf("Not a git repository.\n");
        printf("Do you run command \"git init\"?(y/n)\n");
        input_chr = getch();
        if (input_chr == 'y' || input_chr == 'Y') {
          printf("Input some options.\n");
          fgets(pname, sizeof(pname), stdin);
          pname[strlen(pname) - 1] = '\0';
          system(strcat(init_cmd, pname));
        }
        else { return 0; }
        //ここから先　今書いてます
      }
      break;
    default:
      printf("unknown command!\n");
      break;
  }
  return 0;
}
