#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void goToPage2();
    void goBackToPage1();

    void on_valider_clicked();
    void on_annuler_clicked();
    void on_recherche_clicked();
    void on_trier_clicked();
    void on_supprimer_clicked();
    void on_modifier_clicked();
    void on_supprimer2_clicked();
    void on_exportPDF_clicked();

private:
    Ui::MainWindow *ui;
    void clearAddFields();
    void refreshTable();
    bool validateInputs();
    bool containsOnlyLettersAndSpaces(const QString &str);
    bool containsOnlyLettersAndNumbers(const QString &str);
    bool containsOnlyNumbers(const QString &str);
    void addNotification(const QString &message);
    void setupNotificationTable();
    void updateStatistics();
    QString getProgressBarStyle(const QString& state);
};

#endif // MAINWINDOW_H
