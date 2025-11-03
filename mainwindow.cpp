#include "mainwindow.h"
#include "DatabaseManager.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDebug>
#include <QHeaderView>
#include <QFormLayout>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QPrinter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTextStream>

// QtCharts includes
#include <QBarSet>
#include <QBarSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    dbManager(nullptr),
    currentClientId(-1),
    currentCommandeId(-1),
    isEditingClient(false),
    isEditingCommande(false)
{
    dbManager = new DatabaseManager(this);
    if (!dbManager->open()) {
        QMessageBox::critical(this, "Erreur", "Impossible de se connecter à la base de données");
        return;
    }

    setupUI();
    loadClientsTable();
    loadCommandesTable();
    updateStatisticsCharts();
}

MainWindow::~MainWindow()
{
    // dbManager is deleted automatically as child
}

void MainWindow::applyModernTableStyle(QTableWidget *table)
{
    table->setStyleSheet(R"(
        QTableWidget {
            background-color: #1e1e1e;
            alternate-background-color: #252525;
            selection-background-color: #2a7fff;
            selection-color: white;
            gridline-color: #404040;
            border: 1px solid #404040;
            border-radius: 8px;
            color: #e0e0e0;
            font-size: 12px;
        }
        QTableWidget::item {
            padding: 10px;
            border-bottom: 1px solid #333333;
        }
        QTableWidget::item:selected {
            background-color: #2a7fff;
            color: white;
            border-radius: 4px;
        }
        QTableWidget::item:hover {
            background-color: #2d2d2d;
        }
        QHeaderView::section {
            background-color: #2d2d2d;
            color: #ffffff;
            padding: 12px;
            border: none;
            font-weight: bold;
            font-size: 13px;
            border-bottom: 2px solid #2a7fff;
        }
        QTableWidget QScrollBar:vertical {
            background: #2d2d2d;
            width: 12px;
            margin: 0px;
        }
        QTableWidget QScrollBar::handle:vertical {
            background: #404040;
            border-radius: 6px;
            min-height: 20px;
        }
        QTableWidget QScrollBar::handle:vertical:hover {
            background: #4a4a4a;
        }
    )");

    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
}

void MainWindow::applyModernButtonStyle(QPushButton *button, const QString &color)
{
    QString hoverColor, pressedColor, textColor = "white";

    if (color == "#2a7fff") { // Primary Blue
        hoverColor = "#3d8eff";
        pressedColor = "#1a6fe6";
    } else if (color == "#00d4aa") { // Success Green
        hoverColor = "#00e6b8";
        pressedColor = "#00c295";
    } else if (color == "#ff6b35") { // Warning Orange
        hoverColor = "#ff7a4a";
        pressedColor = "#e65c2a";
    } else if (color == "#ff4757") { // Danger Red
        hoverColor = "#ff5c6c";
        pressedColor = "#e63d4d";
    } else if (color == "#a55eea") { // Info Purple
        hoverColor = "#b370ff";
        pressedColor = "#954dd6";
    } else if (color == "#6c757d") { // Secondary Gray
        hoverColor = "#7a8288";
        pressedColor = "#5a6268";
    } else {
        hoverColor = color;
        pressedColor = color;
    }

    button->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            padding: 10px 20px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 13px;
            min-width: 90px;
        }
        QPushButton:hover {
            background-color: %3;
        }
        QPushButton:pressed {
            background-color: %4;
        }
        QPushButton:disabled {
            background-color: #4a4a4a;
            color: #777777;
        }
    )").arg(color, textColor, hoverColor, pressedColor));
}

void MainWindow::setupUI()
{
    // Set window properties
    setWindowTitle("Gestion de Crédit Pro - Business Suite");
    setMinimumSize(1400, 900);

    centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #121212; color: #e0e0e0;");
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Header
    headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                      stop:0 #1a1a2e, stop:1 #16213e);
            border-radius: 12px;
            padding: 20px;
            border: 1px solid #2a2a4a;
        }
    )");
    headerLayout = new QHBoxLayout(headerFrame);

    headerTitle = new QLabel("🚀 Gestion de Crédit Professionnel", this);
    headerTitle->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
            font-size: 28px;
            font-weight: bold;
            background: transparent;
        }
    )");

    // Navigation buttons
    btnClients = new QPushButton("👥 Clients", this);
    btnCommandes = new QPushButton("📦 Commandes", this);
    btnStatistics = new QPushButton("📊 Statistiques", this);

    applyModernButtonStyle(btnClients, "#00d4aa");
    applyModernButtonStyle(btnCommandes, "#2a7fff");
    applyModernButtonStyle(btnStatistics, "#a55eea");

    headerLayout->addWidget(headerTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(btnClients);
    headerLayout->addWidget(btnCommandes);
    headerLayout->addWidget(btnStatistics);

    mainLayout->addWidget(headerFrame);

    // Stacked widget for sections
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    setupClientSection();
    setupCommandeSection();
    setupStatisticsSection();

    // Connect header buttons
    connect(btnClients, &QPushButton::clicked, this, &MainWindow::showClientSection);
    connect(btnCommandes, &QPushButton::clicked, this, &MainWindow::showCommandeSection);
    connect(btnStatistics, &QPushButton::clicked, this, &MainWindow::showStatisticsSection);

    // Start with client section
    showClientSection();
}

