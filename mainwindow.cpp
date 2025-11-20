#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "materiel.h"
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDateTime>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QFileDialog>
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

    // Initialize table with headers but no data
    QStringList headers;
    headers << "Code Barre" << "Nom" << "Type" << "Etat" << "Quantité";
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setRowCount(0); // Start with empty table

    // Setup notification table
    setupNotificationTable();

    // Setup statistics layout
    ui->groupBox_4->setLayout(new QVBoxLayout());

    // Connect navigation buttons
    connect(ui->buttonGoToPage2, &QPushButton::clicked, this, &MainWindow::goToPage2);
    connect(ui->buttonGoBack, &QPushButton::clicked, this, &MainWindow::goBackToPage1);

    // Connect functionality buttons
    connect(ui->valider, &QPushButton::clicked, this, &MainWindow::on_valider_clicked);
    connect(ui->annuler, &QPushButton::clicked, this, &MainWindow::on_annuler_clicked);
    connect(ui->recherche, &QPushButton::clicked, this, &MainWindow::on_recherche_clicked);
    connect(ui->trier, &QPushButton::clicked, this, &MainWindow::on_trier_clicked);
    connect(ui->supprimer, &QPushButton::clicked, this, &MainWindow::on_supprimer_clicked);
    connect(ui->modifier, &QPushButton::clicked, this, &MainWindow::on_modifier_clicked);
    connect(ui->supprimer2, &QPushButton::clicked, this, &MainWindow::on_supprimer2_clicked);

    // Connect the export PDF button
    connect(ui->exportToPDF, &QPushButton::clicked, this, &MainWindow::on_exportPDF_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupNotificationTable()
{
    // Setup notification table headers
    QStringList notificationHeaders;
    notificationHeaders << "Date" << "Notifications";
    ui->tableWidget_3->setColumnCount(2);
    ui->tableWidget_3->setHorizontalHeaderLabels(notificationHeaders);
    ui->tableWidget_3->setRowCount(0);

    // Set column widths
    ui->tableWidget_3->setColumnWidth(0, 200); // Date column
    ui->tableWidget_3->setColumnWidth(1, 700); // Notification column
}

void MainWindow::addNotification(const QString &message)
{
    // Get current date and time
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // Add new row to notification table
    int row = ui->tableWidget_3->rowCount();
    ui->tableWidget_3->insertRow(row);

    // Set date and message
    ui->tableWidget_3->setItem(row, 0, new QTableWidgetItem(currentDateTime));
    ui->tableWidget_3->setItem(row, 1, new QTableWidgetItem(message));

    // Auto-scroll to the bottom to show latest notification
    ui->tableWidget_3->scrollToBottom();

    qDebug() << "Notification added:" << message;
}

void MainWindow::goToPage2()
{
    ui->stackedWidget->setCurrentIndex(1);
    updateStatistics(); // Refresh statistics when switching to page 2
}

void MainWindow::goBackToPage1()
{
    ui->stackedWidget->setCurrentIndex(0);
    // Clear the table instead of refreshing with all data
    ui->tableWidget->setRowCount(0);
    ui->idrech->clear(); // Clear search field
}

bool MainWindow::containsOnlyLettersAndSpaces(const QString &str)
{
    if (str.isEmpty()) return true;
    QRegularExpression regex("^[a-zA-Z\\sÀ-ÿ]+$");
    return regex.match(str).hasMatch();
}

bool MainWindow::containsOnlyLettersAndNumbers(const QString &str)
{
    if (str.isEmpty()) return true;
    QRegularExpression regex("^[a-zA-Z0-9]+$");
    return regex.match(str).hasMatch();
}

bool MainWindow::containsOnlyNumbers(const QString &str)
{
    if (str.isEmpty()) return true;
    QRegularExpression regex("^[0-9]+$");
    return regex.match(str).hasMatch();
}

bool MainWindow::validateInputs()
{
    QString code_barre = ui->add_2->text().trimmed();
    QString nom = ui->id->text().trimmed();
    QString type = ui->numtel->text().trimmed();
    QString etat = ui->numapp->text().trimmed();
    QString qtiteText = ui->add->text().trimmed();

    // Check required fields
    if (code_barre.isEmpty() || nom.isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires",
                             "Le code barre et le nom sont obligatoires!");
        return false;
    }

    // Validate code barre (letters and numbers only)
    if (!containsOnlyLettersAndNumbers(code_barre)) {
        QMessageBox::warning(this, "Erreur de saisie",
                             "Le code barre ne doit contenir que des lettres et des chiffres!");
        return false;
    }

    if (code_barre.length() > 100) {
        QMessageBox::warning(this, "Erreur", "Le code barre est trop long (max 100 caractères)!");
        return false;
    }

    // Validate nom (letters and spaces only)
    if (!containsOnlyLettersAndSpaces(nom)) {
        QMessageBox::warning(this, "Erreur de saisie",
                             "Le nom ne doit contenir que des lettres et des espaces!");
        return false;
    }

    if (nom.length() > 100) {
        QMessageBox::warning(this, "Erreur", "Le nom est trop long (max 100 caractères)!");
        return false;
    }

    // Validate type (letters and numbers only)
    if (!type.isEmpty() && !containsOnlyLettersAndNumbers(type)) {
        QMessageBox::warning(this, "Erreur de saisie",
                             "Le type ne doit contenir que des lettres et des chiffres!");
        return false;
    }

    if (type.length() > 100) {
        QMessageBox::warning(this, "Erreur", "Le type est trop long (max 100 caractères)!");
        return false;
    }

    // Validate etat - only allowed values
    if (!etat.isEmpty()) {
        QStringList allowedEtats = {"En Reparation", "En Panne", "En Cours d'Utilisation"};
        if (!allowedEtats.contains(etat)) {
            QMessageBox::warning(this, "Erreur de saisie",
                                 "L'état doit être l'un des suivants:\n"
                                 "'En Reparation', 'En Panne', 'En Cours d'Utilisation'");
            return false;
        }
    }

    if (etat.length() > 50) {
        QMessageBox::warning(this, "Erreur", "L'état est trop long (max 50 caractères)!");
        return false;
    }

    // Validate qtite (numbers only)
    if (!qtiteText.isEmpty() && !containsOnlyNumbers(qtiteText)) {
        QMessageBox::warning(this, "Erreur de saisie", "La quantité doit contenir uniquement des chiffres!");
        return false;
    }

    bool ok;
    int qtite = qtiteText.isEmpty() ? 0 : qtiteText.toInt(&ok);
    if (!ok || qtite < 0) {
        QMessageBox::warning(this, "Erreur de saisie", "La quantité doit être un nombre positif!");
        return false;
    }

    return true;
}

