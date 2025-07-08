#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registerdialog.h"
#include "logger.h"
#include "loginwindow.h"
#include "facelogindialog.h"
#include "thememanager.h"
#include "updateprofiledialog.h"
#include "backuprestoredialog.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QVBoxLayout>

MainWindow::MainWindow(const QString& username, const QString& role, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentUser(username)
    , currentRole(role)
    , model(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Vision Entry Dashboard");

    setupDashboard(username, role);

    connect(ui->registerUserButton, &QPushButton::clicked, this, &MainWindow::onRegisterUser);
    connect(ui->exportCsvButton, &QPushButton::clicked, this, &MainWindow::onExportCSV);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogout);
    connect(ui->themeToggleButton, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    connect(ui->updateProfileButton, &QPushButton::clicked, this, [=]() {
        UpdateProfileDialog dialog(currentUserId);
        dialog.exec();
    });
    connect(ui->backupRestoreButton, &QPushButton::clicked, this, [=]() {
        BackupRestoreDialog dialog(this);
        dialog.exec();
    });

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupDashboard(const QString& username, const QString& role)
{
    currentUserId = -1;
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(currentUser);
    if (query.exec() && query.next()) {
        currentUserId = query.value(0).toInt();
    }

    ui->welcomeLabel->setText(QString("Welcome, %1 [%2]").arg(username, role));
    setupUIBasedOnRole();
    loadAttendanceData();

    if (currentRole == "Admin" || currentRole == "HR") {
        showAttendanceChart();
    } else {
        ui->attendanceChartContainer->hide();
    }
    checkAttendanceStatusAndToggleButton();
}

void MainWindow::setupUIBasedOnRole() {
    if (currentRole == "Employee") {
        ui->registerUserButton->hide();
        ui->exportCsvButton->hide();
    } else if (currentRole == "HR") {
        ui->registerUserButton->show();
        ui->exportCsvButton->show();
    } else if (currentRole == "Admin") {
        ui->registerUserButton->show();
        ui->exportCsvButton->hide();
    }
}

void MainWindow::loadAttendanceData() {
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    QString queryStr;

    if (currentRole == "Employee") {
        queryStr = QString(
                       "SELECT id, user_id, DATE(timestamp) AS date, TIME(timestamp) AS login_time, status "
                       "FROM attendance WHERE user_id = %1"
                       ).arg(currentUserId);
    } else {
        queryStr = "SELECT id, user_id, DATE(timestamp) AS date, TIME(timestamp) AS login_time, status FROM attendance";
    }

    queryModel->setQuery(queryStr);

    queryModel->setHeaderData(0, Qt::Horizontal, "ID");
    queryModel->setHeaderData(1, Qt::Horizontal, "User ID");
    queryModel->setHeaderData(2, Qt::Horizontal, "Date");
    queryModel->setHeaderData(3, Qt::Horizontal, "Login Time");
    queryModel->setHeaderData(4, Qt::Horizontal, "Status");

    ui->attendanceTableView->setModel(queryModel);
    ui->attendanceTableView->resizeColumnsToContents();

    Logger::instance()->log("Attendance data with Date & Time loaded.", Logger::INFO);
}

void MainWindow::onRegisterUser() {
    RegisterDialog regDialog(this);
    if (regDialog.exec() == QDialog::Accepted) {
        if (model) model->select();
    }
}

void MainWindow::onExportCSV() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Attendance", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Export Failed", "Cannot open file for writing.");
        return;
    }

    QTextStream out(&file);
    QSqlQuery query("SELECT a.id, u.full_name, u.role, a.timestamp, a.status "
                    "FROM attendance a JOIN users u ON a.user_id = u.id");

    out << "ID,Full Name,Role,Timestamp,Status\n";
    while (query.next()) {
        QString line = QString("%1,%2,%3,%4,%5")
        .arg(query.value(0).toString(),
             query.value(1).toString(),
             query.value(2).toString(),
             query.value(3).toString(),
             query.value(4).toString());
        out << line << "\n";
    }

    file.close();
    QMessageBox::information(this, "Exported", "Attendance exported successfully.");
    Logger::instance()->log("Attendance CSV exported to " + fileName, Logger::INFO);
}

