#include "collectionteam.h"

#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


CollectionTeam::CollectionTeam(int teamID, int teamSize , QString vehiclePlate, QString contactNumber, QString teamStatus, int teamPrice, int averageRating, int ratingCount, int activeAssignment, int averageCompletionTime)
{
    this->teamID=teamID;
    this->teamSize=teamSize;
    this->vehiclePlate=vehiclePlate;
    this->contactNumber=contactNumber;
    this->status=teamStatus;
    this->price=teamPrice;
    this->averageRating=averageRating;
    this->ratingCount=ratingCount;
    this->activeAssignment=activeAssignment;
    this->averageCompletionTime=averageCompletionTime;
}

bool CollectionTeam::addTeam()
{
        QSqlQuery query;
        query.prepare("INSERT INTO COLLECTION_TEAM ""(TEAM_SIZE, VEHICLE_PLATE, CONTACT_NUMBER, STATUS, PRICE)"
                      "VAlUES (:teamSize, :vehiclePlate, :contactNumber, :status, :price)");
        query.bindValue(":teamSize", teamSize);
        query.bindValue(":vehiclePlate", vehiclePlate);
        query.bindValue(":contactNumber", contactNumber);
        query.bindValue(":status", status);
        query.bindValue(":price", price);
        return query.exec();
}

QSqlQueryModel * CollectionTeam::DisplayTeams()
{
    QSqlQueryModel * model=new QSqlQueryModel();
    model->setQuery("select TEAM_ID, TEAM_SIZE, VEHICLE_PLATE, CONTACT_NUMBER, STATUS, PRICE from COLLECTION_TEAM where TEAM_ID != 0 ORDER BY TEAM_ID");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Team ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Team Size"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Vehicle Plate"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Contact Number"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Status"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Price"));

    return model;
}

bool CollectionTeam::deleteTeam(int teamID)
{
    QSqlQuery query;
    query.prepare("delete from COLLECTION_TEAM where TEAM_ID=:teamID");
    query.bindValue(":teamID",teamID);
    return query.exec();
}

bool CollectionTeam::updateTeam()
{
    QSqlQuery query;

    query.prepare("update COLLECTION_TEAM set TEAM_SIZE=:teamSize, VEHICLE_PLATE=:vehiclePlate, CONTACT_NUMBER=:contactNumber, STATUS=:status, PRICE=:price where TEAM_ID=:teamID");

    query.bindValue(":teamSize", teamSize);
    query.bindValue(":vehiclePlate", vehiclePlate);
    query.bindValue(":contactNumber", contactNumber);
    query.bindValue(":status", status);
    query.bindValue(":price", price);
    query.bindValue(":teamID", teamID);

    return query.exec();
}

QSqlQueryModel * CollectionTeam::sortByStatus()
{
    QSqlQueryModel * model = new QSqlQueryModel();
    model->setQuery("SELECT TEAM_ID, TEAM_SIZE, VEHICLE_PLATE, CONTACT_NUMBER, STATUS, PRICE "
                    "FROM COLLECTION_TEAM where TEAM_ID != 0 ORDER BY STATUS ASC");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Team ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Team Size"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Vehicle Plate"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Contact Number"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Status"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Price"));

    return model;
}

QSqlQueryModel* CollectionTeam::searchTeamByID(QString id)
{
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare("SELECT TEAM_ID, TEAM_SIZE, VEHICLE_PLATE, CONTACT_NUMBER, STATUS, PRICE "
                  "FROM COLLECTION_TEAM WHERE TEAM_ID LIKE :id and TEAM_ID !=0 ");
    query.bindValue(":id", id + "%");
    query.exec();

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Team ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Team Size"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Vehicle Plate"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Contact Number"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Status"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Price"));

    return model;
}

