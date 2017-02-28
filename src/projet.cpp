#include "projet.h"
#include "mainwindow.h"
#include "ui_dialSetupProjet.h"


Projet *Projet::_instance = NULL;

Projet* Projet::Instance(){
    if(_instance==NULL){
        _instance = new Projet();
        connect(qApp, SIGNAL(aboutToQuit()), _instance, SLOT(deleteLater()));
    }
    return _instance;
}
void Projet::resetInstance(){
    disconnect(qApp, SIGNAL(aboutToQuit()), _instance, SLOT(deleteLater()));
    delete _instance;
    _instance = NULL;
}

Projet::~Projet()
{
    if(_projectThread){
        _projectThread->quit();
    }
}


Projet::Projet() :
    QObject()
{   
    qRegisterMetaType<Projet::Action>("Projet::Action");

    _projectThread = new QThread(this);

    _database = new DataBase(this);


    _saved = true;
    _closed = true;
    _settingUp = false;
    _currentFile.clear();

    moveToThread(_projectThread);
    _database->moveToThread(_projectThread);

    _projectThread->start();
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);

}

void Projet::init()
{

}

bool Projet::setupCreation(){
    if(QThread::currentThread()!=_projectThread){
        QMetaObject::invokeMethod(this, "setupCreation", Qt::QueuedConnection);
        return false;
    }
    changeAction(initiateProjet);

    if(!isClosed())
        close();

    if(!createTmp())
        return fail();

    if(!readTmp())
        return fail();

    _currentFile.clear();
    _saved = true;
    _settingUp = true;
    return success();

}

bool Projet::validateCreation()
{
    if(QThread::currentThread()!=_projectThread){
        QMetaObject::invokeMethod(this, "validateCreation", Qt::QueuedConnection);
        return false;
    }

    if(!_settingUp && !setupCreation())
        return fail();

    changeAction(createProjet);

    reportOpened();
    reportNotSaved();
    _settingUp = false;

    return success();
}


bool Projet::open(QString filepath){
    if(QThread::currentThread()!=_projectThread){
        QMetaObject::invokeMethod(this, "open", Qt::QueuedConnection, Q_ARG(QString, filepath));
        return false;
    }

    changeAction(openProjet);

    if(filepath.isEmpty())
        return fail("Aucun fichier n'a été spécifié pour être ouvert.");
    else
        filepath = QFileInfo(filepath).absoluteFilePath();

    if(!isClosed()){
        close();
        if(!isClosed())
            return fail();
        return open(filepath);
    }

    if(!extractFromFile(filepath))
        return fail();

    if(!readTmp())
        return fail();

    _currentFile = filepath;

    reportOpened();
    reportSaved();

    return success();
}

bool Projet::save(QString filepath){
    if(QThread::currentThread()!=_projectThread){
        QMetaObject::invokeMethod(this, "save", Qt::QueuedConnection, Q_ARG(QString, filepath));
        return false;
    }
    changeAction(saveProjet);


    if(filepath.isEmpty()){
        if(!_currentFile.isEmpty())
            filepath = _currentFile;
        else
            return fail("Aucun fichier n'a été spécifié pour la sauvegarde du projet.");
    }

    if(!compressToFile(filepath)){
        _currentFile.clear();
        return fail("Erreur lors de l'écriture du projet.");
    }

    _currentFile = filepath;

    reportSaved();
    return success();
}

bool Projet::recover()
{
    if(QThread::currentThread()!=_projectThread){
        QMetaObject::invokeMethod(this, "recover", Qt::QueuedConnection);
        return false;
    }
    changeAction(recoverProjet);
    if(!readTmp())
        return fail("Impossible de lire le dossier temporaire.");

    _currentFile.clear();

    reportOpened();
    reportNotSaved();

    return success();
}

bool Projet::close(){

    if(isClosed())
        return true;

    if(QThread::currentThread()!=_projectThread){
        if(!_saved){
            QMessageBox msgBox(MainWindow::Instance());
             msgBox.setText(tr("Les comptes ont été modifié"));
             msgBox.setInformativeText(tr("Voulez vous enregistrer les changements?"));
             msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
             msgBox.setDefaultButton(QMessageBox::Save);
             switch(msgBox.exec()){
                case QMessageBox::Save:
                    if(!save())
                        return false;
                 break;
                case QMessageBox::Cancel:
                    fail("Annulation de la fermeture du projet.");
                    return false;
            }
        }

        QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
        return false;
    }

    changeAction(closeProjet);


    clearTmp();
    reportSaved();
    _settingUp = false;
    reportClosed();

    return success();
}


