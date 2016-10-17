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
#define LCD_RS_DATE 0x80
#define MAX_CHAR 16
#define CHAR_TABLE "char_table.txt"
#define MAX_WORDCNT 256
#define BLANK 0xfe

/* プロトタイプ宣言 */
//変更されたか
void lcd_write(unsigned char rs, unsigned char data, int fd);
void lcd_set(int fd);
void remove_newline(char *str);
void lcd_print(int fd, char *str);

int main(int argc, char  *argv[]) {

  int fd;//ファイルディスクプリタ
  char str[MAX_CHAR];
  //i2cポートを読み書きでオープン
  if((fd = open(I2C_FILE, O_RDWR)) < 0){
    fprintf(stderr, "Faild to open i2c port\n");
    return 1;
  }
  //通信先アドレスの設定
  if((ioctl(fd, I2C_SLAVE, I2C_ADDRS)) < 0){
    fprintf(stderr, "Unable to get bud access to talk to slave\n");
    return 1;
  }

  usleep(100*1000);//0.02[sec]

  lcd_set(fd);//LCDの初期設定

  if(argc > 1){
    strcpy(str, argv[1]);
    lcd_print(fd, str);
  }else{
    printf(">> "); fgets(str, MAX_CHAR, stdin);
    remove_newline(str);
    lcd_print(fd, str);
  }

  return 0;
}

void lcd_set(int fd){

  int i;
  char cmds[4] = {0x01, 0x38, 0x0c, 0x06};

  for(i = 0; i < 4; i++){
    lcd_write(LCD_RS_CMD, cmds[i], fd);
  }
}

void remove_newline(char *str){
  int i = 0;
  while (str[i]) {
    if(str[i] == '\n'){
      str[i] = 0;
    }
    i++;
  }
}

void lcd_write(unsigned char rs, unsigned char data, int fd) {
  usleep(2000);
  unsigned char buf[2];

  buf[0] = rs;
  buf[1] = data;

  if(write(fd, buf, 2) != 2){
    printf("Error writing to i2c slave1\n");
  }
}

void lcd_print(int fd, char *str){

  int char_cnt = 0;

  while (char_cnt < MAX_CHAR && str[char_cnt]) {
    lcd_write(LCD_RS_DATE, str[char_cnt], fd);
    char_cnt++;
  }
}
