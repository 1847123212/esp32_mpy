# 安装编译指南 #

## 1. ESP-IDF 依赖环境 ##
   当前验证过的环境：

   `cmake: 3.16.9`

   `python: 3.6.9`

## 2. 克隆工程 ##

   因为本工程代码，使用submodule来管理esp-idf和micropython的官方代码，因此clone 本工程时，请采用以下步骤:

   `git clone https://github.com/YanMinge/esp32_mpy`

   `cd esp32_mpy/`

   `git submodule update --init`

   如果是首次克隆，您还需要更新 esp-idf 的 submodule

   进入目录 `esp32_mpy/submodule/esp-idf`，输入命令
   `git submodule update --init`

   同样的，更新 micropython的 submodule
  
   进入目录 `esp32_mpy/submodule/micropython`，输入命令
   `git submodule update --init`
   
## 3. ESP-IDF 和 micropython 工具链安装 ##

   进入目录 `esp32_mpy/submodule/esp-idf`，输入命令

   `./install.sh`

   进入目录 `esp32_mpy/submodule/micropython`，输入命令
   
   `make -C mpy-cross`
## 4. ESP-IDF 和 micropython 对应的版本 ##
    
    `ESP-IDF: commit-id 1d7068e4be430edd92bb63f2d922036dcf5c3cc1  2021.06.04`

    `micropython: commit-id da8aad18a4508476aa041406c0b06f5df4a8e59b 2021.06.11`

    `esp32-camera: commit-id 3159d9d9fa6b431a3aab3e0f8c8741785ebcf425 2021.06.14`

    `lvgl_esp32_drivers: commit-id cccb932d3c8b0035632bfff866d242308d0ce167 2021.06.08`

    `lvgl: commit-id 7c1eb0064535f2d914b9dc885ebb2a2d0d73381d 2021.06.18`

## 5. 编译工程 ##

   进入目录 `esp32_mpy/submodule/esp-idf`，输入命令 `source ./export.sh` , 这个操作是初始化全部的环境变量


   现在您可以从 `esp32_mpy/project/aitest` 启动编译您的固件
   
   首先进入 `/project/aitest` 文件目录

   - 执行 `make` 编译代码(首次编译，需要先执行make)
   
   如果编译没有出错，您在目录 `project/aitest/` 中应该可以看到新创建的 `build-GENERIC_S3`文件夹。在这个文件夹中，有编译生成的 `application.bin` 等文件。
   
   其它可能经常会用到指令包括

   - 执行 `make erase` 擦除模块的flash
   
   - 执行 `make deploy` 编译并烧录固件

## 6. 串口工具 ##
   因为本工程固件支持repl模式，所以可以使用 pytty等串口工具来进行repl调试。
   [点击下载putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html)

## 7. 连接你的设备 ##



# 开机使用 #

## 1. putty正确配置串口 ##


## 2. 使用 REPL测试结果 ##
 
# 文件结构说明 #

## 1. 主要文件结构 ##

- `doc` 文档资料
- `driver` 硬件驱动，主要是串口驱动程序
- `hardware` 开源主控的硬件设计资料
- `project` 开源主控的应用代码
- `submodule` 第三方库的内容，包括esp-idf和micropython
- `tools` 以供使用的一些工具的源码
- `List of FAQ` 常见问题的答疑


# 开发计划 #

- 支持micropython 官方的 network，usocket，urequest，uart，webrepl.