QSqlQueryModel * CollectionTeam::displayIntervenesForAffectC(){
    QSqlQueryModel * model=new QSqlQueryModel();

    model->setQuery(
        "SELECT "
        "M.REGION, "
        "TRUNC(I.INTERVENTION_DATE) AS INTERVENTION_DATE, "
        "MAX(I.PRIORITY) AS PRIORITY, "
        "SUM(S.CURRENT_CAPACITY) AS TOTAL_WASTE, "
        "COUNT(I.SEWER_ID) AS NB_SEWERS "
        "FROM INTERVENES I "
        "JOIN SEWER S ON I.SEWER_ID = S.SEWER_ID "
        "JOIN MUNICIPALITY M ON S.MUNICIPALITY_ID = M.MUNICIPALITY_ID "
        "WHERE I.TEAM_ID = 0 "
        "GROUP BY M.REGION, TRUNC(I.INTERVENTION_DATE) "
        "ORDER BY TRUNC(I.INTERVENTION_DATE), PRIORITY DESC, TOTAL_WASTE DESC"
        );

    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Region"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Intervention Date"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Priority"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Total Waste Quantity"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Nb Sewers"));

    return model;
}

QSqlQueryModel * CollectionTeam::displayTeamsToAffect()
{
    QSqlQueryModel * model=new QSqlQueryModel();
    model->setQuery("select TEAM_ID, TEAM_SIZE, CONTACT_NUMBER,PRICE from COLLECTION_TEAM where TEAM_ID != 0 and STATUS='Available'");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Team ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Team Size"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Contact Number"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Price"));

    return model;
}

bool CollectionTeam::affectTeamToIntervention(int teamID, QString region, QDate date)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) return false;

    QSqlQuery query;

    query.prepare("UPDATE INTERVENES "
                  "SET TEAM_ID = :teamID "
                  "WHERE TEAM_ID = 0 "
                  "AND SEWER_ID IN ("
                    "SELECT S.SEWER_ID "
                    "FROM SEWER S "
                    "JOIN MUNICIPALITY M ON S.MUNICIPALITY_ID = M.MUNICIPALITY_ID "
                    "WHERE M.REGION = :region "
                  ") "
                  "AND TRUNC(INTERVENTION_DATE) = :date "
                  "AND STATUS = 0");

    query.bindValue(":teamID", teamID);
    query.bindValue(":region", region);
    query.bindValue(":date", date);


    // YOU MUST CALL EXEC() HERE
    if (!query.exec()) {
        qDebug() << "Update INTERVENES failed:" << query.lastError().text();
        db.rollback();
        return false;
    }
    query.prepare("UPDATE COLLECTION_TEAM "
                  "SET STATUS = 'Unavailable' "
                  "WHERE TEAM_ID = :teamID");

    query.bindValue(":teamID", teamID);

    // YOU MUST CALL EXEC() HERE TOO
    if (!query.exec()) {
        qDebug() << "Update COLLECTION_TEAM failed:" << query.lastError().text();
        db.rollback();
        return false;
    }

    // Only if BOTH exec() calls worked do we commit
    return db.commit();
}

QSqlQueryModel* CollectionTeam::rankTeams() {
    QSqlQueryModel *model = new QSqlQueryModel();

    QString queryStr =
        "SELECT TEAM_ID, CONTACT_NUMBER, AVERAGE_RATING, RATING_COUNT, "
        "((AVERAGE_RATING * RATING_COUNT + (3.0 * 5)) / (RATING_COUNT + 5)) AS WEIGHTED_SCORE "
        "FROM COLLECTION_TEAM  where TEAM_ID != 0  "
        "ORDER BY WEIGHTED_SCORE DESC";

    model->setQuery(queryStr);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Contact"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Avg Star"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Total Reviews"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Performance Score"));

    return model;
}

bool CollectionTeam::updateTeamRating(int id, int newStars) {
    QSqlQuery query;
    // We calculate the new average and increment the count in one SQL statement
    query.prepare("UPDATE COLLECTION_TEAM SET "
                  "AVERAGE_RATING = (COALESCE(AVERAGE_RATING, 0) * COALESCE(RATING_COUNT, 0) + :newStars) / (COALESCE(RATING_COUNT, 0) + 1), "
                  "RATING_COUNT = COALESCE(RATING_COUNT, 0) + 1 "
                  "WHERE TEAM_ID = :id");
    query.bindValue(":newStars", newStars);
    query.bindValue(":id", id);
    return query.exec();
}