void MainWindow::on_valider_clicked()
{
    qDebug() << "=== VALIDER CLICKED ===";

    // Validate inputs first
    if (!validateInputs()) {
        qDebug() << "Validation failed";
        return;
    }

    qDebug() << "Validation passed, proceeding with addition";

    QString code_barre = ui->add_2->text().trimmed();
    QString nom = ui->id->text().trimmed();
    QString type = ui->numtel->text().trimmed();
    QString etat = ui->numapp->text().trimmed();
    int qtite = ui->add->text().toInt();

    Materiel m(code_barre, nom, type, etat, qtite);
    bool test = m.ajouter();

    if (test) {
        qDebug() << "Material added successfully";

        // Add notification
        QString notificationMessage = QString("Matériel ajouté - Code: %1, Nom: %2, Type: %3, État: %4, Quantité: %5")
                                          .arg(code_barre)
                                          .arg(nom)
                                          .arg(type)
                                          .arg(etat)
                                          .arg(qtite);
        addNotification(notificationMessage);

        QMessageBox::information(this, "Succès", "Matériel ajouté avec succès!");
        clearAddFields();
        updateStatistics(); // Update statistics
        return;
    } else {
        qDebug() << "Failed to add material";
        QMessageBox::critical(this, "Erreur",
                              "Erreur lors de l'ajout du matériel!\n"
                              "Vérifiez que le code barre n'existe pas déjà.");
    }
}

void MainWindow::on_annuler_clicked()
{
    clearAddFields();
}

