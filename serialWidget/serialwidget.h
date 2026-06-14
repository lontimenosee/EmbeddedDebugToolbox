#ifndef SERIALWIDGET_H
#define SERIALWIDGET_H

#include <QByteArray>
#include <QWidget>
#include <QtSerialPort/QSerialPort>

class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QSplitter;

class SerialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialWidget(QWidget *parent = nullptr);
    ~SerialWidget() override;

private slots:
    void refreshPorts();
    void toggleSerialPort();
    void sendPayload();
    void clearReceive();
    void clearSend();
    void clearHistory();
    void loadSelectedHistoryItem();
    void handleSerialDataReady();
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    void buildUi();
    void buildConnections();
    void fillStaticOptions();
    void updateSerialPortUi();
    void updateCounters();
    void appendReceivedText(const QString &text);
    void appendStatusLog(const QString &text);
    QByteArray buildOutgoingPayload(bool *ok, QString *errorMessage) const;
    QString decodeIncomingPayload(const QByteArray &data) const;
    bool isValidHexText(const QString &text, QString *errorMessage) const;

    QSerialPort *m_serialPort = nullptr;

    QPlainTextEdit *m_receiveTextEdit = nullptr;
    QPushButton *m_clearReceiveButton = nullptr;

    QPlainTextEdit *m_sendTextEdit = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_clearSendButton = nullptr;

    QComboBox *m_portComboBox = nullptr;
    QComboBox *m_baudRateComboBox = nullptr;
    QComboBox *m_dataBitsComboBox = nullptr;
    QComboBox *m_stopBitsComboBox = nullptr;
    QComboBox *m_parityComboBox = nullptr;
    QPushButton *m_refreshPortsButton = nullptr;
    QPushButton *m_togglePortButton = nullptr;

    QComboBox *m_receiveModeComboBox = nullptr;
    QComboBox *m_receiveCodecComboBox = nullptr;
    QCheckBox *m_addTimestampCheckBox = nullptr;

    QComboBox *m_sendModeComboBox = nullptr;
    QComboBox *m_sendCodecComboBox = nullptr;
    QCheckBox *m_appendCrlfCheckBox = nullptr;
    QSpinBox *m_repeatCountSpinBox = nullptr;

    QListWidget *m_historyList = nullptr;
    QPushButton *m_clearHistoryButton = nullptr;
    QProgressBar *m_sendProgressBar = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_counterLabel = nullptr;
    QPlainTextEdit *m_logTextEdit = nullptr;
    QSplitter *m_rootSplitter = nullptr;
    QSplitter *m_leftSplitter = nullptr;

    qint64 m_totalTransmittedBytes = 0;
    qint64 m_totalReceivedBytes = 0;
};

#endif // SERIALWIDGET_H
