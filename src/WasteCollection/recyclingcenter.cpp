#include "recyclingcenter.h"
RecyclingCenter::RecyclingCenter(int centerID,QString name,QString address,QString email,int maxCapacity,int currentCapacity,QString status,int municipalityID){
    this->centerID=centerID;
    this->name=name;
    this->address=address;
    this->email=email;
    this->maxCapacity=maxCapacity;
    this->currentCapacity=currentCapacity;
    this->status=status;
    this->municipalityID=municipalityID;
}
bool RecyclingCenter::addCenter(){
    QSqlQuery query;
    QString maxC = QString::number(maxCapacity);
    QString curC = QString::number(currentCapacity);
    QString munID = QString::number(municipalityID);
    query.prepare("insert into RECYCLING_CENTER (NAME,ADDRESS,EMAIL,MAX_CAPACITY,CURRENT_CAPACITY,STATUS,MUNICIPALITY_ID)" "values (:NAME, :ADDRESS, :EMAIL, :MAX_CAPACITY, :CURRENT_CAPACITY, :STATUS, :MUNICIPALITY_ID)");
    query.bindValue(":NAME",name);
    query.bindValue(":ADDRESS",address);
    query.bindValue(":EMAIL",email);
    query.bindValue(":MAX_CAPACITY",maxC);
    query.bindValue(":CURRENT_CAPACITY",curC);
    query.bindValue(":STATUS",status);
    query.bindValue(":MUNICIPALITY_ID",munID);
    return query.exec();
}