void MainWindow::on_recherche_clicked()
{
    QString code_barre = ui->idrech->text().trimmed();
    Materiel m;

    if (code_barre.isEmpty()) {
        ui->tableWidget->setRowCount(0);
        QMessageBox::information(this, "Recherche", "Veuillez saisir un code barre pour rechercher.");
    } else {
        m.rechercher(ui->tableWidget, code_barre);

        if (ui->tableWidget->rowCount() == 0) {
            QMessageBox::information(this, "Recherche",
                                     "Aucun matériel trouvé avec le code barre: " + code_barre);
        }
    }
}

void MainWindow::on_trier_clicked()
{
    QString critere = ui->tripar->currentText();
    Materiel m;
    m.trier(ui->tableWidget, critere);

    if (ui->tableWidget->rowCount() == 0) {
        QMessageBox::information(this, "Tri", "Aucun matériel à trier.");
    }
}

void MainWindow::on_supprimer_clicked()
{
    QString code_barre = ui->idmodsup->text().trimmed();

    if (code_barre.isEmpty()) {
        QMessageBox::warning(this, "Champ vide", "Veuillez saisir un code barre!");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Voulez-vous vraiment supprimer le matériel avec le code barre:\n"
                                      + code_barre + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        Materiel m;
        bool test = m.supprimer(code_barre);

        if (test) {
            // Add notification
            QString notificationMessage = QString("Matériel supprimé - Code barre: %1").arg(code_barre);
            addNotification(notificationMessage);

            QMessageBox::information(this, "Succès", "Matériel supprimé avec succès!");
            ui->idmodsup->clear();
            updateStatistics(); // Update statistics
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la suppression!\nMatériel non trouvé.");
        }
    }
}

void MainWindow::on_modifier_clicked()
{
    qDebug() << "=== MODIFIER CLICKED ===";

    QString code_barre = ui->idmodsup->text().trimmed();

    if (code_barre.isEmpty()) {
        QMessageBox::warning(this, "Champ vide", "Veuillez saisir un code barre!");
        return;
    }

    // Check if the material exists
    Materiel m_check;
    if (!m_check.existe(code_barre)) {
        QMessageBox::warning(this, "Non trouvé", "Aucun matériel trouvé avec ce code barre!");
        return;
    }

    // Get the new data from input fields
    QString new_nom = ui->id->text().trimmed();
    QString new_type = ui->numtel->text().trimmed();
    QString new_etat = ui->numapp->text().trimmed();
    QString new_qtiteText = ui->add->text().trimmed();

    // Check if at least one field is filled for modification
    if (new_nom.isEmpty() && new_type.isEmpty() && new_etat.isEmpty() && new_qtiteText.isEmpty()) {
        QMessageBox::warning(this, "Aucune modification",
                             "Veuillez saisir au moins un champ à modifier!");
        return;
    }

    // Validate only the fields that are not empty
    if (!new_nom.isEmpty()) {
        if (!containsOnlyLettersAndSpaces(new_nom)) {
            QMessageBox::warning(this, "Erreur de saisie", "Le nom ne doit contenir que des lettres et des espaces!");
            return;
        }
        if (new_nom.length() > 100) {
            QMessageBox::warning(this, "Erreur", "Le nom est trop long (max 100 caractères)!");
            return;
        }
    }

    if (!new_type.isEmpty()) {
        if (!containsOnlyLettersAndNumbers(new_type)) {
            QMessageBox::warning(this, "Erreur de saisie", "Le type ne doit contenir que des lettres et des chiffres!");
            return;
        }
        if (new_type.length() > 100) {
            QMessageBox::warning(this, "Erreur", "Le type est trop long (max 100 caractères)!");
            return;
        }
    }

    if (!new_etat.isEmpty()) {
        QStringList allowedEtats = {"En Reparation", "En Panne", "En Cours d'Utilisation"};
        if (!allowedEtats.contains(new_etat)) {
            QMessageBox::warning(this, "Erreur de saisie",
                                 "L'état doit être l'un des suivants:\n"
                                 "'En Reparation', 'En Panne', 'En Cours d'Utilisation'");
            return;
        }
        if (new_etat.length() > 50) {
            QMessageBox::warning(this, "Erreur", "L'état est trop long (max 50 caractères)!");
            return;
        }
    }

    if (!new_qtiteText.isEmpty()) {
        if (!containsOnlyNumbers(new_qtiteText)) {
            QMessageBox::warning(this, "Erreur de saisie", "La quantité doit contenir uniquement des chiffres!");
            return;
        }

        bool ok;
        int new_qtite = new_qtiteText.toInt(&ok);
        if (!ok || new_qtite < 0) {
            QMessageBox::warning(this, "Erreur de saisie", "La quantité doit être un nombre positif!");
            return;
        }
    }

    // Prepare confirmation message with only the fields that will be changed
    QString confirmationMessage = "Voulez-vous vraiment modifier le matériel avec le code barre:\n" + code_barre + "?\n\n";
    QString changesMessage = "Changements:\n";

    if (!new_nom.isEmpty()) changesMessage += "Nom: " + new_nom + "\n";
    if (!new_type.isEmpty()) changesMessage += "Type: " + new_type + "\n";
    if (!new_etat.isEmpty()) changesMessage += "État: " + new_etat + "\n";
    if (!new_qtiteText.isEmpty()) changesMessage += "Quantité: " + new_qtiteText + "\n";

    if (changesMessage == "Changements:\n") {
        changesMessage = "Aucun changement spécifié";
    }

    confirmationMessage += changesMessage;

    // Ask for confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation de modification",
                                  confirmationMessage,
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Proceeding with modification";

        // Create Materiel object with the data that will be changed
        QString final_nom = new_nom.isEmpty() ? "" : new_nom;
        QString final_type = new_type.isEmpty() ? "" : new_type;
        QString final_etat = new_etat.isEmpty() ? "" : new_etat;
        int final_qtite = new_qtiteText.isEmpty() ? 0 : new_qtiteText.toInt();

        Materiel m(final_nom, final_nom, final_type, final_etat, final_qtite);
        bool test = m.modifier(code_barre);

        if (test) {
            qDebug() << "Material modified successfully";

            // Prepare notification message with only the changed fields
            QString notificationMessage = "Matériel modifié - Code: " + code_barre;
            if (!new_nom.isEmpty()) notificationMessage += ", Nouveau nom: " + new_nom;
            if (!new_type.isEmpty()) notificationMessage += ", Type: " + new_type;
            if (!new_etat.isEmpty()) notificationMessage += ", État: " + new_etat;
            if (!new_qtiteText.isEmpty()) notificationMessage += ", Quantité: " + new_qtiteText;

            addNotification(notificationMessage);

            QMessageBox::information(this, "Succès", "Matériel modifié avec succès!");
            clearAddFields();
            ui->idmodsup->clear();
            updateStatistics(); // Update statistics
        } else {
            qDebug() << "Failed to modify material";
            QMessageBox::critical(this, "Erreur", "Erreur lors de la modification du matériel!");
        }
    }
}

