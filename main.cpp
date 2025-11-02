#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DatabaseManager db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, "Erreur", "Impossible d'ouvrir la base de données. Voir la console pour l'erreur.");
        return -1;
    }

    // Test: ajouter un client avec des données différentes
    qint64 clientId;
    QString uniqueEmail = "client." + QString::number(QDateTime::currentMSecsSinceEpoch()) + "@example.com";

    if (db.addClient("Dupont", "Marie", uniqueEmail, "01234567", "123 Avenue des Champs", clientId)) {
        qDebug() << "Client ajouté id =" << clientId;

        // Test: ajouter une commande pour ce client
        qint64 commandeId;
        if (db.addCommande((int)clientId, QDateTime::currentDateTime(), "EN_COURS", 299.99, "Carte Bancaire", "Commande de test", commandeId)) {
            qDebug() << "Commande ajoutée id =" << commandeId;
        } else {
            qDebug() << "Échec ajout commande";
        }
    } else {
        qDebug() << "Échec ajout client - essai avec un autre email";

        // Try with another unique email
        uniqueEmail = "test." + QString::number(QDateTime::currentMSecsSinceEpoch()) + "@domain.com";
        if (db.addClient("Martin", "Pierre", uniqueEmail, "06543210", "456 Boulevard Voltaire", clientId)) {
            qDebug() << "Client ajouté id =" << clientId;

            // Test: ajouter une commande pour ce client
            qint64 commandeId;
            if (db.addCommande((int)clientId, QDateTime::currentDateTime(), "LIVRE", 450.75, "Espèces", "Deuxième commande test", commandeId)) {
                qDebug() << "Commande ajoutée id =" << commandeId;
            } else {
                qDebug() << "Échec ajout commande";
            }
        }
    }

    // Test: recherche de tous les clients
    qDebug() << "\n=== Recherche de toutes les commandes ===";
    QSqlQuery q = db.searchCommandes("%", "", QDate(), QDate(), "date_desc");
    int count = 0;
    while (q.next()) {
        count++;
        qDebug() << "Commande" << count << ":"
                 << "ID:" << q.value("id_commande").toInt()
                 << "Client:" << q.value("prenom").toString() << q.value("nom").toString()
                 << "Date:" << q.value("date_commande").toDateTime().toString("dd/MM/yyyy hh:mm")
                 << "Montant:" << q.value("montant_total").toDouble()
                 << "Statut:" << q.value("statut").toString();
    }

    if (count == 0) {
        qDebug() << "Aucune commande trouvée dans la base de données";
    }

    // Test: statistiques par mois pour l'année en cours
    qDebug() << "\n=== Statistiques commandes par mois ===";
    int currentYear = QDate::currentDate().year();
    QSqlQuery stats = db.ordersPerMonth(currentYear);
    while (stats.next()) {
        qDebug() << "Mois" << stats.value("mois").toInt()
        << ":" << stats.value("total").toInt() << "commandes,"
        << "chiffre d'affaires:" << stats.value("chiffre").toDouble() << "€";
    }

    // Test: recherche avec critères spécifiques
    qDebug() << "\n=== Recherche avec critères ===";
    QDate today = QDate::currentDate();
    QSqlQuery q2 = db.searchCommandes("%", "EN_COURS", today.addDays(-30), today, "montant_desc");
    while (q2.next()) {
        qDebug() << "Commande en cours:"
                 << q2.value("prenom").toString() << q2.value("nom").toString()
                 << "- Montant:" << q2.value("montant_total").toDouble() << "€";
    }

    return 0;
}
