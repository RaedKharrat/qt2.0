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
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>
#include <QSpacerItem>

// QtCharts includes
#include <QtCharts>
#include <QChartView>
#include <QBarSet>
#include <QBarSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>

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
    void showStatisticsSection();

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
    void setupStatisticsSection();
    void clearClientForm();
    void clearCommandeForm();
    void populateClientForm(const QSqlRecord &record);
    void populateCommandeForm(const QSqlRecord &record);
    void applyModernTableStyle(QTableWidget *table);
    void applyModernButtonStyle(QPushButton *button, const QString &color = "#0078D4");
    void updateStatisticsCharts();

    // Main widgets
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QStackedWidget *stackedWidget;

    // Header
    QFrame *headerFrame;
    QLabel *headerTitle;

    // Navigation buttons
    QPushButton *btnClients;
    QPushButton *btnCommandes;
    QPushButton *btnStatistics;

    // Client section widgets
    QWidget *clientWidget;
    QVBoxLayout *clientLayout;

    // Client controls
    QFrame *clientControlFrame;
    QHBoxLayout *clientButtonLayout;
    QPushButton *btnAddClient;
    QPushButton *btnEditClient;
    QPushButton *btnDeleteClient;
    QPushButton *btnRefreshClients;

    // Client search
    QFrame *clientSearchFrame;
    QHBoxLayout *clientSearchLayout;
    QLineEdit *txtSearchClient;
    QPushButton *btnSearchClient;

    // Clients table
    QGroupBox *clientTableGroup;
    QTableWidget *clientsTable;

    // Client form widgets
    QGroupBox *clientFormGroup;
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

    // Commande controls
    QFrame *commandeControlFrame;
    QHBoxLayout *commandeButtonLayout;
    QPushButton *btnAddCommande;
    QPushButton *btnEditCommande;
    QPushButton *btnDeleteCommande;
    QPushButton *btnRefreshCommandes;

    // Commande search
    QGroupBox *commandeSearchGroup;
    QHBoxLayout *commandeSearchLayout;
    QLineEdit *txtSearchCommande;
    QComboBox *cmbStatutFilter;
    QDateEdit *dateFromFilter;
    QDateEdit *dateToFilter;
    QPushButton *btnSearchCommande;
    QPushButton *btnClearFilter;

    // Commandes table
    QGroupBox *commandeTableGroup;
    QTableWidget *commandesTable;

    // Commande form widgets
    QGroupBox *commandeFormGroup;
    QVBoxLayout *commandeFormLayout;
    QComboBox *cmbClient;
    QDateEdit *dateCommande;
    QComboBox *cmbStatut;
    QLineEdit *txtMontant;
    QComboBox *cmbMoyenPaiement;
    QTextEdit *txtRemarque;
    QPushButton *btnSaveCommande;
    QPushButton *btnCancelCommande;

    // Statistics section
    QWidget *statisticsWidget;
    QVBoxLayout *statisticsLayout;
    QChartView *chartViewOrders;
    QChartView *chartViewRevenue;
    QLabel *statsSummary;

    DatabaseManager *dbManager;
    int currentClientId;
    int currentCommandeId;
    bool isEditingClient;
    bool isEditingCommande;
};

#endif // MAINWINDOW_H