void MainWindow::showAttendanceChart() {
    QBarSet *set = new QBarSet("Attendance");
    QStringList dayLabels;
    QMap<QString, int> attendanceMap;

    // Loop last 7 days (6 days ago to today)
    for (int i = 6; i >= 0; --i) {
        QDate date = QDate::currentDate().addDays(-i);
        QString label = date.toString("dd MMM");  // e.g. "07 Jul"
        dayLabels << label;

        QSqlQuery query;
        query.prepare("SELECT COUNT(DISTINCT user_id) FROM attendance WHERE DATE(timestamp) = ? AND status = 'Present'");
        query.addBindValue(date.toString("yyyy-MM-dd"));

        int count = 0;
        if (query.exec() && query.next()) {
            count = query.value(0).toInt();
        }
        attendanceMap[label] = count;
    }

    for (int i=0;i<dayLabels.size();i++)
        *set << attendanceMap[dayLabels[i]];

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Attendance in Last 7 Days");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // X-axis setup
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(dayLabels);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Y-axis setup with proper grid
    int maxVal = *std::max_element(attendanceMap.begin(), attendanceMap.end());
    if (maxVal < 3) maxVal = 3;

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxVal);
    axisY->setTickCount(maxVal + 1);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Users Present");

    // Styling for theme (including grid and axis lines)
    bool dark = ThemeManager::instance().isDarkTheme();
    QBrush axisBrush = dark ? QBrush(Qt::white) : QBrush(Qt::black);
    QPen axisPen = QPen(axisBrush.color());
    QPen gridPen = QPen(axisBrush.color().lighter(130));
    gridPen.setStyle(Qt::DashLine);

    axisX->setLabelsBrush(axisBrush);
    axisX->setLinePen(axisPen);
    axisX->setGridLinePen(gridPen);

    axisY->setLabelsBrush(axisBrush);
    axisY->setTitleBrush(axisBrush);
    axisY->setLinePen(axisPen);
    axisY->setGridLinePen(gridPen);

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Background color
    chart->setBackgroundBrush(dark ? QBrush(QColor("#2e2e2e")) : QBrush(Qt::white));
    chart->setTitleBrush(axisBrush);
    chart->legend()->setLabelBrush(axisBrush);

    // ChartView
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Clear and insert into container
    QLayout *oldLayout = ui->attendanceChartContainer->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    QVBoxLayout *layout = new QVBoxLayout(ui->attendanceChartContainer);
    layout->addWidget(chartView);
}

void MainWindow::checkAttendanceStatusAndToggleButton() {
    if (currentUserId == -1) return;

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM attendance WHERE user_id = ? AND DATE(timestamp) = CURDATE()");
    query.addBindValue(currentUserId);

    if (query.exec() && query.next()) {
        int count = query.value(0).toInt();
        if (count == 0) {
            ui->markAttendanceButton->setVisible(true);
            disconnect(ui->markAttendanceButton, nullptr, nullptr, nullptr);
            connect(ui->markAttendanceButton, &QPushButton::clicked, this, &MainWindow::handleFaceAttendance);
        } else {
            ui->markAttendanceButton->setVisible(false);
        }
    }
}

void MainWindow::handleFaceAttendance() {
    FaceLoginDialog dialog(this);
    dialog.setExpectedUserId(currentUserId);

    connect(&dialog, &FaceLoginDialog::loginSuccessful, this, [=](const QString &username, const QString &role) {
        Logger::instance()->log("Manual Face Attendance Success: " + username + " (" + role + ")", Logger::INFO);
        ui->markAttendanceButton->setVisible(false);
        loadAttendanceData();
    });

    dialog.exec();
}

void MainWindow::onLogout() {
    this->close();
}

void MainWindow::onToggleTheme() {
    ThemeManager::instance().toggleTheme();
    showAttendanceChart();
}
