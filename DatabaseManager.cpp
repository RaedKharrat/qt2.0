#include "DatabaseManager.h"
#include <QDebug>


DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    // Change to QODBC since QMYSQL driver is not available
    m_driver = "QODBC"; // Changed from "QMYSQL"

    m_db = QSqlDatabase::addDatabase(m_driver);

    if (m_driver == "QMYSQL") {
        m_db.setHostName("localhost");
        m_db.setDatabaseName("credit_db");
        m_db.setUserName("root");
        m_db.setPassword(""); // change if you set a password
    } else {
        // ODBC connection string - adjust based on your MySQL ODBC driver
        QString conn = "DRIVER={MySQL ODBC 8.0 Unicode Driver};SERVER=localhost;DATABASE=credit_db;USER=root;PASSWORD=;OPTION=3;";
        m_db.setDatabaseName(conn);
    }
}


DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open()
{
    if (!m_db.open()) {
        qCritical() << "DB open error:" << m_db.lastError().text();
        return false;
    }
    qDebug() << "DB opened. Drivers available:" << QSqlDatabase::drivers();
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

// ---- CLIENT ----
bool DatabaseManager::addClient(const QString &nom, const QString &prenom, const QString &email,
                                const QString &telephone, const QString &adresse, qint64 &outId)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO client (nom, prenom, email, telephone, adresse) "
              "VALUES (:nom, :prenom, :email, :telephone, :adresse)");
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":email", email);
    q.bindValue(":telephone", telephone);
    q.bindValue(":adresse", adresse);

    if (!q.exec()) {
        qWarning() << "addClient failed:" << q.lastError().text();
        return false;
    }
    QVariant id = q.lastInsertId();
    outId = id.isValid() ? id.toLongLong() : -1;
    return true;
}

bool DatabaseManager::getClient(int id, QSqlRecord &outRecord)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM client WHERE id_client = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "getClient exec failed:" << q.lastError().text();
        return false;
    }
    if (q.next()) {
        outRecord = q.record();
        return true;
    }
    return false;
}

bool DatabaseManager::updateClient(int id, const QString &nom, const QString &prenom, const QString &email,
                                   const QString &telephone, const QString &adresse)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE client SET nom=:nom, prenom=:prenom, email=:email, telephone=:telephone, adresse=:adresse WHERE id_client=:id");
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":email", email);
    q.bindValue(":telephone", telephone);
    q.bindValue(":adresse", adresse);
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "updateClient failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool DatabaseManager::deleteClient(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM client WHERE id_client = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "deleteClient failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// ---- COMMANDE ----
bool DatabaseManager::addCommande(int idClient, const QDateTime &dateCommande, const QString &statut,
                                  double montantTotal, const QString &moyenPaiement, const QString &remarque, qint64 &outId)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO commande (id_client, date_commande, statut, montant_total, moyen_paiement, remarque) "
              "VALUES (:id_client, :date_commande, :statut, :montant_total, :moyen_paiement, :remarque)");
    q.bindValue(":id_client", idClient);
    q.bindValue(":date_commande", dateCommande);
    q.bindValue(":statut", statut);
    q.bindValue(":montant_total", montantTotal);
    q.bindValue(":moyen_paiement", moyenPaiement);
    q.bindValue(":remarque", remarque);

    if (!q.exec()) {
        qWarning() << "addCommande failed:" << q.lastError().text();
        return false;
    }
    QVariant id = q.lastInsertId();
    outId = id.isValid() ? id.toLongLong() : -1;
    return true;
}

bool DatabaseManager::getCommande(int id, QSqlRecord &outRecord)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM commande WHERE id_commande = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "getCommande exec failed:" << q.lastError().text();
        return false;
    }
    if (q.next()) {
        outRecord = q.record();
        return true;
    }
    return false;
}

bool DatabaseManager::updateCommande(int id, const QString &statut, double montantTotal, const QString &moyenPaiement, const QString &remarque)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE commande SET statut=:statut, montant_total=:montant_total, moyen_paiement=:moyen_paiement, remarque=:remarque WHERE id_commande=:id");
    q.bindValue(":statut", statut);
    q.bindValue(":montant_total", montantTotal);
    q.bindValue(":moyen_paiement", moyenPaiement);
    q.bindValue(":remarque", remarque);
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "updateCommande failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool DatabaseManager::deleteCommande(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM commande WHERE id_commande = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "deleteCommande failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// ---- Recherche/tri multicritères ----
// clientNameLike: substring (ex: "%ali%")
// statut: exact match or empty QString() to ignore
// fromDate/toDate: if invalid(), ignored
QSqlQuery DatabaseManager::searchCommandes(const QString &clientNameLike,
                                           const QString &statut,
                                           const QDate &fromDate,
                                           const QDate &toDate,
                                           const QString &orderBy)
{
    QSqlQuery q(m_db);
    QString sql =
        "SELECT c.id_client, c.nom, c.prenom, co.id_commande, co.date_commande, co.statut, co.montant_total "
        "FROM client c JOIN commande co ON c.id_client = co.id_client "
        "WHERE (:name IS NULL OR c.nom LIKE :name) "
        "AND (:statut IS NULL OR co.statut = :statut) "
        "AND (:fromDate IS NULL OR co.date_commande >= :fromDate) "
        "AND (:toDate IS NULL OR co.date_commande <= :toDate)";

    if (orderBy == "date_desc") sql += " ORDER BY co.date_commande DESC";
    else if (orderBy == "montant_desc") sql += " ORDER BY co.montant_total DESC";
    else sql += " ORDER BY co.date_commande ASC";

    q.prepare(sql);

    if (clientNameLike.isEmpty())
        q.bindValue(":name", QVariant()); // NULL
    else
        q.bindValue(":name", clientNameLike);

    if (statut.isEmpty())
        q.bindValue(":statut", QVariant());
    else
        q.bindValue(":statut", statut);

    if (!fromDate.isValid())
        q.bindValue(":fromDate", QVariant());
    else
        q.bindValue(":fromDate", QDateTime(fromDate, QTime(0, 0)));

    if (!toDate.isValid())
        q.bindValue(":toDate", QVariant());
    else
        q.bindValue(":toDate", QDateTime(toDate, QTime(23, 59, 59)));

    if (!q.exec()) qWarning() << "searchCommandes failed:" << q.lastError().text();
    return q;
}

QSqlQuery DatabaseManager::ordersPerMonth(int year)
{
    QSqlQuery q(m_db);
    QString sql;
    if (m_driver == "QMYSQL" || m_driver == "QODBC") {
        sql = "SELECT MONTH(date_commande) AS mois, COUNT(*) AS total, SUM(montant_total) AS chiffre "
              "FROM commande WHERE YEAR(date_commande) = :year GROUP BY MONTH(date_commande) ORDER BY mois";
    } else {
        // generic fallback: Oracle would need EXTRACT(MONTH FROM date_commande)
        sql = "SELECT EXTRACT(MONTH FROM date_commande) AS mois, COUNT(*) AS total, SUM(montant_total) AS chiffre "
              "FROM commande WHERE EXTRACT(YEAR FROM date_commande) = :year GROUP BY EXTRACT(MONTH FROM date_commande) ORDER BY mois";
    }
    q.prepare(sql);
    q.bindValue(":year", year);
    if (!q.exec()) qWarning() << "ordersPerMonth failed:" << q.lastError().text();
    return q;
}