void MainWindow::on_exportPDF_clicked()
{
    // Get all materials from database
    QSqlQuery query;
    QString queryStr = "SELECT CODE_BARRE, NOM_MATR, TYPE_MAT, ETAT, QTITE FROM MATERIEL ORDER BY NOM_MATR";

    if (!query.exec(queryStr)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la récupération des données: " + query.lastError().text());
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "liste_materiels.pdf", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    // Create a SIMPLE text-based approach
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le PDF");
        return;
    }

    // Use VERY LARGE fonts
    QFont headerFont("Arial", 16, QFont::Bold);
    QFont dataFont("Arial", 12);

    int y = 100;
    int rowCount = 0;

    // Title
    painter.setFont(headerFont);
    painter.drawText(100, y, "LISTE DES MATÉRIELS");
    y += 50;

    // Headers
    painter.drawText(100, y, "CODE BARRE");
    painter.drawText(250, y, "NOM");
    painter.drawText(450, y, "TYPE");
    painter.drawText(600, y, "ETAT");
    painter.drawText(750, y, "QUANTITE");
    y += 40;

    // Data
    painter.setFont(dataFont);
    while (query.next()) {
        if (y > 700) { // New page if needed
            printer.newPage();
            y = 100;
        }

        painter.drawText(100, y, query.value(0).toString());
        painter.drawText(250, y, query.value(1).toString());
        painter.drawText(450, y, query.value(2).toString());
        painter.drawText(600, y, query.value(3).toString());
        painter.drawText(750, y, query.value(4).toString());

        y += 30;
        rowCount++;
    }

    painter.end();

    // Notification
    addNotification(QString("Export PDF - %1 matériels").arg(rowCount));
    QMessageBox::information(this, "Succès", QString("PDF créé: %1 matériels").arg(rowCount));
}

