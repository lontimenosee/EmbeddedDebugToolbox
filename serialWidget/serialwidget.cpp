#include "serialwidget.h"

#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTextCursor>
#include <QStringConverter>
#include <QVBoxLayout>

namespace {

// 所有显示到界面上的时间戳都统一从这里生成，
// 这样接收区和日志区的格式可以保持一致。
QString timePrefix(bool enabled)
{
    if (!enabled) {
        return QString();
    }

    return QDateTime::currentDateTime().toString("[HH:mm:ss] ");
}

}

SerialWidget::SerialWidget(QWidget *parent)
    : QWidget(parent),
      m_serialPort(new QSerialPort(this))
{
    // 构造顺序也有讲究：
    // 1. 先把界面搭起来
    // 2. 再填充下拉选项
    // 3. 再连接信号槽
    // 4. 最后做一次初始刷新和状态同步
    // 这样可以避免控件还没创建好就被访问。
    buildUi();
    fillStaticOptions();
    buildConnections();
    refreshPorts();
    updateSerialPortUi();
    updateCounters();
}

SerialWidget::~SerialWidget()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

void SerialWidget::buildUi()
{
    // 最外层主布局只负责把“左侧工作区”和“右侧配置区”装进去。
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // 历史和日志被放到标签页里，是为了让左侧主工作区更紧凑。
    // 后续如果还要加“脚本”“收藏命令”之类的辅助页面，也很适合继续放在这里。
    auto *historyTabWidget = new QTabWidget(this);

    auto *historyPage = new QWidget(this);
    auto *historyPageLayout = new QVBoxLayout(historyPage);
    m_historyList = new QListWidget(this);
    m_clearHistoryButton = new QPushButton("Clear History", this);
    historyPageLayout->addWidget(m_historyList);
    historyPageLayout->addWidget(m_clearHistoryButton);

    auto *logsPage = new QWidget(this);
    auto *logsPageLayout = new QVBoxLayout(logsPage);
    m_logTextEdit = new QPlainTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setPlaceholderText("Status and debug logs go here.");
    logsPageLayout->addWidget(m_logTextEdit);

    historyTabWidget->addTab(historyPage, "History");
    historyTabWidget->addTab(logsPage, "Logs");

    // 接收区只显示“真正收到的串口数据”。
    // 我们特意把运行日志拆出去，是为了避免协议调试时看花眼。
    auto *receiveGroupBox = new QGroupBox("Receive Area", this);
    auto *receiveLayout = new QVBoxLayout(receiveGroupBox);
    m_receiveTextEdit = new QPlainTextEdit(this);
    m_receiveTextEdit->setReadOnly(true);
    m_receiveTextEdit->setPlaceholderText("Incoming serial data will be shown here.");
    m_clearReceiveButton = new QPushButton("Clear Receive", this);
    receiveLayout->addWidget(m_receiveTextEdit);
    receiveLayout->addWidget(m_clearReceiveButton);

    // 发送区就是用户真正编辑待发内容的地方。
    // 这里保留多行编辑框，是为了兼顾文本命令、HEX 字符串、批量粘贴等场景。
    auto *sendGroupBox = new QGroupBox("Send Area", this);
    auto *sendLayout = new QVBoxLayout(sendGroupBox);
    m_sendTextEdit = new QPlainTextEdit(this);
    m_sendTextEdit->setPlaceholderText("Type text or hex data here.");
    auto *sendButtonLayout = new QHBoxLayout;
    m_sendButton = new QPushButton("Send", this);
    m_clearSendButton = new QPushButton("Clear Send", this);
    sendButtonLayout->addStretch();
    sendButtonLayout->addWidget(m_sendButton);
    sendButtonLayout->addWidget(m_clearSendButton);
    sendLayout->addWidget(m_sendTextEdit);
    sendLayout->addLayout(sendButtonLayout);

    // 右侧配置区放所有“不会频繁变化但会影响行为”的参数。
    // 这样左侧保持收发主视图，右侧保持配置面板，职责划分更清楚。
    auto *rightWidget = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(12);
    rightLayout->setAlignment(Qt::AlignTop);

    // 串口基础参数区。
    // 这里对应的是串口打开前必须确认的硬件通信参数。
    auto *serialConfigGroupBox = new QGroupBox("Serial Config", this);
    auto *serialConfigLayout = new QFormLayout(serialConfigGroupBox);
    m_portComboBox = new QComboBox(this);
    m_baudRateComboBox = new QComboBox(this);
    m_dataBitsComboBox = new QComboBox(this);
    m_stopBitsComboBox = new QComboBox(this);
    m_parityComboBox = new QComboBox(this);
    m_refreshPortsButton = new QPushButton("Refresh", this);
    m_togglePortButton = new QPushButton("Open Port", this);
    auto *portActionLayout = new QHBoxLayout;
    portActionLayout->addWidget(m_refreshPortsButton);
    portActionLayout->addWidget(m_togglePortButton);
    serialConfigLayout->addRow("Port", m_portComboBox);
    serialConfigLayout->addRow("Baud", m_baudRateComboBox);
    serialConfigLayout->addRow("Data Bits", m_dataBitsComboBox);
    serialConfigLayout->addRow("Stop Bits", m_stopBitsComboBox);
    serialConfigLayout->addRow("Parity", m_parityComboBox);
    serialConfigLayout->addRow("Actions", portActionLayout);

    // 接收配置区主要影响“收到的数据怎样显示”，
    // 而不影响串口底层本身怎么收。
    auto *receiveConfigGroupBox = new QGroupBox("Receive Config", this);
    auto *receiveConfigLayout = new QFormLayout(receiveConfigGroupBox);
    m_receiveModeComboBox = new QComboBox(this);
    m_receiveCodecComboBox = new QComboBox(this);
    m_addTimestampCheckBox = new QCheckBox("Add Timestamp", this);
    receiveConfigLayout->addRow("Mode", m_receiveModeComboBox);
    receiveConfigLayout->addRow("Codec", m_receiveCodecComboBox);
    receiveConfigLayout->addRow("", m_addTimestampCheckBox);

    // 发送配置区主要影响“准备发出去的数据怎样编码和补尾”，
    // 特别是 ASCII / HEX、是否补 CRLF，会直接影响下位机协议解析。
    auto *sendConfigGroupBox = new QGroupBox("Send Config", this);
    auto *sendConfigLayout = new QFormLayout(sendConfigGroupBox);
    m_sendModeComboBox = new QComboBox(this);
    m_sendCodecComboBox = new QComboBox(this);
    m_appendCrlfCheckBox = new QCheckBox("Append CRLF", this);
    m_repeatCountSpinBox = new QSpinBox(this);
    m_repeatCountSpinBox->setRange(1, 100);
    sendConfigLayout->addRow("Mode", m_sendModeComboBox);
    sendConfigLayout->addRow("Codec", m_sendCodecComboBox);
    sendConfigLayout->addRow("Repeat", m_repeatCountSpinBox);
    sendConfigLayout->addRow("", m_appendCrlfCheckBox);

    // 状态区给出最核心的运行信息：
    // 当前端口状态、累计收发字节数、以及发送进度条。
    auto *statusGroupBox = new QGroupBox("Status", this);
    auto *statusLayout = new QVBoxLayout(statusGroupBox);
    m_sendProgressBar = new QProgressBar(this);
    m_sendProgressBar->setRange(0, 100);
    m_statusLabel = new QLabel("Port closed", this);
    m_counterLabel = new QLabel("Tx 0 bytes | Rx 0 bytes", this);
    statusLayout->addWidget(m_sendProgressBar);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_counterLabel);

    rightLayout->addWidget(serialConfigGroupBox);
    rightLayout->addWidget(receiveConfigGroupBox);
    rightLayout->addWidget(sendConfigGroupBox);
    rightLayout->addWidget(statusGroupBox);
    rightLayout->addStretch();

    // 这里用 QSplitter 而不是单纯靠 QVBoxLayout 的原因是：
    // 用户在调试时经常需要“临时放大接收区”或者“压缩日志区”。
    // QSplitter 允许用户直接拖拽分隔条重新分配空间。
    m_leftSplitter = new QSplitter(Qt::Vertical, this);
    m_leftSplitter->addWidget(receiveGroupBox);
    m_leftSplitter->addWidget(sendGroupBox);
    m_leftSplitter->addWidget(historyTabWidget);
    m_leftSplitter->setStretchFactor(0, 3);
    m_leftSplitter->setStretchFactor(1, 2);
    m_leftSplitter->setStretchFactor(2, 2);

    m_rootSplitter = new QSplitter(Qt::Horizontal, this);
    m_rootSplitter->addWidget(m_leftSplitter);
    m_rootSplitter->addWidget(rightWidget);
    m_rootSplitter->setStretchFactor(0, 4);
    m_rootSplitter->setStretchFactor(1, 2);

    mainLayout->addWidget(m_rootSplitter);
}

