#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QAbstractAxis>

class ThemeManager {
public:
    static ThemeManager& instance();

    void applyDarkTheme();
    void applyLightTheme();
    void toggleTheme();
    bool isDarkTheme() const;

    void applyTheme(QWidget *widget);  // ✅ ADD THIS
    void styleChart(QChart *chart, QAbstractAxis *xAxis, QAbstractAxis *yAxis);  // ✅ ADD THIS

private:
    ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    bool darkMode;
};

#endif // THEMEMANAGER_H
