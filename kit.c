/*    kitコマンド
  kit init
    gitディレクトリがあればそこに.kitstackを作成する
    無ければgit initを実行するか聞く
 */
#include <stdio.h>
const char KitVersion[] = "Kit 0.0.1";

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
      printf("init\n");
      break;
    default:
      printf("unknown command!\n");
      break;
  }
  return 0;
}
