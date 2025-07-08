#include "thememanager.h"
#include <QApplication>
#include <QPalette>
#include <QChart>
#include <QAbstractAxis>

ThemeManager::ThemeManager() : darkMode(false) {}

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

void ThemeManager::applyDarkTheme() {
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #2e2e2e;
            color: white;
        }
        QLabel#welcomeLabel {
            font-weight: bold;
            font-size: 16px;
            color: white;
        }
        QTableView {
            background-color: #3a3a3a;
            alternate-background-color: #444444;
            color: white;
            gridline-color: gray;
            selection-background-color: #555555;
        }
        QHeaderView::section {
            background-color: #444444;
            color: white;
            padding: 4px;
            border: 1px solid #333;
        }
        QTableCornerButton::section {
            background-color: #444444;
            border: 1px solid #333;
        }
        QPushButton {
            background-color: #4a4a4a;
            color: white;
            border: 1px solid #555;
            padding: 5px 10px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #5a5a5a;
        }
        QPushButton:pressed {
            background-color: #3a3a3a;
        }
    )");
    darkMode = true;
}

void ThemeManager::applyLightTheme() {
    qApp->setStyleSheet("");  // Revert to default
    darkMode = false;
}

void ThemeManager::toggleTheme() {
    if (darkMode) applyLightTheme();
    else applyDarkTheme();
}

bool ThemeManager::isDarkTheme() const {
    return darkMode;
}

//Apply current theme to any widget
void ThemeManager::applyTheme(QWidget *widget) {
    if (darkMode)
        applyDarkTheme();
    else
        applyLightTheme();
}

void ThemeManager::styleChart(QChart *chart, QAbstractAxis *xAxis, QAbstractAxis *yAxis) {
    if (!chart || !xAxis || !yAxis) return;

    if (darkMode) {
        chart->setBackgroundBrush(QBrush(QColor("#2e2e2e")));
        chart->setTitleBrush(QBrush(Qt::white));
        xAxis->setLabelsBrush(QBrush(Qt::white));
        yAxis->setLabelsBrush(QBrush(Qt::white));
    } else {
        chart->setBackgroundBrush(QBrush(Qt::white));
        chart->setTitleBrush(QBrush(Qt::black));
        xAxis->setLabelsBrush(QBrush(Qt::black));
        yAxis->setLabelsBrush(QBrush(Qt::black));
    }
}