void MainWindow::setupClientSection()
{
    clientWidget = new QWidget();
    clientWidget->setStyleSheet("background: transparent;");
    clientLayout = new QVBoxLayout(clientWidget);
    clientLayout->setSpacing(20);
    clientLayout->setContentsMargins(0, 0, 0, 0);

    // Client Controls Frame
    clientControlFrame = new QFrame(this);
    clientControlFrame->setStyleSheet(R"(
        QFrame {
            background-color: #1e1e2e;
            border-radius: 12px;
            padding: 15px;
            border: 1px solid #2a2a3a;
        }
    )");
    clientButtonLayout = new QHBoxLayout(clientControlFrame);

    btnAddClient = new QPushButton("➕ Nouveau Client", this);
    btnEditClient = new QPushButton("✏️ Modifier", this);
    btnDeleteClient = new QPushButton("🗑️ Supprimer", this);
    btnRefreshClients = new QPushButton("🔄 Actualiser", this);
    btnExportClientsPDF = new QPushButton("📄 PDF Clients", this);
    btnClientAnalytics = new QPushButton("📊 Analytics", this);
    btnClientDetails = new QPushButton("👁️ Détails", this);

    applyModernButtonStyle(btnAddClient, "#00d4aa");
    applyModernButtonStyle(btnEditClient, "#2a7fff");
    applyModernButtonStyle(btnDeleteClient, "#ff4757");
    applyModernButtonStyle(btnRefreshClients, "#6c757d");
    applyModernButtonStyle(btnExportClientsPDF, "#ff6b35");
    applyModernButtonStyle(btnClientAnalytics, "#a55eea");
    applyModernButtonStyle(btnClientDetails, "#00d4aa");

    clientButtonLayout->addWidget(btnAddClient);
    clientButtonLayout->addWidget(btnEditClient);
    clientButtonLayout->addWidget(btnDeleteClient);
    clientButtonLayout->addWidget(btnClientDetails);
    clientButtonLayout->addWidget(btnClientAnalytics);
    clientButtonLayout->addStretch();
    clientButtonLayout->addWidget(btnExportClientsPDF);
    clientButtonLayout->addWidget(btnRefreshClients);

    // Client Search Frame
    clientSearchFrame = new QFrame(this);
    clientSearchFrame->setStyleSheet(R"(
        QFrame {
            background-color: #1e1e2e;
            border-radius: 12px;
            padding: 15px;
            border: 1px solid #2a2a3a;
        }
    )");
    clientSearchLayout = new QHBoxLayout(clientSearchFrame);

    txtSearchClient = new QLineEdit(this);
    txtSearchClient->setPlaceholderText("🔍 Rechercher par nom, prénom ou email...");
    txtSearchClient->setStyleSheet(R"(
        QLineEdit {
            background-color: #2d2d3d;
            color: #ffffff;
            border: 2px solid #3d3d4d;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            selection-background-color: #2a7fff;
        }
        QLineEdit:focus {
            border-color: #2a7fff;
            background-color: #252535;
        }
        QLineEdit::placeholder {
            color: #888888;
            font-style: italic;
        }
    )");

    btnSearchClient = new QPushButton("Rechercher", this);
    applyModernButtonStyle(btnSearchClient, "#2a7fff");

    clientSearchLayout->addWidget(txtSearchClient);
    clientSearchLayout->addWidget(btnSearchClient);

    // Clients Table Group
    clientTableGroup = new QGroupBox("📋 Liste des Clients", this);
    clientTableGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 18px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");

    QVBoxLayout *tableLayout = new QVBoxLayout(clientTableGroup);
    clientsTable = new QTableWidget(this);
    clientsTable->setColumnCount(7); // Added command count column
    clientsTable->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Email", "Téléphone", "Adresse", "Nb Commandes"});
    applyModernTableStyle(clientsTable);
    tableLayout->addWidget(clientsTable);

    // Client Form Group (initially hidden)
    clientFormGroup = new QGroupBox("📝 Formulaire Client", this);
    clientFormGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 18px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");
    clientFormGroup->setVisible(false);

    clientFormLayout = new QVBoxLayout(clientFormGroup);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setSpacing(15);

    // Style form elements
    QString inputStyle = R"(
        QLineEdit, QTextEdit, QComboBox, QDateEdit {
            background-color: #2d2d3d;
            color: #ffffff;
            border: 2px solid #3d3d4d;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            selection-background-color: #2a7fff;
        }
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QDateEdit:focus {
            border-color: #2a7fff;
            background-color: #252535;
        }
        QTextEdit {
            min-height: 100px;
        }
        QComboBox::drop-down {
            border: none;
            background: #3d3d4d;
            border-radius: 0 6px 6px 0;
        }
        QComboBox QAbstractItemView {
            background-color: #2d2d3d;
            color: #ffffff;
            selection-background-color: #2a7fff;
            border: 1px solid #3d3d4d;
        }
    )";

    txtClientNom = new QLineEdit(this);
    txtClientPrenom = new QLineEdit(this);
    txtClientEmail = new QLineEdit(this);
    txtClientTelephone = new QLineEdit(this);
    txtClientAdresse = new QTextEdit(this);

    txtClientNom->setStyleSheet(inputStyle);
    txtClientPrenom->setStyleSheet(inputStyle);
    txtClientEmail->setStyleSheet(inputStyle);
    txtClientTelephone->setStyleSheet(inputStyle);
    txtClientAdresse->setStyleSheet(inputStyle);

    // Create styled labels
    QLabel *labelNom = new QLabel("👤 Nom *:");
    QLabel *labelPrenom = new QLabel("👤 Prénom *:");
    QLabel *labelEmail = new QLabel("📧 Email *:");
    QLabel *labelTelephone = new QLabel("📞 Téléphone:");
    QLabel *labelAdresse = new QLabel("🏠 Adresse:");

    QString labelStyle = "QLabel { color: #e0e0e0; font-weight: bold; font-size: 14px; }";
    labelNom->setStyleSheet(labelStyle);
    labelPrenom->setStyleSheet(labelStyle);
    labelEmail->setStyleSheet(labelStyle);
    labelTelephone->setStyleSheet(labelStyle);
    labelAdresse->setStyleSheet(labelStyle);

    formLayout->addRow(labelNom, txtClientNom);
    formLayout->addRow(labelPrenom, txtClientPrenom);
    formLayout->addRow(labelEmail, txtClientEmail);
    formLayout->addRow(labelTelephone, txtClientTelephone);
    formLayout->addRow(labelAdresse, txtClientAdresse);

    clientFormLayout->addLayout(formLayout);

    QHBoxLayout *clientFormButtons = new QHBoxLayout();
    btnSaveClient = new QPushButton("💾 Sauvegarder", this);
    btnCancelClient = new QPushButton("❌ Annuler", this);

    applyModernButtonStyle(btnSaveClient, "#00d4aa");
    applyModernButtonStyle(btnCancelClient, "#6c757d");

    clientFormButtons->addWidget(btnSaveClient);
    clientFormButtons->addWidget(btnCancelClient);
    clientFormButtons->addStretch();

    clientFormLayout->addLayout(clientFormButtons);

    // Add all to client layout
    clientLayout->addWidget(clientControlFrame);
    clientLayout->addWidget(clientSearchFrame);
    clientLayout->addWidget(clientTableGroup);
    clientLayout->addWidget(clientFormGroup);

    // Connect client signals
    connect(btnAddClient, &QPushButton::clicked, this, &MainWindow::addNewClient);
    connect(btnEditClient, &QPushButton::clicked, this, &MainWindow::editSelectedClient);
    connect(btnDeleteClient, &QPushButton::clicked, this, &MainWindow::deleteSelectedClient);
    connect(btnRefreshClients, &QPushButton::clicked, this, &MainWindow::loadClientsTable);
    connect(btnSearchClient, &QPushButton::clicked, this, &MainWindow::searchClients);
    connect(btnSaveClient, &QPushButton::clicked, this, &MainWindow::saveClient);
    connect(btnCancelClient, &QPushButton::clicked, this, &MainWindow::cancelClientEdit);
    connect(btnExportClientsPDF, &QPushButton::clicked, this, &MainWindow::exportClientsPDF);
    connect(btnClientAnalytics, &QPushButton::clicked, this, &MainWindow::showClientAnalytics);
    connect(btnClientDetails, &QPushButton::clicked, this, &MainWindow::showClientDetails);

    stackedWidget->addWidget(clientWidget);
}

