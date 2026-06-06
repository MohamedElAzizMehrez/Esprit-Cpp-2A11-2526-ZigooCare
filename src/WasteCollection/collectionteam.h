#ifndef COLLECTIONTEAM_H
#define COLLECTIONTEAM_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class CollectionTeam
{
    int teamID;
    int teamSize;
    QString vehiclePlate;
    QString contactNumber;
    QString status;
    int price;
    int averageRating;
    int ratingCount;
    int activeAssignment;
    int averageCompletionTime;
public:

    //Constructeurs
    CollectionTeam(){}
    CollectionTeam(int,int,QString,QString,QString,int,int,int,int,int);

    //Getters
    int getCollectionTeamID(){return teamID;}
    int getTeamSize(){return teamSize;}
    QString getVehiclePlate(){return vehiclePlate;}
    QString getContactNumber(){return contactNumber;}
    QString getStatus(){return status;}
    int getPrice(){return price;}
    int getAverageRating(){return averageRating;}
    int getRatingCount(){return ratingCount;}
    int getActiveAssignment(){return activeAssignment;}
    int getAverageCompletionTime(){return averageCompletionTime;}

    //Setters
    void setCollectionTeamID(int cID){teamID=cID;}
    void setTeamSize(int tSize){teamSize=tSize;}
    void setVehiclePlate(QString VPlate){vehiclePlate=VPlate;}
    void setContactNumber(QString Cnum){contactNumber=Cnum;}
    void setStatus(QString TStatus){status=TStatus;}
    void setPrice(int TPrice){price=TPrice;}
    void setAverageRating(int AvRating){averageRating=AvRating;}
    void setRatingCount(int RatingCoun){averageRating=RatingCoun;}
    void setActiveAssignment(int ActiveAss){activeAssignment=ActiveAss;}
    void setAverageCompletionTime(int AvComplTime){averageCompletionTime=AvComplTime;}

    //CRUD CollectionTeam
    bool addTeam();
    QSqlQueryModel * DisplayTeams();
    bool deleteTeam(int);
    bool updateTeam();

    //Basic functions
    QSqlQueryModel * sortByStatus();
    QSqlQueryModel* searchTeamByID(QString id);

    //Advanced Features
    QSqlQueryModel * displayIntervenesForAffectC();
    QSqlQueryModel * displayTeamsToAffect();
    bool affectTeamToIntervention(int teamID, QString region, QDate date);
    QSqlQueryModel* rankTeams();
    bool updateTeamRating(int id, int newStars);

};

#endif // COLLECTIONTEAM_H
