#include "mainwindow.h"
#include "DatabaseManager.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDebug>
#include <QHeaderView>
#include <QFormLayout>

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
}

MainWindow::~MainWindow()
{
    // dbManager is deleted automatically as child
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // Header with section buttons
    headerLayout = new QHBoxLayout();
    btnClients = new QPushButton("Gestion Clients", this);
    btnCommandes = new QPushButton("Gestion Commandes", this);

    btnClients->setStyleSheet("QPushButton { font-size: 14px; padding: 10px; }");
    btnCommandes->setStyleSheet("QPushButton { font-size: 14px; padding: 10px; }");

    headerLayout->addWidget(btnClients);
    headerLayout->addWidget(btnCommandes);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // Stacked widget for sections
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    setupClientSection();
    setupCommandeSection();

    // Connect header buttons
    connect(btnClients, &QPushButton::clicked, this, &MainWindow::showClientSection);
    connect(btnCommandes, &QPushButton::clicked, this, &MainWindow::showCommandeSection);

    // Start with client section
    showClientSection();
}

void MainWindow::setupClientSection()
{
    clientWidget = new QWidget();
    clientLayout = new QVBoxLayout(clientWidget);

    // Client buttons
    clientButtonLayout = new QHBoxLayout();
    btnAddClient = new QPushButton("Nouveau Client", this);
    btnEditClient = new QPushButton("Modifier", this);
    btnDeleteClient = new QPushButton("Supprimer", this);
    btnSearchClient = new QPushButton("Rechercher", this);
    txtSearchClient = new QLineEdit(this);
    txtSearchClient->setPlaceholderText("Rechercher par nom...");

    clientButtonLayout->addWidget(btnAddClient);
    clientButtonLayout->addWidget(btnEditClient);
    clientButtonLayout->addWidget(btnDeleteClient);
    clientButtonLayout->addStretch();
    clientButtonLayout->addWidget(txtSearchClient);
    clientButtonLayout->addWidget(btnSearchClient);

    clientLayout->addLayout(clientButtonLayout);

    // Clients table
    clientsTable = new QTableWidget(this);
    clientsTable->setColumnCount(6);
    clientsTable->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Email", "Téléphone", "Adresse"});
    clientsTable->horizontalHeader()->setStretchLastSection(true);
    clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    clientLayout->addWidget(clientsTable);

    // Client form (initially hidden)
    clientFormWidget = new QWidget(this);
    clientFormLayout = new QVBoxLayout(clientFormWidget);

    QFormLayout *formLayout = new QFormLayout();
    txtClientNom = new QLineEdit(this);
    txtClientPrenom = new QLineEdit(this);
    txtClientEmail = new QLineEdit(this);
    txtClientTelephone = new QLineEdit(this);
    txtClientAdresse = new QTextEdit(this);
    txtClientAdresse->setMaximumHeight(80);

    formLayout->addRow("Nom *:", txtClientNom);
    formLayout->addRow("Prénom *:", txtClientPrenom);
    formLayout->addRow("Email *:", txtClientEmail);
    formLayout->addRow("Téléphone:", txtClientTelephone);
    formLayout->addRow("Adresse:", txtClientAdresse);

    clientFormLayout->addLayout(formLayout);

    QHBoxLayout *clientFormButtons = new QHBoxLayout();
    btnSaveClient = new QPushButton("Sauvegarder", this);
    btnCancelClient = new QPushButton("Annuler", this);
    clientFormButtons->addWidget(btnSaveClient);
    clientFormButtons->addWidget(btnCancelClient);
    clientFormButtons->addStretch();

    clientFormLayout->addLayout(clientFormButtons);
    clientFormWidget->setVisible(false);
    clientLayout->addWidget(clientFormWidget);

    // Connect client signals
    connect(btnAddClient, &QPushButton::clicked, this, &MainWindow::addNewClient);
    connect(btnEditClient, &QPushButton::clicked, this, &MainWindow::editSelectedClient);
    connect(btnDeleteClient, &QPushButton::clicked, this, &MainWindow::deleteSelectedClient);
    connect(btnSearchClient, &QPushButton::clicked, this, &MainWindow::searchClients);
    connect(btnSaveClient, &QPushButton::clicked, this, &MainWindow::saveClient);
    connect(btnCancelClient, &QPushButton::clicked, this, &MainWindow::cancelClientEdit);

    stackedWidget->addWidget(clientWidget);
}