void MainWindow::setupCommandeSection()
{
    commandeWidget = new QWidget();
    commandeWidget->setStyleSheet("background: transparent;");
    commandeLayout = new QVBoxLayout(commandeWidget);
    commandeLayout->setSpacing(20);
    commandeLayout->setContentsMargins(0, 0, 0, 0);

    // Commande Controls Frame
    commandeControlFrame = new QFrame(this);
    commandeControlFrame->setStyleSheet(R"(
        QFrame {
            background-color: #1e1e2e;
            border-radius: 12px;
            padding: 15px;
            border: 1px solid #2a2a3a;
        }
    )");
    commandeButtonLayout = new QHBoxLayout(commandeControlFrame);

    btnAddCommande = new QPushButton("➕ Nouvelle Commande", this);
    btnEditCommande = new QPushButton("✏️ Modifier", this);
    btnDeleteCommande = new QPushButton("🗑️ Supprimer", this);
    btnRefreshCommandes = new QPushButton("🔄 Actualiser", this);
    btnExportPDF = new QPushButton("📄 PDF Ce Mois", this);

    applyModernButtonStyle(btnAddCommande, "#00d4aa");
    applyModernButtonStyle(btnEditCommande, "#2a7fff");
    applyModernButtonStyle(btnDeleteCommande, "#ff4757");
    applyModernButtonStyle(btnRefreshCommandes, "#6c757d");
    applyModernButtonStyle(btnExportPDF, "#ff6b35");

    commandeButtonLayout->addWidget(btnAddCommande);
    commandeButtonLayout->addWidget(btnEditCommande);
    commandeButtonLayout->addWidget(btnDeleteCommande);
    commandeButtonLayout->addStretch();
    commandeButtonLayout->addWidget(btnExportPDF);
    commandeButtonLayout->addWidget(btnRefreshCommandes);

    // Commande Search Group
    commandeSearchGroup = new QGroupBox("🔍 Filtres de Recherche Avancés", this);
    commandeSearchGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 16px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");

    commandeSearchLayout = new QHBoxLayout(commandeSearchGroup);

    QString filterStyle = R"(
        QLineEdit, QComboBox, QDateEdit {
            background-color: #2d2d3d;
            color: #ffffff;
            border: 2px solid #3d3d4d;
            border-radius: 8px;
            padding: 10px;
            font-size: 13px;
            min-width: 140px;
            selection-background-color: #2a7fff;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border-color: #2a7fff;
            background-color: #252535;
        }
    )";

    txtSearchCommande = new QLineEdit(this);
    txtSearchCommande->setPlaceholderText("Nom client...");
    txtSearchCommande->setStyleSheet(filterStyle);

    cmbStatutFilter = new QComboBox(this);
    cmbStatutFilter->addItem("📋 Tous les statuts", "");
    cmbStatutFilter->addItem("🟡 EN_COURS", "EN_COURS");
    cmbStatutFilter->addItem("🟢 LIVRE", "LIVRE");
    cmbStatutFilter->addItem("🔴 ANNULE", "ANNULE");
    cmbStatutFilter->setStyleSheet(filterStyle);

    dateFromFilter = new QDateEdit(this);
    dateFromFilter->setDate(QDate::currentDate().addDays(-30));
    dateFromFilter->setCalendarPopup(true);
    dateFromFilter->setStyleSheet(filterStyle);

    dateToFilter = new QDateEdit(this);
    dateToFilter->setDate(QDate::currentDate());
    dateToFilter->setCalendarPopup(true);
    dateToFilter->setStyleSheet(filterStyle);

    btnSearchCommande = new QPushButton("🔍 Rechercher", this);
    btnClearFilter = new QPushButton("🗑️ Effacer", this);

    applyModernButtonStyle(btnSearchCommande, "#2a7fff");
    applyModernButtonStyle(btnClearFilter, "#6c757d");

    // Create styled labels
    QLabel *labelClient = new QLabel("Client:");
    QLabel *labelStatut = new QLabel("Statut:");
    QLabel *labelFrom = new QLabel("Du:");
    QLabel *labelTo = new QLabel("Au:");

    QString labelStyle = "QLabel { color: #e0e0e0; font-weight: bold; font-size: 13px; }";
    labelClient->setStyleSheet(labelStyle);
    labelStatut->setStyleSheet(labelStyle);
    labelFrom->setStyleSheet(labelStyle);
    labelTo->setStyleSheet(labelStyle);

    commandeSearchLayout->addWidget(labelClient);
    commandeSearchLayout->addWidget(txtSearchCommande);
    commandeSearchLayout->addWidget(labelStatut);
    commandeSearchLayout->addWidget(cmbStatutFilter);
    commandeSearchLayout->addWidget(labelFrom);
    commandeSearchLayout->addWidget(dateFromFilter);
    commandeSearchLayout->addWidget(labelTo);
    commandeSearchLayout->addWidget(dateToFilter);
    commandeSearchLayout->addWidget(btnSearchCommande);
    commandeSearchLayout->addWidget(btnClearFilter);

    // Commandes Table Group
    commandeTableGroup = new QGroupBox("📦 Liste des Commandes", this);
    commandeTableGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 18px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");

    QVBoxLayout *commandeTableLayout = new QVBoxLayout(commandeTableGroup);
    commandesTable = new QTableWidget(this);
    commandesTable->setColumnCount(7);
    commandesTable->setHorizontalHeaderLabels({"ID", "Client", "Date", "Statut", "Montant", "Paiement", "Remarque"});
    applyModernTableStyle(commandesTable);
    commandeTableLayout->addWidget(commandesTable);

    // Commande Form Group (initially hidden)
    commandeFormGroup = new QGroupBox("📝 Formulaire Commande", this);
    commandeFormGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 18px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");
    commandeFormGroup->setVisible(false);

    commandeFormLayout = new QVBoxLayout(commandeFormGroup);

    QFormLayout *commandeForm = new QFormLayout();
    commandeForm->setLabelAlignment(Qt::AlignRight);
    commandeForm->setSpacing(15);

    QString formStyle = R"(
        QLineEdit, QTextEdit, QComboBox, QDateEdit {
            background-color: #2d2d3d;
            color: #ffffff;
            border: 2px solid #3d3d4d;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            min-width: 250px;
            selection-background-color: #2a7fff;
        }
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QDateEdit:focus {
            border-color: #2a7fff;
            background-color: #252535;
        }
        QTextEdit {
            min-height: 100px;
        }
    )";

    cmbClient = new QComboBox(this);
    dateCommande = new QDateEdit(this);
    dateCommande->setDate(QDate::currentDate());
    dateCommande->setCalendarPopup(true);

    cmbStatut = new QComboBox(this);
    cmbStatut->addItem("🟡 EN_COURS");
    cmbStatut->addItem("🟢 LIVRE");
    cmbStatut->addItem("🔴 ANNULE");

    txtMontant = new QLineEdit(this);

    cmbMoyenPaiement = new QComboBox(this);
    cmbMoyenPaiement->addItem("💳 Carte Bancaire");
    cmbMoyenPaiement->addItem("💵 Espèces");
    cmbMoyenPaiement->addItem("📄 Chèque");
    cmbMoyenPaiement->addItem("🏦 Virement");

    txtRemarque = new QTextEdit(this);

    // Apply styles
    cmbClient->setStyleSheet(formStyle);
    dateCommande->setStyleSheet(formStyle);
    cmbStatut->setStyleSheet(formStyle);
    txtMontant->setStyleSheet(formStyle);
    cmbMoyenPaiement->setStyleSheet(formStyle);
    txtRemarque->setStyleSheet(formStyle);

    // Create styled labels for commande form
    QLabel *labelFormClient = new QLabel("👤 Client *:");
    QLabel *labelFormDate = new QLabel("📅 Date Commande *:");
    QLabel *labelFormStatut = new QLabel("📊 Statut *:");
    QLabel *labelFormMontant = new QLabel("💰 Montant *:");
    QLabel *labelFormPaiement = new QLabel("💳 Moyen Paiement:");
    QLabel *labelFormRemarque = new QLabel("📝 Remarque:");

    labelFormClient->setStyleSheet(labelStyle);
    labelFormDate->setStyleSheet(labelStyle);
    labelFormStatut->setStyleSheet(labelStyle);
    labelFormMontant->setStyleSheet(labelStyle);
    labelFormPaiement->setStyleSheet(labelStyle);
    labelFormRemarque->setStyleSheet(labelStyle);

    commandeForm->addRow(labelFormClient, cmbClient);
    commandeForm->addRow(labelFormDate, dateCommande);
    commandeForm->addRow(labelFormStatut, cmbStatut);
    commandeForm->addRow(labelFormMontant, txtMontant);
    commandeForm->addRow(labelFormPaiement, cmbMoyenPaiement);
    commandeForm->addRow(labelFormRemarque, txtRemarque);

    commandeFormLayout->addLayout(commandeForm);

    QHBoxLayout *commandeFormButtons = new QHBoxLayout();
    btnSaveCommande = new QPushButton("💾 Sauvegarder", this);
    btnCancelCommande = new QPushButton("❌ Annuler", this);

    applyModernButtonStyle(btnSaveCommande, "#00d4aa");
    applyModernButtonStyle(btnCancelCommande, "#6c757d");

    commandeFormButtons->addWidget(btnSaveCommande);
    commandeFormButtons->addWidget(btnCancelCommande);
    commandeFormButtons->addStretch();

    commandeFormLayout->addLayout(commandeFormButtons);

    // Add all to commande layout
    commandeLayout->addWidget(commandeControlFrame);
    commandeLayout->addWidget(commandeSearchGroup);
    commandeLayout->addWidget(commandeTableGroup);
    commandeLayout->addWidget(commandeFormGroup);

    // Connect commande signals
    connect(btnAddCommande, &QPushButton::clicked, this, &MainWindow::addNewCommande);
    connect(btnEditCommande, &QPushButton::clicked, this, &MainWindow::editSelectedCommande);
    connect(btnDeleteCommande, &QPushButton::clicked, this, &MainWindow::deleteSelectedCommande);
    connect(btnSearchCommande, &QPushButton::clicked, this, &MainWindow::searchCommandes);
    connect(btnClearFilter, &QPushButton::clicked, this, [this]() {
        txtSearchCommande->clear();
        cmbStatutFilter->setCurrentIndex(0);
        dateFromFilter->setDate(QDate::currentDate().addDays(-30));
        dateToFilter->setDate(QDate::currentDate());
        searchCommandes();
    });
    connect(btnRefreshCommandes, &QPushButton::clicked, this, &MainWindow::loadCommandesTable);
    connect(btnSaveCommande, &QPushButton::clicked, this, &MainWindow::saveCommande);
    connect(btnCancelCommande, &QPushButton::clicked, this, &MainWindow::cancelCommandeEdit);
    connect(btnExportPDF, &QPushButton::clicked, this, &MainWindow::exportCommandesPDF);

    stackedWidget->addWidget(commandeWidget);
}

