#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void run(char* program, char** args){

    if (fork() == 0){
        exec(program, args);
        exit(0);
    }
    return;
}


int main(int argc, char *argv[]){ //argv是指针变量char**，指向指针的指针
    char buf[2048]; //字符数组
    char* p =buf, * last_p =buf; //指针p和指针last_p，指向buf的首地址
    char* argsbuf[128]; //类型为指针的数组，存放地址
    char** args =argsbuf; //指向字符串指针的指针

    for (int i = 1; i < argc; i++){
        *args = argv[i]; //等价于*(argv+1)
        args++;//指向下一个地址
    }

    char** pa = args;//记住当前参数的位置

    while (read(0, p, 1) != 0){ //从文件描述符0（标准输入）中读取一个字符，存到p制定的位置，返回值为0时，表示读取结束
        if (*p == ' ' || *p == '\n'){ //如果读取到空格或换行符
            char temp = *p;
            *p = '\0'; //将当前字符置为'\0'，表示字符串结束
            *(pa++) = last_p; 
            last_p = p+1;

            if (temp == '\n'){
                *pa = 0; //补上null
                run(argv[1], argsbuf); //argv[1]是程序名
                pa = args;
            }
        }
        p++; //指针p向后移动一个字符
    }

    if (pa != args){
        *p = '\0';
        *(pa++) = last_p;
        *pa = 0;
        run(argv[1],argsbuf);
    }

    while (wait(0) != -1){};
    exit(0);
}