void Projet::reportNotSaved(){
    _saved = false;
    emit(changed());
}

void Projet::reportSaved()
{
    _saved = true;
    emit(saved());
}

void Projet::reportOpened()
{
    _closed = false;
    emit(oppened());
}

void Projet::reportClosed()
{
    _closed = true;
    emit closed();
}

//________________________ GESTION DES FICHIERS TEMPORAIRES _____________________________

bool Projet::readTmp()
{

    QSqlDatabase *db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    QString path = QDir::toNativeSeparators(QDir::tempPath()+QDir::separator()+"GMoney"+QDir::separator()+"data.sqlite");
    db->setDatabaseName(path);

    if(!_database->connectDB(db)){
        QMessageBox::critical(MainWindow::Instance(), tr("Erreur de lecture"), tr("Une erreur est survenue lors de la lecture de la base de donnée."));
        return false;
    }

    return true;
}

bool Projet::createTmp()
{
    QDir(QDir::tempPath()).mkdir("GMoney");
    ///TODO: initialiser les fichiers de projets
    return true;
}

bool Projet::extractFromFile(QString file)
{
    QByteArray zipPathArray(QFileInfo(file).absoluteFilePath().toLocal8Bit());
    char* zipPath = zipPathArray.data();


    QDir(QDir::tempPath()).mkdir("GMoney");

    /*----------------- On ouvre le fichier zip --------------*/

    struct zip *zipArchive = NULL;
    int errOuverture=0;

    zipArchive = zip_open(zipPath, ZIP_CHECKCONS, &errOuverture);

    if(errOuverture!=ZIP_ER_OK || zipArchive == NULL)
        return openFailure(tr("Erreur lors de l'ouverture du fichier: "));

    int nbrFile = zip_get_num_files(zipArchive);

    if(nbrFile==0){
        zip_close(zipArchive);
        return openFailure(tr("Le fichier est illisible"));
    }

    QDir(QApplication::applicationDirPath()).mkdir("tmp");


    for(int i=0; i<nbrFile; i++){

        //-------------------- Ouverture de chaque fichier ------

        //Recuperation des stat
        struct zip_stat fileStat;
        if(zip_stat_index(zipArchive, i, 0, &fileStat) == -1){
            zip_close(zipArchive);
            return openFailure(tr("Erreur lors de la lecture des propriétés du fichier %1 de l'archive").arg(i));
        }

        //Ouverture du fichier
        struct zip_file *file = zip_fopen_index(zipArchive, i, ZIP_FL_UNCHANGED);
        if(file == NULL){
            zip_close(zipArchive);
            return openFailure(tr("Erreur lors de la lecture du fichier %1 de l'archive").arg(i));
        }

        //Creation et allocation mémoire pour le buffer
        char *str=NULL;
          str = (char*) malloc ((size_t)(fileStat.size+1));
          memset(str, 0, (size_t)(fileStat.size+1));
          if(str == NULL){
              zip_close(zipArchive);
              return openFailure(tr("L'allocation memoire à échouer"));
          }


        //Décodage du zip
          if(zip_fread(file, str, fileStat.size) == -1)
          {
              free (str);
              QString error(zip_file_strerror(file));
              zip_close(zipArchive);
              return openFailure(tr("Le decodage du fichier %1 à échouer: \n").arg(i) + error);
          }

          zip_fclose(file);

        std::string fileName = fileStat.name;
        if(fileName.at(fileName.size()-1)=='/'){
            // Creation d'un dossier
            if(!QDir(QDir::tempPath()+"/GMoney/").mkpath(fileStat.name))
                return openFailure("Impossible de créer une arborescence dans le dossier temporaire");

        }else{
            // Creation du fichier de sortie
            QFile outputFile(QDir::toNativeSeparators(QDir::tempPath()+QDir::separator()+"GMoney"+QDir::separator()+fileStat.name));
            if(!outputFile.open(QIODevice::WriteOnly)){
                zip_close(zipArchive);
                return openFailure("Impossible d'écrire dans le dossier temporaire");
            }
            QDataStream *out = new QDataStream(&outputFile);

            //Ecriture du fichier de sortie
            out->writeRawData(str,fileStat.size);

            //Nettoyage memoire
            free (str);
            outputFile.close();
        }
    }

    zip_close(zipArchive);

    return true;
}

