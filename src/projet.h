#ifndef PROJET_H
#define PROJET_H

#include <QtCore>
#include <zip.h>
#include <QtSql>

#include "GUI/config.h"
#include "database.h"

class Projet;



class Projet : public QObject
{
    Q_OBJECT
public:
    static Projet* Instance();
    static void resetInstance();

    virtual ~Projet();

    DataBase *getDatabase() {return _database;}
    QThread *getDataThread(){return _projectThread;}

    bool isClosed() const {return _closed;}

    enum Action{
        initiateProjet,
        createProjet,
        openProjet,
        saveProjet,
        recoverProjet,
        closeProjet,
        ActionLast
    };


    Q_ENUM(Action)

    Action currentAction() const {return _action;}
    const QString& getLastError() const { return _lastError;}

    QString getCurrentFile() const {return _currentFile;}
    bool    hasCurrentFile() const {return !_currentFile.isEmpty();}

signals:
    void closed();
    void oppened();

    void changed();
    void saved();

    void projetSetuped();

    void actionChanged(Projet::Action a);
    void actionSucceed(Projet::Action a);
    void actionFailed(Projet::Action a);

public slots:
    void init();
    bool setupCreation();
    bool validateCreation();
    bool open(QString filepath);
    bool save(QString filepath="");
    bool recover();
    bool close();

    bool checkForRecovery();

    void reportNotSaved();
    void reportSaved();

    void reportOpened();
    void reportClosed();

private:
    Projet();

    bool readTmp();
    bool createTmp();
    bool extractFromFile(QString file);
    bool compressToFile(QString file);
    bool clearTmp();

    bool openFailure(QString errorDescription);
    bool saveFailure(QString errorDescription);
    void listSubContents(QStringList &content, QDir path);


    void changeAction(Action a);
    bool fail(QString error="");
    bool success();

    DataBase *_database;

    QThread *_projectThread;
    Action _action = ActionLast;

    QString _currentFile;
    QString _lastError;

    bool _saved;
    bool _closed;
    bool _settingUp;

    static Projet* _instance;
};


#endif // PROJET_H
