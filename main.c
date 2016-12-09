/*

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

Made by BEAN

BLOG : https://beanlog.wordpress.com/2016/12/04/raspberry-pi-3-%e3%81%a7lcd%ef%bc%88acm1602ni-flw-fbw%ef%bc%89/

*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define I2C_FILE "/dev/i2c-1"
#define I2C_ADDRS 0x50
#define LCD_RS_CMD 0x00
#define LCD_RS_DATA 0x80
#define MAX_CHAR 16

/* 基礎関数 */
int setI2c(int *fd);//i2c通信の初期設定
void lcdSet(int fd);//LCDの初期設定
void setCursor(int fd, int n);//カーソル移動
void clearDisplay(int fd);//LCDのディスプレイを綺麗にする
void lcdWrite(int fd, unsigned char rs, unsigned char data);//LCDにデータを書きこむ
void lcdPrint(int fd, char *str, int row, int col);//LCDに文字を表示させる

/* 応用関数 */
void lcdPrint_flow(int fd, char *str, int row, double speed);
void lcdPrint_loop(int fd, char *str, int row, double speed);
void lcdPrint_rowp(int fd, char *str, double speed);

/* その他処理 */
void strReverse(char *sstr, char *str);//文字列反転させる

int main(int argc, char *argv[]) {

  int fd;//ファイルディスクリプタ
  char str[MAX_CHAR];

  if(!setI2c(&fd)) return 1;

  lcdSet(fd);//LCDの初期設定

  //普通に表示してみる
  printf("lcdPrint関数実行中\n普通に表示してみる\n\n");
  lcdPrint(fd, "beanlog : RPi", 0, 0);
  lcdPrint(fd, "by BEAN", 1, 9);
  sleep(3.0);
  clearDisplay(fd);

  //電光掲示板ぽい？？
  printf("lcdPrint_flow関数実行中\n右から左に文字列が流れていきます\n\n");
  lcdPrint_flow(fd, "beanlog", 0, 0.5);//文字列流れるような
  clearDisplay(fd);
  printf("lcdPrint_loop関数実行中\n電車でよく見るやつです。\n\n");
  lcdPrint_loop(fd, "Raspberry Pi", 1, 0.5);//よくある文字列のループ
  clearDisplay(fd);
  printf("lcdPrint_rowp関数実行中\nLCD内を文字列が流れていきます。\n\n");
  lcdPrint_rowp(fd, "DrPepper", 0.5);
  clearDisplay(fd);

  close(fd);
  puts("プログラム終了");
  return 0;
}

int setI2c(int *fd){
  //i2cポートを読み書きでオープン
  if((*fd = open(I2C_FILE, O_RDWR)) < 0){
    fprintf(stderr, "Faild to open i2c port\n");
    return -1;
  }
  //通信先アドレスの設定
  if((ioctl(*fd, I2C_SLAVE, I2C_ADDRS)) < 0){
    fprintf(stderr, "Unable to get bud access to talk to slave\n");
    return -1;
  }
  return 1;
}

void lcdSet(int fd){

  int i;
  char cmds[4] = {0x01, 0x38, 0x0c, 0x06};

  for(i = 0; i < 4; i++){
    lcdWrite(fd, LCD_RS_CMD, cmds[i]);
  }
}

void lcdWrite(int fd, unsigned char rs, unsigned char data) {
  usleep(20);
  unsigned char buf[2];

  buf[0] = rs;
  buf[1] = data;

  if(write(fd, buf, 2) != 2){
    printf("Error writing to i2c slave1\n");
  }
}

void lcdPrint(int fd, char *str, int row, int col){

  int char_cnt = 0;
  int place[2] = {0x00, 0x40};
  if(col > 16) col = 16;

  setCursor(fd, place[row] + col);
  while (char_cnt < MAX_CHAR && str[char_cnt]) {
    lcdWrite(fd, LCD_RS_DATA, str[char_cnt]);
    char_cnt++;
  }
}

void setCursor(int fd, int n){
    lcdWrite(fd, LCD_RS_CMD, LCD_RS_DATA | n);
}

void clearDisplay(int fd){
  lcdWrite(fd, LCD_RS_CMD, 0x01);
}


void lcdPrint_flow(int fd, char *str, int row, double speed){

  int cnt = 1, cursor, i, len = strlen(str);

  while(cnt--){
    cursor = MAX_CHAR;
    while(cursor--){
      lcdPrint(fd, str, row, cursor);
      usleep((int)(speed * 1000000));
      clearDisplay(fd);
    }
    for(i = 1; i < len; i++){
      lcdPrint(fd, &str[i], row, 0);
      usleep((int)(speed * 1000000));
      clearDisplay(fd);
    }
  }
}

void lcdPrint_loop(int fd, char *str, int row, double speed){

  int cnt = 2, cursor, i, len = strlen(str);

  while(cnt--){
    cursor = MAX_CHAR - len;
    while(cursor--){
      lcdPrint(fd, str, row, cursor);
      usleep((int)(speed * 1000000));
      clearDisplay(fd);
    }
    for(i = 1; i < len; i++){
      lcdPrint(fd, &str[i], row, 0);
      lcdPrint(fd, str, row, MAX_CHAR - i);
      usleep((int)(speed * 1000000));
      clearDisplay(fd);
    }
    lcdPrint(fd, str, row, MAX_CHAR - len);
    usleep((int)(speed * 1000000));
    clearDisplay(fd);
  }
}

void lcdPrint_rowp(int fd, char *str, double speed){
  int cnt = 2, cursor, i, len = strlen(str);
  char sstr[MAX_CHAR] = {0};

  strReverse(sstr, str);

  while(cnt--){
    i = 0;
    cursor = 0;
    while(cursor < MAX_CHAR){
      lcdPrint(fd, str, 0, cursor);
      if(cursor > MAX_CHAR - len - 1){
        lcdPrint(fd, sstr, 1, MAX_CHAR - i);
        i++;
      }
      usleep((int)(speed * 1000000));
      clearDisplay(fd);
      cursor++;
    }
    cursor = MAX_CHAR - len;
    i = 1;
    while(cursor + len){
      lcdPrint(fd, sstr, 1, cursor);
      if(cursor < 0){
        lcdPrint(fd, &str[len - i], 0, 0);
        i++;
      }
      usleep((int)(speed * 1000000));
      clearDisplay(fd);
      cursor--;
    }
  }
}

void strReverse(char *sstr, char *str){
  int i, len = strlen(str);

  for(i = 0; i < len; i++){
    sstr[i] = str[len - i - 1];
  }
}