bool Projet::compressToFile(QString file)
{
    struct zip *f_zip=NULL;
    struct zip_source *doc = NULL;
    int err = 0;

    QString outPath = QFileInfo(file).absoluteFilePath();
    QFile::remove(file);

    f_zip=zip_open(outPath.toStdString().data(), ZIP_CREATE, &err); /* on crée l'archive */

    /* s'il y a des erreurs */
    if(err != ZIP_ER_OK || f_zip==NULL)
        return saveFailure("Impossible d'ouvrir l'archive "+ outPath);

    QString root = QDir::tempPath()+QDir::separator()+"GMoney";
    QStringList content;
    listSubContents(content, root);

    qDebug()<<content;

    foreach(QString f, content)
    {
        /* on teste si le fichier demandé est un dossier ou non */
        bool error = false;

        if(f.endsWith('/')){
            /* Si cela en est un, on l'ajoute à l'archive */
            f = f.remove(f.length()-1);

            error = zip_add_dir(f_zip, f.toStdString().data()) == -1;
        } else { /* Sinon on le traite comme un fichier normal */
            /* que l'on récupère dans un zip_source */
            error = !(doc=zip_source_file(f_zip,(root+'/'+f).toStdString().data(),(off_t)0,(off_t)0));

            /* pour l'inclure dans l'archive */
            if(!error)
                error = zip_add(f_zip, f.toStdString().data(), doc)==-1;
        }

        if(error){
            QString error(zip_strerror(f_zip));
            zip_close(f_zip);
            f_zip = NULL;
            zip_source_free(doc);
            doc = NULL;
            return saveFailure("Erreur lors de la compressions de " + f + "\n" + error);
        }
    }

    zip_close(f_zip);
    f_zip = NULL;
    doc = NULL;

    return true;
}

bool Projet::clearTmp()
{
    return QDir(QDir::tempPath()+QDir::separator()+"GMoney").removeRecursively();
}

void Projet::listSubContents(QStringList& content, QDir path){
    QString root="";
    if(!content.isEmpty())
        root = content.last();

    foreach(QString p,  path.entryList(QDir::AllDirs|QDir::NoDotAndDotDot)){
        content.append(root + p+'/');
        listSubContents(content, path.path()+'/'+p);
    }

    foreach(QString f, path.entryList(QDir::Files))
        content.append( root + f );
}

bool Projet::openFailure(QString errorDescription)
{
    _lastError = errorDescription;
    return false;
}

bool Projet::saveFailure(QString errorDescription)
{
    _lastError = errorDescription;
    return false;
}

bool Projet::checkForRecovery()
{
    if(QDir(QDir::tempPath()+QDir::separator()+"GMoney").exists()){
        //-- Le logiciel a sans doute crashé, récupération du projet précédant --

        QMessageBox* mBox = new QMessageBox( QMessageBox::Warning,
                                             tr("Récupération de la session précédente"),
                                             tr("Le logiciel a du crasher lors de la dernière utilisation, voulez-vous tenter récuperer la session précédente?"),
                                             QMessageBox::Yes|QMessageBox::Ignore);
        int r = mBox->exec();
        if(r == QMessageBox::Yes)
            recover();
        else
            clearTmp();

        mBox->deleteLater();

        return r == QMessageBox::Yes;
    }

    return false;
}

void Projet::changeAction(Projet::Action a)
{
    if(a==_action)
        return;

    if(_action!=ActionLast)
        fail();

    if(_settingUp && (a!=createProjet || a!=closeProjet))
        close();

    _action = a;
    _lastError = "";
    emit(actionChanged(a));

}

bool Projet::fail(QString error)
{
    if(_action==ActionLast)
        return true;

    emit(actionFailed(_action));
    qDebug()<<_action<<" failed" << (error.isEmpty()?"!":(": "+error));
    if(!error.isEmpty())
        _lastError = error;
    _action = ActionLast;

    return false;
}

bool Projet::success()
{
    if(_action==ActionLast)
        return false;

    emit(actionSucceed(_action));
    qDebug()<<_action<<" succeeded!";
    _action = ActionLast;

    return true;
}