void SerialWidget::fillStaticOptions()
{
    // 这一段只做“静态选项初始化”，不做真正的串口打开。
    // 这样职责单一，后续你想改默认值时也更容易定位。
    m_baudRateComboBox->addItems(QStringList() << "9600" << "19200" << "38400" << "57600" << "115200");
    m_baudRateComboBox->setCurrentText("115200");

    m_dataBitsComboBox->addItem("5", QSerialPort::Data5);
    m_dataBitsComboBox->addItem("6", QSerialPort::Data6);
    m_dataBitsComboBox->addItem("7", QSerialPort::Data7);
    m_dataBitsComboBox->addItem("8", QSerialPort::Data8);
    m_dataBitsComboBox->setCurrentIndex(3);

    m_stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    m_stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    m_stopBitsComboBox->addItem("2", QSerialPort::TwoStop);

    m_parityComboBox->addItem("None", QSerialPort::NoParity);
    m_parityComboBox->addItem("Odd", QSerialPort::OddParity);
    m_parityComboBox->addItem("Even", QSerialPort::EvenParity);

    m_receiveModeComboBox->addItems(QStringList() << "ASCII" << "HEX");
    m_sendModeComboBox->addItems(QStringList() << "ASCII" << "HEX");
    m_receiveCodecComboBox->addItems(QStringList() << "UTF-8" << "GBK");
    m_sendCodecComboBox->addItems(QStringList() << "UTF-8" << "GBK");

    m_addTimestampCheckBox->setChecked(true);
    m_appendCrlfCheckBox->setChecked(true);
    m_repeatCountSpinBox->setValue(1);
}

