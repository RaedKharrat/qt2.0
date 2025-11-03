#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QtSql>
#include <QDateTime>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool open();
    void close();

    // Add this method to get database connection
    QSqlDatabase getDatabase() const { return m_db; }

    // CLIENT CRUD
    bool addClient(const QString &nom, const QString &prenom, const QString &email,
                   const QString &telephone, const QString &adresse, qint64 &outId);
    bool getClient(int id, QSqlRecord &outRecord);
    bool updateClient(int id, const QString &nom, const QString &prenom, const QString &email,
                      const QString &telephone, const QString &adresse);
    bool deleteClient(int id);

    // New client methods
    QSqlQuery getClientsWithCommandCount();
    double getTotalRevenueFromClient(int clientId);
    int getClientCommandCount(int clientId);

    // COMMANDE CRUD
    bool addCommande(int idClient, const QDateTime &dateCommande, const QString &statut,
                     double montantTotal, const QString &moyenPaiement, const QString &remarque, qint64 &outId);
    bool getCommande(int id, QSqlRecord &outRecord);
    bool updateCommande(int id, const QString &statut, double montantTotal, const QString &moyenPaiement, const QString &remarque);
    bool deleteCommande(int id);

    // recherche / tri exemple (3 critères)
    QSqlQuery searchCommandes(const QString &clientNameLike,
                              const QString &statut,
                              const QDate &fromDate,
                              const QDate &toDate,
                              const QString &orderBy);

    // statistique: commandes par mois
    QSqlQuery ordersPerMonth(int year);

    // Get commands for current month for PDF export
    QSqlQuery getCommandesThisMonth();

private:
    QSqlDatabase m_db;
    QString m_driver; // "QMYSQL" or "QODBC"
};

#endif // DATABASEMANAGER_H
