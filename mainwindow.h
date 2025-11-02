#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QFormLayout>
#include <QSqlRecord>

// Forward declaration
class DatabaseManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showClientSection();
    void showCommandeSection();

    // Client slots
    void addNewClient();
    void searchClients();
    void loadClientsTable();
    void editSelectedClient();
    void deleteSelectedClient();
    void saveClient();
    void cancelClientEdit();

    // Commande slots
    void addNewCommande();
    void searchCommandes();
    void loadCommandesTable();
    void editSelectedCommande();
    void deleteSelectedCommande();
    void saveCommande();
    void cancelCommandeEdit();
    void showStatistics();

private:
    void setupUI();
    void setupClientSection();
    void setupCommandeSection();
    void clearClientForm();
    void clearCommandeForm();
    void populateClientForm(const QSqlRecord &record);
    void populateCommandeForm(const QSqlRecord &record);

    // Main widgets
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QStackedWidget *stackedWidget;

    // Header buttons
    QPushButton *btnClients;
    QPushButton *btnCommandes;

    // Client section widgets
    QWidget *clientWidget;
    QVBoxLayout *clientLayout;
    QHBoxLayout *clientButtonLayout;
    QPushButton *btnAddClient;
    QPushButton *btnEditClient;
    QPushButton *btnDeleteClient;
    QPushButton *btnSearchClient;
    QLineEdit *txtSearchClient;
    QTableWidget *clientsTable;

    // Client form widgets
    QWidget *clientFormWidget;
    QVBoxLayout *clientFormLayout;
    QLineEdit *txtClientNom;
    QLineEdit *txtClientPrenom;
    QLineEdit *txtClientEmail;
    QLineEdit *txtClientTelephone;
    QTextEdit *txtClientAdresse;
    QPushButton *btnSaveClient;
    QPushButton *btnCancelClient;

    // Commande section widgets
    QWidget *commandeWidget;
    QVBoxLayout *commandeLayout;
    QHBoxLayout *commandeButtonLayout;
    QPushButton *btnAddCommande;
    QPushButton *btnEditCommande;
    QPushButton *btnDeleteCommande;
    QPushButton *btnSearchCommande;
    QPushButton *btnStatsCommande;
    QLineEdit *txtSearchCommande;
    QComboBox *cmbStatutFilter;
    QDateEdit *dateFromFilter;
    QDateEdit *dateToFilter;
    QTableWidget *commandesTable;

    // Commande form widgets
    QWidget *commandeFormWidget;
    QVBoxLayout *commandeFormLayout;
    QComboBox *cmbClient;
    QDateEdit *dateCommande;
    QComboBox *cmbStatut;
    QLineEdit *txtMontant;
    QComboBox *cmbMoyenPaiement;
    QTextEdit *txtRemarque;
    QPushButton *btnSaveCommande;
    QPushButton *btnCancelCommande;

    DatabaseManager *dbManager;
    int currentClientId;
    int currentCommandeId;
    bool isEditingClient;
    bool isEditingCommande;
};

#endif // MAINWINDOW_H