void SerialWidget::buildConnections()
{
    // 这里集中处理所有信号槽连接。
    // 这样构造函数里不会堆满 connect，后续你找交互逻辑也更方便。
    connect(m_refreshPortsButton, &QPushButton::clicked, this, &SerialWidget::refreshPorts);
    connect(m_togglePortButton, &QPushButton::clicked, this, &SerialWidget::toggleSerialPort);
    connect(m_sendButton, &QPushButton::clicked, this, &SerialWidget::sendPayload);
    connect(m_clearReceiveButton, &QPushButton::clicked, this, &SerialWidget::clearReceive);
    connect(m_clearSendButton, &QPushButton::clicked, this, &SerialWidget::clearSend);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, &SerialWidget::clearHistory);
    connect(m_historyList, &QListWidget::itemDoubleClicked, this, [this]() { loadSelectedHistoryItem(); });
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialWidget::handleSerialDataReady);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialWidget::handleSerialError);
}

void SerialWidget::refreshPorts()
{
    // 每次刷新前先清空旧列表，避免重复追加相同串口项。
    m_portComboBox->clear();

    // QSerialPortInfo::availablePorts() 会枚举当前系统可见的串口设备。
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        // 如果有设备描述，就把描述也显示出来，方便区分多个串口。
        const QString displayText = info.description().isEmpty()
                ? info.portName()
                : QString("%1 (%2)").arg(info.portName(), info.description());
        m_portComboBox->addItem(displayText, info.portName());
    }

    appendStatusLog(QString("Found %1 port(s)").arg(ports.size()));
}

