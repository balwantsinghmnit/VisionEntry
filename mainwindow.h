#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, const QString& role, QWidget *parent = nullptr);
    ~MainWindow();

    void setupDashboard(const QString& username, const QString& role);

private slots:
    void onRegisterUser();
    void onExportCSV();
    void onLogout();
    void onToggleTheme();


private:
    void setupUIBasedOnRole();
    void loadAttendanceData();
    void showAttendanceChart();
    void exportAttendanceToCSV();
    void checkAttendanceStatusAndToggleButton();
    void handleFaceAttendance();

    Ui::MainWindow *ui;
    QString currentUser;
    QString currentRole;
    QSqlTableModel *model;
    int currentUserId;
};

#endif // MAINWINDOW_H