void MainWindow::setupCommandeSection()
{
    commandeWidget = new QWidget();
    commandeLayout = new QVBoxLayout(commandeWidget);

    // Commande buttons and filters
    commandeButtonLayout = new QHBoxLayout();
    btnAddCommande = new QPushButton("Nouvelle Commande", this);
    btnEditCommande = new QPushButton("Modifier", this);
    btnDeleteCommande = new QPushButton("Supprimer", this);
    btnStatsCommande = new QPushButton("Statistiques", this);
    btnSearchCommande = new QPushButton("Rechercher", this);

    txtSearchCommande = new QLineEdit(this);
    txtSearchCommande->setPlaceholderText("Nom client...");

    cmbStatutFilter = new QComboBox(this);
    cmbStatutFilter->addItem("Tous les statuts", "");
    cmbStatutFilter->addItem("EN_COURS", "EN_COURS");
    cmbStatutFilter->addItem("LIVRE", "LIVRE");
    cmbStatutFilter->addItem("ANNULE", "ANNULE");

    dateFromFilter = new QDateEdit(this);
    dateFromFilter->setDate(QDate::currentDate().addDays(-30));
    dateFromFilter->setCalendarPopup(true);

    dateToFilter = new QDateEdit(this);
    dateToFilter->setDate(QDate::currentDate());
    dateToFilter->setCalendarPopup(true);

    commandeButtonLayout->addWidget(btnAddCommande);
    commandeButtonLayout->addWidget(btnEditCommande);
    commandeButtonLayout->addWidget(btnDeleteCommande);
    commandeButtonLayout->addWidget(btnStatsCommande);
    commandeButtonLayout->addStretch();
    commandeButtonLayout->addWidget(new QLabel("Client:", this));
    commandeButtonLayout->addWidget(txtSearchCommande);
    commandeButtonLayout->addWidget(new QLabel("Statut:", this));
    commandeButtonLayout->addWidget(cmbStatutFilter);
    commandeButtonLayout->addWidget(new QLabel("Du:", this));
    commandeButtonLayout->addWidget(dateFromFilter);
    commandeButtonLayout->addWidget(new QLabel("Au:", this));
    commandeButtonLayout->addWidget(dateToFilter);
    commandeButtonLayout->addWidget(btnSearchCommande);

    commandeLayout->addLayout(commandeButtonLayout);

    // Commandes table
    commandesTable = new QTableWidget(this);
    commandesTable->setColumnCount(7);
    commandesTable->setHorizontalHeaderLabels({"ID", "Client", "Date", "Statut", "Montant", "Paiement", "Remarque"});
    commandesTable->horizontalHeader()->setStretchLastSection(true);
    commandesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    commandesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    commandeLayout->addWidget(commandesTable);

    // Commande form (initially hidden)
    commandeFormWidget = new QWidget(this);
    commandeFormLayout = new QVBoxLayout(commandeFormWidget);

    QFormLayout *commandeForm = new QFormLayout();
    cmbClient = new QComboBox(this);
    dateCommande = new QDateEdit(this);
    dateCommande->setDate(QDate::currentDate());
    dateCommande->setCalendarPopup(true);

    cmbStatut = new QComboBox(this);
    cmbStatut->addItem("EN_COURS");
    cmbStatut->addItem("LIVRE");
    cmbStatut->addItem("ANNULE");

    txtMontant = new QLineEdit(this);

    cmbMoyenPaiement = new QComboBox(this);
    cmbMoyenPaiement->addItem("Carte Bancaire");
    cmbMoyenPaiement->addItem("Espèces");
    cmbMoyenPaiement->addItem("Chèque");
    cmbMoyenPaiement->addItem("Virement");

    txtRemarque = new QTextEdit(this);
    txtRemarque->setMaximumHeight(80);

    commandeForm->addRow("Client *:", cmbClient);
    commandeForm->addRow("Date Commande *:", dateCommande);
    commandeForm->addRow("Statut *:", cmbStatut);
    commandeForm->addRow("Montant *:", txtMontant);
    commandeForm->addRow("Moyen Paiement:", cmbMoyenPaiement);
    commandeForm->addRow("Remarque:", txtRemarque);

    commandeFormLayout->addLayout(commandeForm);

    QHBoxLayout *commandeFormButtons = new QHBoxLayout();
    btnSaveCommande = new QPushButton("Sauvegarder", this);
    btnCancelCommande = new QPushButton("Annuler", this);
    commandeFormButtons->addWidget(btnSaveCommande);
    commandeFormButtons->addWidget(btnCancelCommande);
    commandeFormButtons->addStretch();

    commandeFormLayout->addLayout(commandeFormButtons);
    commandeFormWidget->setVisible(false);
    commandeLayout->addWidget(commandeFormWidget);

    // Connect commande signals
    connect(btnAddCommande, &QPushButton::clicked, this, &MainWindow::addNewCommande);
    connect(btnEditCommande, &QPushButton::clicked, this, &MainWindow::editSelectedCommande);
    connect(btnDeleteCommande, &QPushButton::clicked, this, &MainWindow::deleteSelectedCommande);
    connect(btnSearchCommande, &QPushButton::clicked, this, &MainWindow::searchCommandes);
    connect(btnStatsCommande, &QPushButton::clicked, this, &MainWindow::showStatistics);
    connect(btnSaveCommande, &QPushButton::clicked, this, &MainWindow::saveCommande);
    connect(btnCancelCommande, &QPushButton::clicked, this, &MainWindow::cancelCommandeEdit);

    stackedWidget->addWidget(commandeWidget);
}

