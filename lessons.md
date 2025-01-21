# Lessons
### Shell in the xv6 systeml
- `sh.c`
  - `gettoken(char **ps, char *es, char **q, char **eq)` 
    - 作用：在字符串指针 *ps -> *es（不包括 *es）之间的字符串中，得到最先一个token，并且修改 **ps 为新的非whitespace的 *ps位置（为第二次调用这个方法做准备），\*\*q为token的起始位置，\*\*eq为终止位置
    - 返回值 表示得到的token类型 (identifier为'a')
  - `main(void)`
    - 主函数
    - 两种运行：cd 或者 普通指令
- `init.c`
  - 先用`mknod`创建一个console文件，并且fd = 0，用于存储标准输入
  - `dup(int fd)` 的作用是复制fd的内容给一个新的file descriptor（依次增大）
    - 通过这个，stdout和stderr的内容都会写到console里面了
    - dup的功能是不同的fd在同一文件内的光标的偏移量是相同的
  - `fork`一个console主要的进程，不同于init进程
    - 通过判断console进程是否退出，决定是否要重启console进程
    - console里面输入`Ctrl-D`（相当于EOF），就会退出

### 