void MainWindow::on_supprimer2_clicked()
{
    // Clear the notification table
    ui->tableWidget_3->setRowCount(0);
    QMessageBox::information(this, "Notifications", "Toutes les notifications ont été supprimées.");
}

void MainWindow::clearAddFields()
{
    ui->add_2->clear();
    ui->id->clear();
    ui->numtel->clear();
    ui->numapp->clear();
    ui->add->clear();
}

void MainWindow::refreshTable()
{
    Materiel m;
    m.afficher(ui->tableWidget);
}

void MainWindow::updateStatistics()
{
    QSqlQuery query;

    // Get total count
    int total = 0;
    if (query.exec("SELECT COUNT(*) FROM MATERIEL") && query.next()) {
        total = query.value(0).toInt();
    }

    if (total == 0) {
        // Clear statistics if no data
        QLayoutItem* child;
        while ((child = ui->groupBox_4->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        // Add "No data" message
        QLabel *noDataLabel = new QLabel("Aucun matériel dans la base de données");
        noDataLabel->setStyleSheet("font-weight: bold; font-size: 12pt; color: #666666;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        ui->groupBox_4->layout()->addWidget(noDataLabel);
        return;
    }

    // Get counts for each state
    QMap<QString, int> stateCounts;
    QStringList states = {"En Reparation", "En Panne", "En Cours d'Utilisation"};

    for (const QString& state : states) {
        query.prepare("SELECT COUNT(*) FROM MATERIEL WHERE ETAT = :etat");
        query.bindValue(":etat", state);
        if (query.exec() && query.next()) {
            stateCounts[state] = query.value(0).toInt();
        }
    }

    // Clear existing layout
    QLayoutItem* child;
    while ((child = ui->groupBox_4->layout()->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // Add title
    QLabel *titleLabel = new QLabel("Statistiques des États");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14pt; color: #007DD1; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    ui->groupBox_4->layout()->addWidget(titleLabel);

    // Add statistics for each state
    for (const QString& state : states) {
        int count = stateCounts[state];
        double percentage = total > 0 ? (count * 100.0) / total : 0;

        QHBoxLayout *stateLayout = new QHBoxLayout();

        // State label
        QLabel *stateLabel = new QLabel(state + ":");
        stateLabel->setStyleSheet("font-weight: bold; color: #333333; min-width: 180px;");
        stateLayout->addWidget(stateLabel);

        // Percentage label
        QLabel *percentLabel = new QLabel(QString::number(percentage, 'f', 1) + "%");
        percentLabel->setStyleSheet("color: #007DD1; min-width: 60px; font-weight: bold;");
        stateLayout->addWidget(percentLabel);

        // Progress bar
        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(static_cast<int>(percentage));
        progressBar->setStyleSheet(getProgressBarStyle(state));
        progressBar->setFixedHeight(20);
        progressBar->setFixedWidth(200);
        stateLayout->addWidget(progressBar);

        // Count label
        QLabel *countLabel = new QLabel("(" + QString::number(count) + ")");
        countLabel->setStyleSheet("color: #666666; min-width: 50px;");
        stateLayout->addWidget(countLabel);

        QWidget *container = new QWidget();
        container->setLayout(stateLayout);
        ui->groupBox_4->layout()->addWidget(container);
    }

    // Add total count
    QLabel *totalLabel = new QLabel("Total: " + QString::number(total) + " matériels");
    totalLabel->setStyleSheet("font-weight: bold; color: #24B37A; margin-top: 10px; font-size: 12pt;");
    totalLabel->setAlignment(Qt::AlignCenter);
    ui->groupBox_4->layout()->addWidget(totalLabel);
}

QString MainWindow::getProgressBarStyle(const QString& state)
{
    if (state == "En Reparation") {
        return "QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; }"
               "QProgressBar::chunk { background-color: #FFA500; }";
    } else if (state == "En Panne") {
        return "QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; }"
               "QProgressBar::chunk { background-color: #FF0000; }";
    } else { // En Cours d'Utilisation
        return "QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; }"
               "QProgressBar::chunk { background-color: #008000; }";
    }
}