void MainWindow::showClientSection()
{
    stackedWidget->setCurrentWidget(clientWidget);
    btnClients->setStyleSheet("QPushButton { font-size: 14px; padding: 10px; background-color: #0078D4; color: white; }");
    btnCommandes->setStyleSheet("QPushButton { font-size: 14px; padding: 10px; }");
}

void MainWindow::showCommandeSection()
{
    stackedWidget->setCurrentWidget(commandeWidget);
    btnCommandes->setStyleSheet("QPushButton { font-size: 14px; padding: 10px; background-color: #0078D4; color: white; }");
    btnClients->setStyleSheet("QPushButton { font-size: 14px; padding: 10px; }");
}

// Client methods
void MainWindow::loadClientsTable()
{
    QSqlQuery query("SELECT * FROM client ORDER BY nom, prenom", dbManager->getDatabase());
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
        row++;
    }
}

void MainWindow::addNewClient()
{
    clearClientForm();
    clientFormWidget->setVisible(true);
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
        clientFormWidget->setVisible(true);
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

    QString sql = "SELECT * FROM client WHERE nom LIKE ? OR prenom LIKE ? OR email LIKE ? ORDER BY nom, prenom";
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
        clientFormWidget->setVisible(false);
        loadClientsTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la sauvegarde du client");
    }
}

void MainWindow::cancelClientEdit()
{
    clientFormWidget->setVisible(false);
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
    commandeFormWidget->setVisible(true);
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
        commandeFormWidget->setVisible(true);
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
        // Note: moyen_paiement and remarque are not in the search results, you might need to adjust your searchCommandes method
        commandesTable->setItem(row, 5, new QTableWidgetItem("")); // Placeholder for moyen_paiement
        commandesTable->setItem(row, 6, new QTableWidgetItem("")); // Placeholder for remarque
        row++;
    }
}

void MainWindow::showStatistics()
{
    int currentYear = QDate::currentDate().year();
    QSqlQuery stats = dbManager->ordersPerMonth(currentYear);

    QString statsText = QString("Statistiques des commandes - Année %1\n\n").arg(currentYear);
    double totalCA = 0;
    int totalCommandes = 0;

    while (stats.next()) {
        int mois = stats.value("mois").toInt();
        int nbCommandes = stats.value("total").toInt();
        double ca = stats.value("chiffre").toDouble();

        statsText += QString("Mois %1: %2 commandes, Chiffre d'affaires: %3 €\n")
                         .arg(mois).arg(nbCommandes).arg(ca, 0, 'f', 2);

        totalCommandes += nbCommandes;
        totalCA += ca;
    }

    statsText += QString("\nTotal année: %1 commandes, Chiffre d'affaires total: %2 €")
                     .arg(totalCommandes).arg(totalCA, 0, 'f', 2);

    QMessageBox::information(this, "Statistiques", statsText);
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
        commandeFormWidget->setVisible(false);
        loadCommandesTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la sauvegarde de la commande");
    }
}

void MainWindow::cancelCommandeEdit()
{
    commandeFormWidget->setVisible(false);
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