void MainWindow::setupStatisticsSection()
{
    statisticsWidget = new QWidget();
    statisticsWidget->setStyleSheet("background: transparent;");
    statisticsLayout = new QVBoxLayout(statisticsWidget);
    statisticsLayout->setSpacing(20);
    statisticsLayout->setContentsMargins(0, 0, 0, 0);

    // Statistics header
    QLabel *statsHeader = new QLabel("📊 Tableau de Bord - Analytics", this);
    statsHeader->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
            font-size: 24px;
            font-weight: bold;
            padding: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                      stop:0 #1a1a2e, stop:1 #16213e);
            border-radius: 12px;
            border: 1px solid #2a2a4a;
        }
    )");
    statsHeader->setAlignment(Qt::AlignCenter);

    // Stats summary
    statsSummary = new QLabel(this);
    statsSummary->setStyleSheet(R"(
        QLabel {
            color: #e0e0e0;
            font-size: 16px;
            padding: 20px;
            background-color: #1e1e2e;
            border-radius: 12px;
            border: 1px solid #2a2a3a;
        }
    )");
    statsSummary->setAlignment(Qt::AlignCenter);

    // Charts container
    QHBoxLayout *chartsLayout = new QHBoxLayout();

    // Orders chart
    QGroupBox *ordersChartGroup = new QGroupBox("📈 Commandes par Mois", this);
    ordersChartGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 16px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");

    QVBoxLayout *ordersChartLayout = new QVBoxLayout(ordersChartGroup);
    chartViewOrders = new QChartView(this);
    chartViewOrders->setStyleSheet("background: transparent; border: none;");
    chartViewOrders->setRenderHint(QPainter::Antialiasing);
    ordersChartLayout->addWidget(chartViewOrders);

    // Revenue chart
    QGroupBox *revenueChartGroup = new QGroupBox("💰 Chiffre d'Affaires par Mois", this);
    revenueChartGroup->setStyleSheet(R"(
        QGroupBox {
            color: #ffffff;
            font-weight: bold;
            font-size: 16px;
            border: 2px solid #2a2a3a;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #1a1a2e;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #ffffff;
            background-color: #1a1a2e;
        }
    )");

    QVBoxLayout *revenueChartLayout = new QVBoxLayout(revenueChartGroup);
    chartViewRevenue = new QChartView(this);
    chartViewRevenue->setStyleSheet("background: transparent; border: none;");
    chartViewRevenue->setRenderHint(QPainter::Antialiasing);
    revenueChartLayout->addWidget(chartViewRevenue);

    chartsLayout->addWidget(ordersChartGroup);
    chartsLayout->addWidget(revenueChartGroup);

    // Add all to statistics layout
    statisticsLayout->addWidget(statsHeader);
    statisticsLayout->addWidget(statsSummary);
    statisticsLayout->addLayout(chartsLayout);

    stackedWidget->addWidget(statisticsWidget);
}

