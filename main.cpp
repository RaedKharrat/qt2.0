#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Test database connection first
    DatabaseManager db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, "Erreur",
                              "Impossible de se connecter à la base de données.\n"
                              "Vérifiez que MySQL est démarré et que la base 'credit_db' existe.");
        return -1;
    }

    MainWindow w;
    w.setWindowTitle("Gestion de Crédit - Clients et Commandes");
    w.resize(1200, 700);
    w.show();

    return a.exec();
}
