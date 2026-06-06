#ifndef RECYCLINGCENTER_H
#define RECYCLINGCENTER_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class RecyclingCenter
{
    int centerID,maxCapacity,currentCapacity,municipalityID;
    QString name,address,email,status;

public:
    //constructeurs
    RecyclingCenter(){}
    RecyclingCenter(int,QString,QString,QString,int,int,QString,int);
    //getters
    int getID(){return centerID;}
    QString getName(){return name;}
    QString getAddress(){return address;}
    QString getEmail(){return email;}
    int getMaxc(){return maxCapacity;}
    int getCurc(){return currentCapacity;}
    QString getStatus(){return status;}
    int getMunicipalityID(){return municipalityID;}
    //setters
    void setID(int ID){centerID=ID;}
    void setName(QString name){this->name=name;}
    void setAddress(QString address){this->address=address;}
    void setEmail(QString email){this->email=email;}
    void setMaxc(int maxc){maxCapacity=maxc;}
    void setCurc(int curc){currentCapacity=curc;}
    void setStatus(QString st){status=st;}
    void setMunicipalityID(int munID){municipalityID=munID;}
    //functions
    bool addCenter();
    QSqlQueryModel * displayCenter();
    bool deleteCenter(int centerID);
    bool updateCenter(int centerID);
    QSqlQueryModel* searchCenterByName(const QString &name);
    QSqlQueryModel* sortCenterByStatus();
    bool getCenterByEmail(QString email);
    QSqlQueryModel * getCenterActiveByMunicipality(int MunicipalityID,int quantity);
    QSqlQueryModel * getCentersSaturated();
    QSqlQueryModel * getCentersPredictionData();
    QSqlQueryModel * getCentersActive();
    QSqlQueryModel * displayIntervenesForAffect();
    int getCentersInMunicipality(int id);
    bool updateIntervenesStatus(int teamID, int municipalityID, QDate date);
    QSqlQueryModel * getCentersPredictionDataForOne(int centerID);
};

#endif // RECYCLINGCENTER_H
