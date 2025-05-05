#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* //返回值是一个字符指针
fmtname(char *path)
{
  static char buf[DIRSIZ+1]; //buf是一个字符数组，存储路径名
  //DIRSIZ是目录项的大小，+1是为了存储字符串结束符\0
  //static表示buf是一个静态变量，生命周期是整个程序运行期间
  char *p; //指针变量，所指内存属于外部，所以不要static

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++; //找到文件的base name，p指向最后一个斜杠后的字符

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ) //fmtname是独立函数，因此必须判断
    return p;
  memmove(buf, p, strlen(p)); //复制p到buf中，p指向buf的末尾
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p)); //在buf的末尾添加空格，填充到DIRSIZ大小
  return buf; //这也是指针吗
}

void
ls(char *path) //void表示没有返回值，参数是一个字符串指针，指向路径
{
  char buf[512], *p; //buf是一个字符数组，存储路径名；p是一个字符指针
  int fd; //fd是一个整数，表示文件描述符
  //定义两个结构体变量
  struct dirent de; //目录项结构体，包含文件名和文件号
  struct stat st; //文件状态结构体

  if((fd = open(path, 0)) < 0){ //open系统调用，打开文件，0表示只读模式；
  // fd是文件描述符，小于0表示打开失败；先赋值给fd，如果小于0，表示打开失败
    fprintf(2, "ls: cannot open %s\n", path); //2表示标准错误输出，
    return; //直接返回，表示函数执行失败
  }

  if(fstat(fd, &st) < 0){ //fstat系统调用，获取文件状态，fd是文件描述符，st是文件状态结构体
    //如果小于0，表示获取失败；传递st的地址，获取文件状态信息；因为c是传值调用，所以需要传递地址
    fprintf(2, "ls: cannot stat %s\n", path); 
    close(fd); //关闭文件描述符，可以继续使用变量，但不能用其值
    return;
  }

  switch(st.type){
  case T_FILE: //文件类型
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR: //目录类型
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    } //如果路径名太长，超过了buf的大小，打印错误信息
    strcpy(buf, path); //复制路径名到buf中
    p = buf+strlen(buf); //p指向buf的末尾
    *p++ = '/'; //在buf的末尾添加一个斜杠， 相当于*p='/'，p++；
    while(read(fd, &de, sizeof(de)) == sizeof(de)){ //每次读取一个dir entry
      //即子目录
      if(de.inum == 0) //表示无效目录（可能被删除了），continue跳过
        continue;
      memmove(p, de.name, DIRSIZ); //将de.name复制到buf中，p指向buf的末尾,
      // DIRSIZ是目录项的大小
      p[DIRSIZ] = 0; //在buf的末尾添加一个空字符\0，表示字符串结束。*(p + DIRSIZ) = 0;
      if(stat(buf, &st) < 0){ //获取子文件的状态信息
        printf("ls: cannot stat %s\n", buf);
        continue;
      } //如果获取失败，打印错误信息，继续下一个子文件
      //打印子文件的状态信息，fmtname函数返回子文件的名称，st.type是文件类型，st.ino是文件号，st.size是文件大小
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int //返回的参数是int
main(int argc, char *argv[]) // 入口函数，argc是参数个数，包括程序名，argv是参数数组
{
  int i;

  if(argc < 2){ //没有参数
    ls(".");
    exit(0); //退出程序，表示正常退出，相当于return 0
  }
  for(i=1; i<argc; i++) //从1开始，argv[0]是程序名
    ls(argv[i]); //分别调用ls函数处理每个参数
  exit(0);
}