void MainWindow::updateStatisticsCharts()
{
    int currentYear = QDate::currentDate().year();
    QSqlQuery stats = dbManager->ordersPerMonth(currentYear);

    // Prepare data
    QBarSet *ordersSet = new QBarSet("Commandes");
    QBarSet *revenueSet = new QBarSet("Chiffre d'Affaires (€)");

    QStringList months;
    months << "Jan" << "Fév" << "Mar" << "Avr" << "Mai" << "Jun"
           << "Jul" << "Aoû" << "Sep" << "Oct" << "Nov" << "Déc";

    QVector<int> ordersData(12, 0);
    QVector<double> revenueData(12, 0.0);

    int totalOrders = 0;
    double totalRevenue = 0.0;

    while (stats.next()) {
        int mois = stats.value("mois").toInt() - 1; // Convert to 0-based index
        int nbCommandes = stats.value("total").toInt();
        double ca = stats.value("chiffre").toDouble();

        if (mois >= 0 && mois < 12) {
            ordersData[mois] = nbCommandes;
            revenueData[mois] = ca;
            totalOrders += nbCommandes;
            totalRevenue += ca;
        }
    }

    // Populate barsets
    for (int i = 0; i < 12; ++i) {
        *ordersSet << ordersData[i];
        *revenueSet << revenueData[i];
    }

    // Style the barsets
    ordersSet->setColor(QColor(42, 127, 255));
    ordersSet->setBorderColor(QColor(26, 95, 204));
    revenueSet->setColor(QColor(0, 212, 170));
    revenueSet->setBorderColor(QColor(0, 184, 148));

    // Create orders chart
    QBarSeries *ordersSeries = new QBarSeries();
    ordersSeries->append(ordersSet);

    QChart *ordersChart = new QChart();
    ordersChart->addSeries(ordersSeries);
    ordersChart->setTitle("Évolution des Commandes - " + QString::number(currentYear));
    ordersChart->setAnimationOptions(QChart::SeriesAnimations);

    // Style orders chart
    ordersChart->setTheme(QChart::ChartThemeDark);
    ordersChart->setBackgroundBrush(QBrush(QColor(26, 26, 46)));
    ordersChart->setTitleBrush(QBrush(QColor(255, 255, 255)));
    ordersChart->legend()->setLabelColor(QColor(224, 224, 224));

    QBarCategoryAxis *axisXOrders = new QBarCategoryAxis();
    axisXOrders->append(months);
    axisXOrders->setLabelsColor(QColor(224, 224, 224));

    QValueAxis *axisYOrders = new QValueAxis();
    axisYOrders->setLabelsColor(QColor(224, 224, 224));

    ordersChart->addAxis(axisXOrders, Qt::AlignBottom);
    ordersChart->addAxis(axisYOrders, Qt::AlignLeft);
    ordersSeries->attachAxis(axisXOrders);
    ordersSeries->attachAxis(axisYOrders);

    chartViewOrders->setChart(ordersChart);

    // Create revenue chart
    QBarSeries *revenueSeries = new QBarSeries();
    revenueSeries->append(revenueSet);

    QChart *revenueChart = new QChart();
    revenueChart->addSeries(revenueSeries);
    revenueChart->setTitle("Chiffre d'Affaires - " + QString::number(currentYear));
    revenueChart->setAnimationOptions(QChart::SeriesAnimations);

    // Style revenue chart
    revenueChart->setTheme(QChart::ChartThemeDark);
    revenueChart->setBackgroundBrush(QBrush(QColor(26, 26, 46)));
    revenueChart->setTitleBrush(QBrush(QColor(255, 255, 255)));
    revenueChart->legend()->setLabelColor(QColor(224, 224, 224));

    QBarCategoryAxis *axisXRevenue = new QBarCategoryAxis();
    axisXRevenue->append(months);
    axisXRevenue->setLabelsColor(QColor(224, 224, 224));

    QValueAxis *axisYRevenue = new QValueAxis();
    axisYRevenue->setLabelsColor(QColor(224, 224, 224));

    revenueChart->addAxis(axisXRevenue, Qt::AlignBottom);
    revenueChart->addAxis(axisYRevenue, Qt::AlignLeft);
    revenueSeries->attachAxis(axisXRevenue);
    revenueSeries->attachAxis(axisYRevenue);

    chartViewRevenue->setChart(revenueChart);

    // Update summary
    QString summaryText = QString("📊 Résumé Annuel %1\n\n"
                                  "• Total Commandes: %2\n"
                                  "• Chiffre d'Affaires Total: %3 €\n"
                                  "• Moyenne par Commande: %4 €")
                              .arg(QString::number(currentYear),
                                   QString::number(totalOrders),
                                   QString::number(totalRevenue, 'f', 2),
                                   totalOrders > 0 ? QString::number(totalRevenue / totalOrders, 'f', 2) : "0.00");

    statsSummary->setText(summaryText);
}