void SerialWidget::toggleSerialPort()
{
    // 这个槽函数同时承担“打开”和“关闭”两个动作，
    // 依据当前串口是否已打开来决定接下来做什么。
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        appendStatusLog("Serial port closed");
        updateSerialPortUi();
        return;
    }

    if (m_portComboBox->currentIndex() < 0) {
        QMessageBox::warning(this, "Notice", "Please select a serial port first.");
        return;
    }

    // 串口打开前，必须先把参数完整设置到 QSerialPort 对象上。
    m_serialPort->setPortName(m_portComboBox->currentData().toString());
    m_serialPort->setBaudRate(m_baudRateComboBox->currentText().toInt());
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_dataBitsComboBox->currentData().toInt()));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_stopBitsComboBox->currentData().toInt()));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_parityComboBox->currentData().toInt()));

    // 真正打开底层串口设备。
    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "Open Failed", m_serialPort->errorString());
        appendStatusLog(QString("Open failed: %1").arg(m_serialPort->errorString()));
        updateSerialPortUi();
        return;
    }

    appendStatusLog(QString("Opened %1 @ %2").arg(m_serialPort->portName(), m_baudRateComboBox->currentText()));
    updateSerialPortUi();
}

void SerialWidget::sendPayload()
{
    // 没打开串口时直接阻止发送，避免产生“以为发出去了”的错觉。
    if (!m_serialPort->isOpen()) {
        QMessageBox::warning(this, "Notice", "Please open the serial port first.");
        return;
    }

    // 发送前先把界面文本转换成真正的待发字节流。
    // 这里可能失败，例如 HEX 模式下输入了非法字符。
    bool ok = false;
    QString errorMessage;
    const QByteArray payload = buildOutgoingPayload(&ok, &errorMessage);
    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", errorMessage);
        return;
    }

    if (payload.isEmpty()) {
        QMessageBox::information(this, "Notice", "There is no data to send.");
        return;
    }

    // 支持重复发送，方便做简单压力测试或多次命令触发。
    const int repeatCount = m_repeatCountSpinBox->value();
    qint64 writtenBytes = 0;
    for (int i = 0; i < repeatCount; ++i) {
        writtenBytes += m_serialPort->write(payload);
    }

    // 发送历史里保留的是十六进制可视化结果，
    // 这样即使是不可见字符或二进制字节，也能回看。
    m_totalTransmittedBytes += writtenBytes;
    m_sendProgressBar->setValue(100);
    m_historyList->addItem(
        timePrefix(m_addTimestampCheckBox->isChecked()) + QString::fromLatin1(payload.toHex(' ')));
    appendStatusLog(QString("Sent %1 time(s), %2 byte(s) each").arg(repeatCount).arg(payload.size()));
    updateCounters();
}

void SerialWidget::clearReceive()
{
    m_receiveTextEdit->clear();
    appendStatusLog("Receive area cleared");
}

void SerialWidget::clearSend()
{
    m_sendTextEdit->clear();
}

void SerialWidget::clearHistory()
{
    m_historyList->clear();
}

void SerialWidget::loadSelectedHistoryItem()
{
    if (!m_historyList->currentItem()) {
        return;
    }

    // 双击历史项后，把它重新塞回发送框，便于二次发送。
    m_sendTextEdit->setPlainText(m_historyList->currentItem()->text());
    appendStatusLog("History item loaded into send area");
}

void SerialWidget::handleSerialDataReady()
{
    // readyRead 是事件驱动模型：
    // 不是我们轮询串口，而是底层一有数据就通知我们来读。
    const QByteArray data = m_serialPort->readAll();
    m_totalReceivedBytes += data.size();
    appendReceivedText(decodeIncomingPayload(data));
    updateCounters();
}

void SerialWidget::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    // 这里先记录日志，后续如果你要做更细的恢复策略，
    // 也可以按错误类型分别处理。
    appendStatusLog(QString("Serial error: %1").arg(m_serialPort->errorString()));
    updateSerialPortUi();
}

