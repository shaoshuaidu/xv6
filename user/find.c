#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

//find文件，需文件名和路径名

//path之下是否有目标文件target
void find(char* path, char* target){
    char buf[512], *p; //定义一个buffer，和指针p
    int fd; //file descriptor
    struct dirent de; //定义一个dirent结构体
    struct stat st; //定义一个stat结构体
    
    //打开目录
    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    //获取目录的stat信息
    //stat参数为路径，fstat参数为文件描述符。成功返回0，失败返回-1
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
        //如果是文件，直接判断是否是目标文件
        case T_FILE:
        if (strcmp(path+strlen(path)-strlen(target), target) == 0){
            //判断路径名的后缀是否是目标文件
            printf("%s\n", path);
        }
        break;
        
        case T_DIR:
        if (strlen(path)+ 1 + DIRSIZ > sizeof(buf)){
            printf("find: path too long\n");
            break;
        }

        strcpy(buf, path);
        p = buf + strlen(buf); //p指向buf的末尾，用于后续字符串拼接
        *p++ = '/';

        //读取目录中的每个文件
        while(read(fd,&de, sizeof(de)) == sizeof(de)){
            //read返回实际读取的字节数
            if(de.inum == 0)
                continue; //如果是空文件，跳过
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            
            if (stat(buf,&st)<0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            
            if((strcmp(buf+strlen(buf)-2, "/.")) != 0 && strcmp(buf+strlen(buf)-3, "/..") != 0){
                //如果不是当前目录和上级目录。又是c指针的高级用法！
                //UNIX中，de.name默认包含/.和/..，所以需要判断
                find(buf, target); //递归调用find函数
            }
        }

        break;
    }
    close(fd); //关闭文件描述符

}



//程序入口
int main(int argc, char *argv[]){ // 参数：find源文件 path filename
    if (argc < 3){
        exit(0);
    }

    char target[512];
    target[0] = '/';
    strcpy(target+1, argv[2]); // target = "/filename"
    find(argv[1], target); // argv[1] = path, argv[2] = filename
    exit(0);
}