void MainWindow::showClientSection()
{
    stackedWidget->setCurrentWidget(clientWidget);
    btnClients->setStyleSheet("QPushButton { background-color: #00b894; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
    btnCommandes->setStyleSheet("QPushButton { background-color: #2a7fff; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
    btnStatistics->setStyleSheet("QPushButton { background-color: #a55eea; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
}

void MainWindow::showCommandeSection()
{
    stackedWidget->setCurrentWidget(commandeWidget);
    btnCommandes->setStyleSheet("QPushButton { background-color: #1a6fe6; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
    btnClients->setStyleSheet("QPushButton { background-color: #00d4aa; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
    btnStatistics->setStyleSheet("QPushButton { background-color: #a55eea; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
}

void MainWindow::showStatisticsSection()
{
    updateStatisticsCharts();
    stackedWidget->setCurrentWidget(statisticsWidget);
    btnStatistics->setStyleSheet("QPushButton { background-color: #954dd6; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
    btnClients->setStyleSheet("QPushButton { background-color: #00d4aa; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
    btnCommandes->setStyleSheet("QPushButton { background-color: #2a7fff; color: white; border: none; padding: 10px 20px; border-radius: 8px; font-weight: 600; font-size: 13px; }");
}

// Client methods
void MainWindow::loadClientsTable()
{
    QSqlQuery query = dbManager->getClientsWithCommandCount();
    clientsTable->setRowCount(0);

    int row = 0;
    while (query.next()) {
        clientsTable->insertRow(row);
        clientsTable->setItem(row, 0, new QTableWidgetItem(query.value("id_client").toString()));
        clientsTable->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
        clientsTable->setItem(row, 2, new QTableWidgetItem(query.value("prenom").toString()));
        clientsTable->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        clientsTable->setItem(row, 4, new QTableWidgetItem(query.value("telephone").toString()));
        clientsTable->setItem(row, 5, new QTableWidgetItem(query.value("adresse").toString()));
        clientsTable->setItem(row, 6, new QTableWidgetItem(query.value("nb_commandes").toString()));
        row++;
    }
}

void MainWindow::addNewClient()
{
    clearClientForm();
    clientFormGroup->setVisible(true);
    isEditingClient = false;
    currentClientId = -1;
}

void MainWindow::editSelectedClient()
{
    QList<QTableWidgetItem*> selected = clientsTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client à modifier");
        return;
    }

    int row = selected.first()->row();
    currentClientId = clientsTable->item(row, 0)->text().toInt();

    QSqlRecord record;
    if (dbManager->getClient(currentClientId, record)) {
        populateClientForm(record);
        clientFormGroup->setVisible(true);
        isEditingClient = true;
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de charger les données du client");
    }
}

void MainWindow::deleteSelectedClient()
{
    QList<QTableWidgetItem*> selected = clientsTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client à supprimer");
        return;
    }

    int row = selected.first()->row();
    int clientId = clientsTable->item(row, 0)->text().toInt();
    QString clientName = clientsTable->item(row, 1)->text() + " " + clientsTable->item(row, 2)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              QString("Êtes-vous sûr de vouloir supprimer le client '%1' ?").arg(clientName),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteClient(clientId)) {
            QMessageBox::information(this, "Succès", "Client supprimé avec succès");
            loadClientsTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la suppression du client");
        }
    }
}

void MainWindow::searchClients()
{
    QString searchText = txtSearchClient->text().trimmed();
    QString filter = searchText.isEmpty() ? "" : "%" + searchText + "%";

    QString sql = "SELECT c.*, COUNT(co.id_commande) as nb_commandes FROM client c LEFT JOIN commande co ON c.id_client = co.id_client "
                  "WHERE c.nom LIKE ? OR c.prenom LIKE ? OR c.email LIKE ? "
                  "GROUP BY c.id_client, c.nom, c.prenom, c.email, c.telephone, c.adresse "
                  "ORDER BY c.nom, c.prenom";
    QSqlQuery query(dbManager->getDatabase());
    query.prepare(sql);
    query.addBindValue(filter.isEmpty() ? "%" : filter);
    query.addBindValue(filter.isEmpty() ? "%" : filter);
    query.addBindValue(filter.isEmpty() ? "%" : filter);

    if (query.exec()) {
        clientsTable->setRowCount(0);
        int row = 0;
        while (query.next()) {
            clientsTable->insertRow(row);
            clientsTable->setItem(row, 0, new QTableWidgetItem(query.value("id_client").toString()));
            clientsTable->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
            clientsTable->setItem(row, 2, new QTableWidgetItem(query.value("prenom").toString()));
            clientsTable->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
            clientsTable->setItem(row, 4, new QTableWidgetItem(query.value("telephone").toString()));
            clientsTable->setItem(row, 5, new QTableWidgetItem(query.value("adresse").toString()));
            clientsTable->setItem(row, 6, new QTableWidgetItem(query.value("nb_commandes").toString()));
            row++;
        }
    }
}

void MainWindow::saveClient()
{
    QString nom = txtClientNom->text().trimmed();
    QString prenom = txtClientPrenom->text().trimmed();
    QString email = txtClientEmail->text().trimmed();

    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Les champs Nom, Prénom et Email sont obligatoires");
        return;
    }

    bool success = false;
    qint64 newId;

    if (isEditingClient) {
        success = dbManager->updateClient(currentClientId, nom, prenom, email,
                                          txtClientTelephone->text().trimmed(),
                                          txtClientAdresse->toPlainText().trimmed());
    } else {
        success = dbManager->addClient(nom, prenom, email,
                                       txtClientTelephone->text().trimmed(),
                                       txtClientAdresse->toPlainText().trimmed(), newId);
    }

    if (success) {
        QMessageBox::information(this, "Succès", isEditingClient ? "Client modifié avec succès" : "Client ajouté avec succès");
        clientFormGroup->setVisible(false);
        loadClientsTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la sauvegarde du client");
    }
}

void MainWindow::cancelClientEdit()
{
    clientFormGroup->setVisible(false);
}

void MainWindow::clearClientForm()
{
    txtClientNom->clear();
    txtClientPrenom->clear();
    txtClientEmail->clear();
    txtClientTelephone->clear();
    txtClientAdresse->clear();
}

void MainWindow::populateClientForm(const QSqlRecord &record)
{
    txtClientNom->setText(record.value("nom").toString());
    txtClientPrenom->setText(record.value("prenom").toString());
    txtClientEmail->setText(record.value("email").toString());
    txtClientTelephone->setText(record.value("telephone").toString());
    txtClientAdresse->setText(record.value("adresse").toString());
}

// New client methods
void MainWindow::exportClientsPDF()
{
    QSqlQuery query = dbManager->getClientsWithCommandCount();

    if (!query.isActive()) {
        QMessageBox::critical(this, "Erreur", "Impossible de récupérer les clients: " + query.lastError().text());
        return;
    }

    // Ask for save location
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter PDF Clients",
                                                    "liste_clients.pdf",
                                                    "Fichiers PDF (*.pdf)");

    if (fileName.isEmpty()) {
        return;
    }

    // Create HTML content
    QString html;
    html += "<html><head><style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; }";
    html += "h1 { color: #2a7fff; text-align: center; }";
    html += "h2 { color: #333; border-bottom: 2px solid #2a7fff; padding-bottom: 5px; }";
    html += "table { width: 100%; border-collapse: collapse; margin: 20px 0; }";
    html += "th { background-color: #2a7fff; color: white; padding: 10px; text-align: left; }";
    html += "td { padding: 8px; border: 1px solid #ddd; }";
    html += "tr:nth-child(even) { background-color: #f2f2f2; }";
    html += ".summary { background-color: #e8f4ff; padding: 15px; border-radius: 5px; margin: 20px 0; }";
    html += ".total { font-weight: bold; color: #2a7fff; }";
    html += "</style></head><body>";

    // Header
    html += "<h1>Liste des Clients</h1>";
    html += "<p>Généré le: " + QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</p>";

    // Summary
    int totalClients = 0;
    int totalCommands = 0;

    // First pass to calculate totals
    QSqlQuery countQuery = dbManager->getClientsWithCommandCount();
    while (countQuery.next()) {
        totalClients++;
        totalCommands += countQuery.value("nb_commandes").toInt();
    }

    html += "<div class='summary'>";
    html += "<h2>Résumé</h2>";
    html += "<p>Total des clients: <span class='total'>" + QString::number(totalClients) + "</span></p>";
    html += "<p>Total des commandes: <span class='total'>" + QString::number(totalCommands) + "</span></p>";
    html += "<p>Moyenne par client: <span class='total'>" + (totalClients > 0 ? QString::number(static_cast<double>(totalCommands) / totalClients, 'f', 1) : "0") + "</span></p>";
    html += "</div>";

    // Table header
    html += "<h2>Détail des Clients</h2>";
    html += "<table>";
    html += "<tr>";
    html += "<th>ID</th>";
    html += "<th>Nom</th>";
    html += "<th>Prénom</th>";
    html += "<th>Email</th>";
    html += "<th>Téléphone</th>";
    html += "<th>Adresse</th>";
    html += "<th>Nb Commandes</th>";
    html += "</tr>";

    // Table rows
    while (query.next()) {
        html += "<tr>";
        html += "<td>" + query.value("id_client").toString() + "</td>";
        html += "<td>" + query.value("nom").toString() + "</td>";
        html += "<td>" + query.value("prenom").toString() + "</td>";
        html += "<td>" + query.value("email").toString() + "</td>";
        html += "<td>" + query.value("telephone").toString() + "</td>";
        html += "<td>" + query.value("adresse").toString() + "</td>";
        html += "<td>" + query.value("nb_commandes").toString() + "</td>";
        html += "</tr>";
    }

    html += "</table>";
    html += "</body></html>";

    // Generate PDF
    generatePDF(fileName, html);

    QMessageBox::information(this, "Succès",
                             QString("PDF généré avec succès!\n"
                                     "Clients exportés: %1\n"
                                     "Fichier: %2")
                                 .arg(QString::number(totalClients),
                                      fileName));
}

void MainWindow::showClientAnalytics()
{
    QList<QTableWidgetItem*> selected = clientsTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client");
        return;
    }

    int row = selected.first()->row();
    int clientId = clientsTable->item(row, 0)->text().toInt();
    QString clientName = clientsTable->item(row, 1)->text() + " " + clientsTable->item(row, 2)->text();

    int commandCount = dbManager->getClientCommandCount(clientId);
    double totalRevenue = dbManager->getTotalRevenueFromClient(clientId);
    double averageOrder = commandCount > 0 ? totalRevenue / commandCount : 0;

    QString analytics = QString("📊 Analytics Client: %1\n\n"
                                "• Nombre de commandes: %2\n"
                                "• Chiffre d'affaires total: %3 €\n"
                                "• Moyenne par commande: %4 €\n"
                                "• Client depuis: %5")
                            .arg(clientName,
                                 QString::number(commandCount),
                                 QString::number(totalRevenue, 'f', 2),
                                 QString::number(averageOrder, 'f', 2),
                                 "N/A"); // You could add creation date to client table

    QMessageBox::information(this, "Analytics Client", analytics);
}

void MainWindow::showClientDetails()
{
    QList<QTableWidgetItem*> selected = clientsTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client");
        return;
    }

    int row = selected.first()->row();
    int clientId = clientsTable->item(row, 0)->text().toInt();

    QSqlRecord record;
    if (dbManager->getClient(clientId, record)) {
        QString details = QString("👤 Détails Client\n\n"
                                  "ID: %1\n"
                                  "Nom: %2\n"
                                  "Prénom: %3\n"
                                  "Email: %4\n"
                                  "Téléphone: %5\n"
                                  "Adresse: %6")
                              .arg(record.value("id_client").toString(),
                                   record.value("nom").toString(),
                                   record.value("prenom").toString(),
                                   record.value("email").toString(),
                                   record.value("telephone").toString(),
                                   record.value("adresse").toString());

        QMessageBox::information(this, "Détails Client", details);
    }
}

