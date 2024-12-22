#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QTcpSocket>
#include <QHostAddress>
#include <QInputDialog>
#include <QFileDialog>
#include <QProcess>

class ChatWindow : public QWidget {
public:
    ChatWindow(QWidget *parent = nullptr) : QWidget(parent), socket(new QTcpSocket(this)) {
        auto *layout = new QVBoxLayout(this);

        display = new QTextEdit(this);
        display->setReadOnly(true);
        layout->addWidget(display);

        input = new QLineEdit(this);
        layout->addWidget(input);

        auto *sendButton = new QPushButton("Send", this);
        layout->addWidget(sendButton);

        auto *uploadButton = new QPushButton("Upload File", this);
        layout->addWidget(uploadButton);

        auto *downloadButton = new QPushButton("Download File", this);
        layout->addWidget(downloadButton);

        connect(sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);
        connect(uploadButton, &QPushButton::clicked, this, &ChatWindow::uploadFile);
        connect(downloadButton, &QPushButton::clicked, this, &ChatWindow::downloadFile);

        // 서버에 연결
        connectToServer();
    }

private:
    QTextEdit *display;
    QLineEdit *input;
    QTcpSocket *socket;

    void connectToServer() {
        QString serverAddress = "127.0.0.1"; // 서버 IP 주소
        quint16 port = 8080;                // 서버 포트 번호

        // 사용자명 입력
        QString username = QInputDialog::getText(this, "Username", "Enter your username:");
        if (username.isEmpty()) {
            display->append("Username cannot be empty. Exiting...");
            close();
            return;
        }

        // 서버에 연결
        socket->connectToHost(QHostAddress(serverAddress), port);
        if (!socket->waitForConnected(3000)) {
            display->append("Failed to connect to server.");
            return;
        }

        display->append("Connected to server.");

        // 사용자명 전송
        socket->write(username.toUtf8());
        socket->flush();

        // 서버에서 수신한 데이터 처리
        connect(socket, &QTcpSocket::readyRead, this, &ChatWindow::receiveMessage);
    }

    void sendMessage() {
        QString message = input->text();
        if (!message.isEmpty()) {
            QByteArray data = (message + "\n").toUtf8();
            socket->write(data);
            socket->flush();
            display->append("Me: " + message);
            input->clear();
        }
    }

    void receiveMessage() {
        while (socket->canReadLine()) {
            QString message = socket->readLine().trimmed();
            display->append(message);
        }
    }

    void uploadFile() {
        QString filePath = QFileDialog::getOpenFileName(this, "Select File to Upload");
        if (filePath.isEmpty()) {
            display->append("No file selected for upload.");
            return;
        }

        // Upload script 실행
        QProcess process;
        process.start("bash", QStringList() << "upload.sh" << filePath);
        process.waitForFinished();

        QString output = process.readAllStandardOutput();
        display->append(output.trimmed());
    }

    void downloadFile() {
        QString fileName = QInputDialog::getText(this, "Download File", "Enter file name to download:");
        if (fileName.isEmpty()) {
            display->append("No file name provided for download.");
            return;
        }

        QString savePath = QFileDialog::getSaveFileName(this, "Save Downloaded File");
        if (savePath.isEmpty()) {
            display->append("No path provided to save the downloaded file.");
            return;
        }

        // Download script 실행
        QProcess process;
        process.start("bash", QStringList() << "download.sh" << fileName << savePath);
        process.waitForFinished();

        QString output = process.readAllStandardOutput();
        display->append(output.trimmed());
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ChatWindow window;
    window.setWindowTitle("Chat Client");
    window.resize(400, 300);
    window.show();
    return app.exec();
}