void SerialWidget::updateSerialPortUi()
{
    // 界面状态必须和串口真实状态同步：
    // 打开后禁止改通信参数，关闭后才允许改。
    const bool isOpen = m_serialPort->isOpen();
    m_togglePortButton->setText(isOpen ? "Close Port" : "Open Port");
    m_sendButton->setEnabled(isOpen);
    m_portComboBox->setEnabled(!isOpen);
    m_baudRateComboBox->setEnabled(!isOpen);
    m_dataBitsComboBox->setEnabled(!isOpen);
    m_stopBitsComboBox->setEnabled(!isOpen);
    m_parityComboBox->setEnabled(!isOpen);
    m_statusLabel->setText(isOpen
                           ? QString("Port opened: %1").arg(m_serialPort->portName())
                           : "Port closed");
}

void SerialWidget::updateCounters()
{
    m_counterLabel->setText(
        QString("Tx %1 bytes | Rx %2 bytes").arg(m_totalTransmittedBytes).arg(m_totalReceivedBytes));
}

void SerialWidget::appendReceivedText(const QString &text)
{
    // 接收区只放真正收到的数据，不混入状态日志。
    m_receiveTextEdit->appendPlainText(timePrefix(m_addTimestampCheckBox->isChecked()) + text);
    m_receiveTextEdit->moveCursor(QTextCursor::End);
}

void SerialWidget::appendStatusLog(const QString &text)
{
    // 日志区专门记录程序运行过程中的说明、错误和动作结果。
    m_logTextEdit->appendPlainText(timePrefix(m_addTimestampCheckBox->isChecked()) + text);
    m_logTextEdit->moveCursor(QTextCursor::End);
}

QByteArray SerialWidget::buildOutgoingPayload(bool *ok, QString *errorMessage) const
{
    if (ok) {
        *ok = false;
    }

    // 先从发送编辑框里拿到原始文本。
    QString text = m_sendTextEdit->toPlainText();
    if (m_appendCrlfCheckBox->isChecked()) {
        // 对很多文本协议来说，CRLF 是一条命令的结束标志。
        text.append("\r\n");
    }

    if (m_sendModeComboBox->currentText() == "HEX") {
        // HEX 模式下，界面里输入的是“字符形式的十六进制文本”，
        // 例如 "40 4C 45 44 5F 4F 4E"，最终要转成真实字节。
        if (!isValidHexText(text, errorMessage)) {
            return QByteArray();
        }

        QByteArray hexText = text.toLatin1();
        hexText = hexText.simplified();
        hexText.replace(' ', "");
        if (ok) {
            *ok = true;
        }
        return QByteArray::fromHex(hexText);
    }

    if (ok) {
        *ok = true;
    }

    // 文本模式下，根据用户选择的编码把 QString 转成字节流。
    if (m_sendCodecComboBox->currentText() == "GBK") {
        QStringEncoder encoder(QStringEncoder::System);
        return encoder.encode(text);
    }

    return text.toUtf8();
}

QString SerialWidget::decodeIncomingPayload(const QByteArray &data) const
{
    // 接收显示模式只影响“怎么看”，不影响底层真实收到的字节。
    if (m_receiveModeComboBox->currentText() == "HEX") {
        return QString::fromLatin1(data.toHex(' ')).toUpper();
    }

    if (m_receiveCodecComboBox->currentText() == "GBK") {
        QStringDecoder decoder(QStringDecoder::System);
        return decoder.decode(data);
    }

    return QString::fromUtf8(data);
}

bool SerialWidget::isValidHexText(const QString &text, QString *errorMessage) const
{
    // 先去掉常见空白字符，让用户可以按 "AA BB CC" 的形式输入。
    QString compact = text;
    compact.remove(' ');
    compact.remove('\r');
    compact.remove('\n');
    compact.remove('\t');

    if (compact.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "HEX mode requires at least one hex byte.";
        }
        return false;
    }

    // 十六进制必须两字符表示一个字节，所以长度必须为偶数。
    if (compact.size() % 2 != 0) {
        if (errorMessage) {
            *errorMessage = "HEX text must contain an even number of characters.";
        }
        return false;
    }

    // 只允许 0-9 A-F a-f。
    for (const QChar ch : compact) {
        if (!ch.isDigit() && (ch.toUpper() < QChar('A') || ch.toUpper() > QChar('F'))) {
            if (errorMessage) {
                *errorMessage = QString("Invalid HEX character: %1").arg(ch);
            }
            return false;
        }
    }

    return true;
}
