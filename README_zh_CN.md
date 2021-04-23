# 安装编译指南 #

## 1. ESP-IDF 工具链 ##

   如果您希望可以修改和定制自己私有的固件，以此增加底层python API的支持，您可能需要此操作。

   ESP-IDF工具链是由ESP32芯片官方的开发框架，它的安装使用可以参考官方说明
   [设置工具链](https://docs.espressif.com/projects/esp-idf/en/latest/get-started-cmake/index.html#step-1-set-up-the-toolchain "设置工具链")

   如果你是使用的windows系统，并且使用的是官方集成的[ESP-IDF工具链安装包](https://dl.espressif.com/dl/esp-idf-tools-setup-1.2.exe)，你可以忽略官方的其它步骤，如果你是使用的其它系统，那么你在安装ESP-IDF工具链的时候，你还应当注意官方说明的另外几个步骤 

   [步骤4.安装必需的Python包](https://docs.espressif.com/projects/esp-idf/en/latest/get-started-cmake/index.html#step-4-install-the-required-python-packages)

## 2. 串口工具 ##
   因为本工程固件支持repl模式，所以可以使用 pytty等串口工具来进行repl调试。
   [点击下载putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html)


## 3. 克隆工程 ##

   因为本工程代码，使用submodule来管理esp-idf和micropython的官方代码，因此clone 本工程时，请采用 `git clone --recursive https://github.com/Makeblock-official/micropython-board`，这样才可以同时更新 [esp-idf](https://github.com/espressif/esp-idf) 和 [micropython](https://github.com/micropython/micropython)

   如果是首次克隆，您还需要更新 esp-idf 的 submodule

   进入目录 `micropython-board/submodule/esp-idf`，输入命令
   `git submodule update --init`

   同样的，更新 micropython的 submodule
  
   进入目录 `micropython-board/submodule/micropython`，输入命令
   `git submodule update --init`
   

## 4. 编译工程 ##

   现在您已经准备好了makeblock的micropython主控的应用程序，同时您也已经安装了可用的ESP32开发工具链，您可以从 `project/halocode` 启动编译您的固件
   
   首先进入 `project/halocode` 文件目录

   - 执行 `make` 编译代码(首次编译，需要先执行make)
   
   如果编译没有出错，您在目录 `project/halocode/` 中应该可以看到新创建的 `build`文件夹。在这个文件夹中，有编译生成的 `application.bin` 等文件。
   
   其它可能经常会用到指令包括

   - 执行 `make erase` 擦除模块的flash
   
   - 执行 `make deploy` 编译并烧录固件

## 5. 连接你的设备 ##

   现在将makeblock micropython主控板(例如 halocode)连接到计算机，并检查主机上是否有可见的串行端口。
 
   串行端口的名称具有以下模式

   - Windows：名称如 `COM1`
   - Linux：以 `/dev/tty` 为前缀的设备
   - macOS：以 `/dev/cu` 为前缀的设备

## 6. 配置你的工程 ##

   如果你需要将新生成的固件烧录进micropython主板，你可以修改 `project/halocode/Makefile` 文件。
   通常您需要修改的是 `PORT，BAUD` 的设置。
 
    PORT ?= COM4
    FLASH_MODE ?= dio
    BAUD ?= 921600
    BUILD ?= build
    CONFIG_SPIRAM_ENABLE ?= 1

## 7. 烧录固件 ##

    如果您是第一次烧录此固件，建议您先对整个固件进行擦除处理后再烧录新的固件，否则可能会发生系统分区异常等问题。

   如果你已经正确配置了你的工程，你可以使用 `make erase` 擦除模块的原有程序
   然后使用 `make deploy` 命令编译和烧录新的固件。

   您也可以在终端命令中指定步骤6配置的参数。例如在windows系统中，如果设备的串口号是 COM5.可以使用 `make deploy PORT=COM5` 命令编译和烧录固件。

# 开机使用 #

## 1. putty正确配置串口 ##

  打开 putty软件，连接类型选择 Serial，按照你实际的端口号配置端口，波特率设置为 115200

![](https://github.com/Makeblock-official/micropython-board/blob/master/image/putty%20serial%20configure.png)

## 2. 使用 REPL测试结果 ##
  打开端口，输入回车键，应该可以看到 REPL的提示符，以光环板为示例，此时依次输入指令

  `import halo`

 ` halo.led.show_all(255,0,0)`

  ![](https://github.com/Makeblock-official/micropython-board/blob/master/image/sample%20code.png)

  此时应该可以看到，光环板的灯盘全部被点亮为红色。
 
# 文件结构说明 #

## 1. 主要文件结构 ##

- `adapter` 硬件适配层
- `driver` 硬件驱动，主要是串口驱动程序
- `hardware` 开源主控的硬件设计资料
- `project` 开源主控的应用代码
- `submodule` 第三方库的内容，包括esp-idf和micropython
- `tools` 以供使用的一些工具的源码
- `List of FAQ` 常见问题的答疑


# 开发计划 #

- 支持micropython 官方的 network，usocket，urequest，uart，webrepl.
- 提供光环板的硬件驱动支持
- 更新到最新的 micropython和esp-idf v3.3版本
- 增加 python案例，主要是光环板硬件相关的案例
- 增加 wifi的web server和TCP UDP接口
- 增加 蓝牙的 python接口支持
- 增加 百度、微软语音识别的支持
- 增加 mesh组网的案例支持
- 增加 对外设模块驱动的支持
