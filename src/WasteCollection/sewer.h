#ifndef SEWER_H
#define SEWER_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>
class Sewer
{
    int sewerID;
    QString location;
    int maxCapacity;
    int currentCapacity;
    int waterLevel;
    int blockageRate;
    int municipalityID;

public:
    //constructeurs
    Sewer(){}
    Sewer(int,QString,int,int,int,int,int);

    //geters
    int getSewerID(){return sewerID;}
    QString getLocation(){return location;}
    int getMaxCapacity(){return maxCapacity;}
    int getCurrentCapcity(){return currentCapacity;}
    int getWaterLevel(){return waterLevel;}
    int getBlockageRate(){return blockageRate;}
    int getMunicipalityID(){return municipalityID;}
    QSqlQueryModel* getRegionsModelForAI();
     QSqlQueryModel* getSewersByRegion(QString region);
    QSqlQueryModel* getPrioritizedSewersByRegion(QString region,int waterWeight,int blockageWeight,int chanceOfRain);

    //seters
    void setSewerID(int sID){sewerID=sID;}
    void setLocation(QString l){location=l;}
    void setMaxCapacity(int mc){maxCapacity=mc;}
    void setCurrentCapacity(int cc){currentCapacity=cc;}
    void setWaterLevel(int wl){waterLevel=wl;}
    void setBlockageRate(int br){blockageRate=br;}
    void setMunicipalityID(int mID){municipalityID=mID;}

    bool addSewer();
    QSqlQueryModel* displaySortedSewerByBlockage();
    QSqlQueryModel* searchSewerByLocation(QString text);
    QSqlQueryModel * displaySewer();
    bool deleteSewer(int sewerID);
    bool updateSewer();
    bool getSewerById(int sewerID);
    bool addIntervention(int sewerId, int priority);
bool interventionExists(int sewerId);
bool clearCompletedSewers(int teamId, QDate date);
bool updateWaterLevelFromSensor(int sewerId, int waterLevel);
bool updateCapacityByState(int idSewer, QString state);
bool getSewerEnvironmentalInput(int sewerID,
                                int &maxCapacity,
                                int &currentCapacity,
                                int &waterLevel,
                                int &blockageRate,
                                int &municipalityID);
bool getMostDangerousSewer(int &sewerId,
                           int &waterLevel,
                           int &blockageRate);

bool getCriticalSewersCount(int &count);

bool getSewerSituationSummary(int &total,
                              int &critical);

bool getHighestRiskMunicipality(QString &municipality,
                                int &nbSewers,
                                int &avgWater,
                                int &avgBlockage,
                                int &score);
};
#endif // SEWER_H
