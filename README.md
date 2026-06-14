# Embedded Debug Toolbox

[中文](#中文) | [English](#english)

## 中文

嵌入式调试助手，一个基于 Qt Widgets 的串口调试练习项目。

### 目前功能
- 串口扫描、打开、关闭
- 文本/HEX 发送
- 接收区独立显示
- 日志区独立显示
- 历史发送记录
- 发送/接收计数与状态显示
- `QSplitter` 可拖拽缩放布局

### 使用方法
1. 选择串口号和波特率。
2. 设置数据位、停止位、校验位。
3. 点击打开串口。
4. 在发送区输入内容后发送。
5. 在接收区查看数据，在日志区查看运行状态。

### 构建说明
1. 使用 Qt Creator 打开 `serialAssistant.pro`
2. 切换到 `Release`
3. 编译运行或配合 `windeployqt` 打包

## English

An embedded debugging assistant based on Qt Widgets.

### Features
- Serial port scan, open, and close
- Text / HEX sending
- Separate receive area
- Separate log area
- Send history
- Tx / Rx counters and status display
- Resizable layout with `QSplitter`

### How to Use
1. Select a port and baud rate.
2. Set data bits, stop bits, and parity.
3. Open the serial port.
4. Type data in the send area and send it.
5. Read data in the receive area and status in the log area.

### Build
1. Open `serialAssistant.pro` in Qt Creator
2. Switch to `Release`
3. Build/run, or package with `windeployqt`