// Commande methods
void MainWindow::loadCommandesTable()
{
    // Load clients for combobox
    cmbClient->clear();
    QSqlQuery clientQuery("SELECT id_client, nom, prenom FROM client ORDER BY nom, prenom", dbManager->getDatabase());
    while (clientQuery.next()) {
        QString clientInfo = QString("%1 %2 (ID: %3)").arg(clientQuery.value("prenom").toString(),
                                                           clientQuery.value("nom").toString(),
                                                           clientQuery.value("id_client").toString());
        cmbClient->addItem(clientInfo, clientQuery.value("id_client"));
    }

    // Load commandes
    searchCommandes();
}

void MainWindow::addNewCommande()
{
    clearCommandeForm();
    commandeFormGroup->setVisible(true);
    isEditingCommande = false;
    currentCommandeId = -1;
    dateCommande->setDate(QDate::currentDate());
}

void MainWindow::editSelectedCommande()
{
    QList<QTableWidgetItem*> selected = commandesTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une commande à modifier");
        return;
    }

    int row = selected.first()->row();
    currentCommandeId = commandesTable->item(row, 0)->text().toInt();

    QSqlRecord record;
    if (dbManager->getCommande(currentCommandeId, record)) {
        populateCommandeForm(record);
        commandeFormGroup->setVisible(true);
        isEditingCommande = true;
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de charger les données de la commande");
    }
}

void MainWindow::deleteSelectedCommande()
{
    QList<QTableWidgetItem*> selected = commandesTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une commande à supprimer");
        return;
    }

    int row = selected.first()->row();
    int commandeId = commandesTable->item(row, 0)->text().toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              "Êtes-vous sûr de vouloir supprimer cette commande ?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteCommande(commandeId)) {
            QMessageBox::information(this, "Succès", "Commande supprimée avec succès");
            loadCommandesTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la suppression de la commande");
        }
    }
}

void MainWindow::searchCommandes()
{
    QString clientFilter = txtSearchCommande->text().trimmed();
    QString statutFilter = cmbStatutFilter->currentData().toString();
    QDate fromDate = dateFromFilter->date();
    QDate toDate = dateToFilter->date();

    QString clientNameLike = clientFilter.isEmpty() ? "%" : "%" + clientFilter + "%";

    QSqlQuery query = dbManager->searchCommandes(clientNameLike, statutFilter, fromDate, toDate, "date_desc");

    commandesTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        commandesTable->insertRow(row);
        commandesTable->setItem(row, 0, new QTableWidgetItem(query.value("id_commande").toString()));
        commandesTable->setItem(row, 1, new QTableWidgetItem(
                                            query.value("prenom").toString() + " " + query.value("nom").toString()));
        commandesTable->setItem(row, 2, new QTableWidgetItem(
                                            query.value("date_commande").toDateTime().toString("dd/MM/yyyy hh:mm")));
        commandesTable->setItem(row, 3, new QTableWidgetItem(query.value("statut").toString()));
        commandesTable->setItem(row, 4, new QTableWidgetItem(
                                            QString::number(query.value("montant_total").toDouble(), 'f', 2) + " €"));
        // FIXED: Add moyen_paiement and remarque to the table
        commandesTable->setItem(row, 5, new QTableWidgetItem(query.value("moyen_paiement").toString()));
        commandesTable->setItem(row, 6, new QTableWidgetItem(query.value("remarque").toString()));
        row++;
    }
}