QSqlQueryModel * RecyclingCenter::displayCenter(){
    QSqlQueryModel * model=new QSqlQueryModel();

    model->setQuery("select R.* , M.NAME from RECYCLING_CENTER R LEFT JOIN MUNICIPALITY M on M.MUNICIPALITY_ID = R.MUNICIPALITY_ID ORDER BY R.CENTER_ID ASC");
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("ID"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Name"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Address"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Email"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Max capacity"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("Current capacity"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("Status"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("Municipality ID"));
    model->setHeaderData(8,Qt::Horizontal,QObject::tr("Municipality"));

    return model;
}
bool RecyclingCenter::deleteCenter(int centerID){
    QSqlQuery query;
    QString id=QString::number(centerID);

    query.prepare("Delete from RECYCLING_CENTER where CENTER_ID= :id");

    query.bindValue(":id",id);

    return query.exec();
}
bool RecyclingCenter::updateCenter(int id){
    QSqlQuery query;
    QString maxC = QString::number(maxCapacity);
    QString curC = QString::number(currentCapacity);
    QString munID = QString::number(municipalityID);
    query.prepare("UPDATE RECYCLING_CENTER SET NAME=:NAME,ADDRESS=:ADDRESS,EMAIL=:EMAIL,MAX_CAPACITY=:MAX_CAPACITY,CURRENT_CAPACITY=:CURRENT_CAPACITY,STATUS=:STATUS,MUNICIPALITY_ID=:MUNICIPALITY_ID WHERE CENTER_ID=:id");
    query.bindValue(":NAME",name);
    query.bindValue(":ADDRESS",address);
    query.bindValue(":EMAIL",email);
    query.bindValue(":MAX_CAPACITY",maxC);
    query.bindValue(":CURRENT_CAPACITY",curC);
    query.bindValue(":STATUS",status);
    query.bindValue(":MUNICIPALITY_ID",munID);
    query.bindValue(":id",id);
    return query.exec();
}

QSqlQueryModel* RecyclingCenter::searchCenterByName(const QString &name)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(
        "SELECT  R.* , M.NAME from RECYCLING_CENTER R LEFT JOIN MUNICIPALITY M on M.MUNICIPALITY_ID = R.MUNICIPALITY_ID "
        "WHERE LOWER(R.name) LIKE LOWER(:name) "
        );
    query.bindValue(":name", "%" + name + "%");
    query.exec();
    model->setQuery(std::move(query));
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("ID"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Name"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Address"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Email"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Max capacity"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("Current capacity"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("Status"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("Municipality ID"));
    model->setHeaderData(8,Qt::Horizontal,QObject::tr("Municipality"));
    return model;
}

QSqlQueryModel* RecyclingCenter::sortCenterByStatus()
{
    QSqlQueryModel* model = new QSqlQueryModel();

    model->setQuery(
        "SELECT R.* , M.NAME from RECYCLING_CENTER R LEFT JOIN MUNICIPALITY M on M.MUNICIPALITY_ID = R.MUNICIPALITY_ID "
        "ORDER BY status ASC"
        );
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("ID"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Name"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Address"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Email"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Max capacity"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("Current capacity"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("Status"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("Municipality ID"));
    model->setHeaderData(8,Qt::Horizontal,QObject::tr("Municipality"));

    return model;
}

bool RecyclingCenter::getCenterByEmail(QString email)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM RECYCLING_CENTER WHERE EMAIL=:email");
    query.bindValue(":email", email);
    if(query.exec() && query.next())
    {
        this->centerID=query.value("CENTER_ID").toInt();
        this->name=query.value("NAME").toString();
        this->address=query.value("ADDRESS").toString();
        this->email=query.value("EMAIL").toString();
        this->maxCapacity=query.value("MAX_CAPACITY").toInt();
        this->currentCapacity=query.value("CURRENT_CAPACITY").toInt();
        this->status=query.value("STATUS").toString();
        this->municipalityID=query.value("MUNICIPALITY_ID").toInt();
        return true;
    }
    return false;
}

QSqlQueryModel* RecyclingCenter::getCenterActiveByMunicipality(int MunicipalityID,int quantity)
{
    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT * FROM recycling_center "
        "WHERE MUNICIPALITY_ID = :MunicipalityID AND STATUS = 'Active' AND (MAX_CAPACITY - CURRENT_CAPACITY) >= :quantity"
        );
    query.bindValue(":MunicipalityID", MunicipalityID);
    query.bindValue(":quantity", quantity);
    query.exec();

    model->setQuery(std::move(query));
    return model;
}

QSqlQueryModel * RecyclingCenter::getCentersSaturated(){
    QSqlQueryModel * model = new QSqlQueryModel();
    model->setQuery(
        "SELECT rc.center_id, "
        "COUNT(i.sewer_id) AS nb_interventions, "
        "rc.MAX_CAPACITY AS waste_quantity "
        "FROM Recycling_Center rc "
        "JOIN Municipality m ON rc.municipality_id = m.municipality_id "
        "JOIN Sewer s ON m.municipality_id = s.municipality_id "
        "JOIN intervenes i ON s.sewer_id = i.sewer_id "
        "WHERE rc.status='Saturated' AND i.status=1"
        "GROUP BY rc.center_id, rc.MAX_CAPACITY"
        );
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Center ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Number of Interventions"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Waste Quantity"));
    return model;
}
QSqlQueryModel * RecyclingCenter::getCentersActive(){
    QSqlQueryModel * model=new QSqlQueryModel();

    model->setQuery("select * from RECYCLING_CENTER WHERE STATUS = 'Active' ");
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("ID"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Name"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Address"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Email"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Max capacity"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("Current capacity"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("Status"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("Municipality ID"));

    return model;
}

QSqlQueryModel * RecyclingCenter::getCentersPredictionData(){
    QSqlQueryModel * model = new QSqlQueryModel();

    model->setQuery(
        "SELECT rc.center_id, rc.max_capacity, rc.current_capacity, "
        "SUM(s.current_capacity) AS sum_waste_muni, "
        "rc.municipality_id,"
        "rc.NAME "
        "FROM Recycling_Center rc "
        "JOIN Municipality m ON rc.municipality_id = m.municipality_id "
        "JOIN Sewer s ON m.municipality_id = s.municipality_id "
        "WHERE rc.status='Active' "
        "GROUP BY rc.center_id, rc.max_capacity, rc.current_capacity, rc.municipality_id, rc.NAME"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Center ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Max Capacity"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Current Capacity"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Sum Waste per Intervention"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Municipality ID"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Center Name"));

    return model;
}
QSqlQueryModel * RecyclingCenter::displayIntervenesForAffect(){
    QSqlQueryModel * model=new QSqlQueryModel();

    model->setQuery(
        "SELECT I.TEAM_ID, "
        "MAX(T.CONTACT_NUMBER) AS CONTACT_NUMBER, "
        "MAX(M.NAME) AS MUNICIPALITY_NAME, "
        "TRUNC(I.INTERVENTION_DATE) AS INTERVENTION_DATE, "
        "MAX(I.PRIORITY) AS PRIORITY, "
        "SUM(S.CURRENT_CAPACITY) AS TOTAL_WASTE, "
        "COUNT(I.SEWER_ID) AS NB_SEWERS,"
        "S.MUNICIPALITY_ID "
        "FROM INTERVENES I "
        "JOIN COLLECTION_TEAM T ON I.team_id = T.team_id "
        "JOIN SEWER S ON I.sewer_id = S.sewer_id "
        "JOIN MUNICIPALITY M ON S.municipality_id = M.municipality_id "
        "WHERE I.TEAM_ID > 0 AND I.STATUS=0"
        "GROUP BY I.TEAM_ID, S.MUNICIPALITY_ID, TRUNC(I.INTERVENTION_DATE) "
        "ORDER BY TRUNC(I.INTERVENTION_DATE), PRIORITY DESC, TOTAL_WASTE DESC"
        );

    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Team ID"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Team Number"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Municipality"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Intervention Date"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Priority"));
    model->setHeaderData(5,Qt::Horizontal,QObject::tr("Total Waste Quantity"));
    model->setHeaderData(6,Qt::Horizontal,QObject::tr("Nb Sewers"));
    model->setHeaderData(7,Qt::Horizontal,QObject::tr("Municipality_id"));

    return model;
}
int RecyclingCenter::getCentersInMunicipality(int id)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Recycling_Center WHERE municipality_id = :municipality_id");
    query.bindValue(":municipality_id", id);

    if(query.exec())
    {
        if(query.next())
        {
            return query.value(0).toInt();
        }
    }

    return 0;
}

bool RecyclingCenter::updateIntervenesStatus(int teamID, int municipalityID, QDate date)
{
    QSqlQuery query;

    query.prepare("UPDATE INTERVENES "
                  "SET STATUS = 1 "
                  "WHERE TEAM_ID = :teamID "
                  "AND SEWER_ID IN (SELECT SEWER_ID FROM SEWER WHERE MUNICIPALITY_ID = :municipalityID) "
                  "AND TRUNC(INTERVENTION_DATE) = :date "
                  "AND STATUS = 0");

    query.bindValue(":teamID", teamID);
    query.bindValue(":municipalityID", municipalityID);
    query.bindValue(":date", date);

    return query.exec();
}

QSqlQueryModel * RecyclingCenter::getCentersPredictionDataForOne(int centerID){
    QSqlQueryModel * model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT rc.center_id, rc.max_capacity, rc.current_capacity, "
        "SUM(s.current_capacity) AS sum_waste_muni, "
        "rc.municipality_id, "
        "rc.NAME, "
        "rc.status "
        "FROM Recycling_Center rc "
        "JOIN Municipality m ON rc.municipality_id = m.municipality_id "
        "JOIN Sewer s ON m.municipality_id = s.municipality_id "
        "WHERE rc.center_id = :id "
        "GROUP BY rc.center_id, rc.max_capacity, rc.current_capacity, rc.municipality_id, rc.NAME,rc.status"
        );

    query.bindValue(":id", centerID);
    query.exec();

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Center ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Max Capacity"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Current Capacity"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Sum Waste per Intervention"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Municipality ID"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Center Name"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Center Status"));

    return model;
}
