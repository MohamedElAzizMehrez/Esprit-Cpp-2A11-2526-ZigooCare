#ifndef MUNICIPALITY_H
#define MUNICIPALITY_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QVariantList>
#include <QVariantMap>

class Municipality
{
    int municipalityID;
    QString name;
    QString region;
    QString address;
    QString phone;
    QString email;
    QString longitude;
    QString latitude;
    int radius;

public:
    //Constructeurs
    Municipality(){}
    Municipality(int,QString,QString,QString,QString,QString,QString,QString,int);
    //Getters
    int getMunicipalityID(){return municipalityID;}
    QString getName(){return name;}
    QString getRegion(){return region;}
    QString getAddress(){return address;}
    QString getPhone(){return phone;}
    QString getEmail(){return email;}
    QString getLongitude(){return longitude;}
    QString getLatitude(){return latitude;}
    int getRadius(){return radius;}
    //Setters
    void setMunicipalityID(int municipalityID){this->municipalityID=municipalityID;}
    void setName(QString name){this->name=name;}
    void setRegion(QString region){this->region=region;}
    void setAddress(QString address){this->address=address;}
    void setPhone(QString phone){this->phone=phone;}
    void setEmail(QString email){this->email=email;}
    void setLongitude(QString longitude){this->longitude=longitude;}
    void setLatitude(QString latitude){this->latitude=latitude;}
    void setRadius(int radius){this->radius=radius;}
    //Fonctionnalités de base relatives a l 'entité Municipality
    bool addMunicipality();
    bool getMunicipalityEmail(QString email);
    bool getMunicipalityAddress(QString address);
    bool getMunicipalityName(QString name);
    bool getMunicipalityPhone(QString phone);
    QSqlQueryModel * displayMunicipality();
    QSqlQueryModel * displaySortedMunicipality();
    bool deleteMunicipality(int);
    bool updateMunicipality();
    QVariantList getMunicipalitiesMapData();
    //budget
    int    getNbInterventions(int municipalityID);
    double getAvgBlockageRate(int municipalityID);
    int    getNbSewers(int municipalityID);
    double calculateScore(int municipalityID);
    double calculateTotalScore();
    double calculatePercentage(int municipalityID, double totalScore);
    double calculateBudget(int municipalityID, double totalScore, double totalBudget);
    QSqlQueryModel* getBudgetMunicipalityList();
    QString getMunicipalityRiskMaquette(int municipalityId);

};

#endif // MUNICIPALITY_H