void MainWindow::exportCommandesPDF()
{
    // Get commands for current month
    QSqlQuery query = dbManager->getCommandesThisMonth();

    if (!query.isActive()) {
        QMessageBox::critical(this, "Erreur", "Impossible de récupérer les commandes du mois: " + query.lastError().text());
        return;
    }

    // Ask for save location
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter PDF",
                                                    QString("commandes_%1_%2.pdf")
                                                        .arg(QDate::currentDate().month())
                                                        .arg(QDate::currentDate().year()),
                                                    "Fichiers PDF (*.pdf)");

    if (fileName.isEmpty()) {
        return;
    }

    // Create HTML content
    QString html;
    html += "<html><head><style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; }";
    html += "h1 { color: #2a7fff; text-align: center; }";
    html += "h2 { color: #333; border-bottom: 2px solid #2a7fff; padding-bottom: 5px; }";
    html += "table { width: 100%; border-collapse: collapse; margin: 20px 0; }";
    html += "th { background-color: #2a7fff; color: white; padding: 10px; text-align: left; }";
    html += "td { padding: 8px; border: 1px solid #ddd; }";
    html += "tr:nth-child(even) { background-color: #f2f2f2; }";
    html += ".summary { background-color: #e8f4ff; padding: 15px; border-radius: 5px; margin: 20px 0; }";
    html += ".total { font-weight: bold; color: #2a7fff; }";
    html += "</style></head><body>";

    // Header
    html += "<h1>Rapport des Commandes - " + QDate::currentDate().toString("MMMM yyyy") + "</h1>";
    html += "<p>Généré le: " + QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</p>";

    // Summary
    int totalCommandes = 0;
    double totalMontant = 0.0;
    QMap<QString, int> statutsCount;

    // First pass to calculate totals
    QSqlQuery countQuery = dbManager->getCommandesThisMonth();
    while (countQuery.next()) {
        totalCommandes++;
        totalMontant += countQuery.value("montant_total").toDouble();
        QString statut = countQuery.value("statut").toString();
        statutsCount[statut]++;
    }

    html += "<div class='summary'>";
    html += "<h2>Résumé</h2>";
    html += "<p>Total des commandes: <span class='total'>" + QString::number(totalCommandes) + "</span></p>";
    html += "<p>Chiffre d'affaires total: <span class='total'>" + QString::number(totalMontant, 'f', 2) + " €</span></p>";
    html += "<p>Répartition par statut:</p><ul>";
    for (auto it = statutsCount.begin(); it != statutsCount.end(); ++it) {
        html += "<li>" + it.key() + ": " + QString::number(it.value()) + "</li>";
    }
    html += "</ul></div>";

    // Table header
    html += "<h2>Détail des Commandes</h2>";
    html += "<table>";
    html += "<tr>";
    html += "<th>ID</th>";
    html += "<th>Client</th>";
    html += "<th>Date</th>";
    html += "<th>Statut</th>";
    html += "<th>Montant</th>";
    html += "<th>Paiement</th>";
    html += "<th>Remarque</th>";
    html += "</tr>";

    // Table rows
    while (query.next()) {
        html += "<tr>";
        html += "<td>" + query.value("id_commande").toString() + "</td>";
        html += "<td>" + query.value("prenom").toString() + " " + query.value("nom").toString() + "</td>";
        html += "<td>" + query.value("date_commande").toDateTime().toString("dd/MM/yyyy HH:mm") + "</td>";
        html += "<td>" + query.value("statut").toString() + "</td>";
        html += "<td>" + QString::number(query.value("montant_total").toDouble(), 'f', 2) + " €</td>";
        html += "<td>" + query.value("moyen_paiement").toString() + "</td>";
        html += "<td>" + query.value("remarque").toString() + "</td>";
        html += "</tr>";
    }

    html += "</table>";
    html += "</body></html>";

    // Generate PDF
    generatePDF(fileName, html);

    QMessageBox::information(this, "Succès",
                             QString("PDF généré avec succès!\n"
                                     "Commandes exportées: %1\n"
                                     "Fichier: %2")
                                 .arg(QString::number(totalCommandes),
                                      fileName));
}

void MainWindow::generatePDF(const QString &fileName, const QString &htmlContent)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QTextDocument document;
    document.setHtml(htmlContent);
    document.print(&printer);
}

void MainWindow::showStatistics()
{
    showStatisticsSection();
}

void MainWindow::saveCommande()
{
    if (cmbClient->currentData().isNull()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client");
        return;
    }

    int clientId = cmbClient->currentData().toInt();
    QString statut = cmbStatut->currentText();
    bool ok;
    double montant = txtMontant->text().toDouble(&ok);

    if (!ok || montant <= 0) {
        QMessageBox::warning(this, "Attention", "Montant invalide");
        return;
    }

    bool success = false;
    qint64 newId;

    QDateTime dateTimeCommande = QDateTime(dateCommande->date(), QTime::currentTime());

    if (isEditingCommande) {
        success = dbManager->updateCommande(currentCommandeId, statut, montant,
                                            cmbMoyenPaiement->currentText(),
                                            txtRemarque->toPlainText().trimmed());
    } else {
        success = dbManager->addCommande(clientId, dateTimeCommande, statut, montant,
                                         cmbMoyenPaiement->currentText(),
                                         txtRemarque->toPlainText().trimmed(), newId);
    }

    if (success) {
        QMessageBox::information(this, "Succès", isEditingCommande ? "Commande modifiée avec succès" : "Commande ajoutée avec succès");
        commandeFormGroup->setVisible(false);
        loadCommandesTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la sauvegarde de la commande");
    }
}

void MainWindow::cancelCommandeEdit()
{
    commandeFormGroup->setVisible(false);
}

void MainWindow::clearCommandeForm()
{
    cmbStatut->setCurrentIndex(0);
    txtMontant->clear();
    cmbMoyenPaiement->setCurrentIndex(0);
    txtRemarque->clear();
}

void MainWindow::populateCommandeForm(const QSqlRecord &record)
{
    int clientId = record.value("id_client").toInt();

    // Find and select the client in combobox
    for (int i = 0; i < cmbClient->count(); ++i) {
        if (cmbClient->itemData(i).toInt() == clientId) {
            cmbClient->setCurrentIndex(i);
            break;
        }
    }

    dateCommande->setDate(record.value("date_commande").toDateTime().date());

    QString statut = record.value("statut").toString();
    int statutIndex = cmbStatut->findText(statut);
    if (statutIndex >= 0) cmbStatut->setCurrentIndex(statutIndex);

    txtMontant->setText(QString::number(record.value("montant_total").toDouble(), 'f', 2));

    QString paiement = record.value("moyen_paiement").toString();
    int paiementIndex = cmbMoyenPaiement->findText(paiement);
    if (paiementIndex >= 0) cmbMoyenPaiement->setCurrentIndex(paiementIndex);

    txtRemarque->setText(record.value("remarque").toString());
}
