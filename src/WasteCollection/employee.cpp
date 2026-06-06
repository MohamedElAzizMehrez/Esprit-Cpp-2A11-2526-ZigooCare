#include "employee.h"

Employee::Employee(int id, QString nic, QString firstName, QString lastName, QString position, QString email, QString password, QString question, QString answer, int fingerprint)
{
    employeeID=id;
    this->nic=nic;
    this->firstName=firstName;
    this->lastName=lastName;
    this->position=position;
    this->email=email;
    this->password=password;
    this->question=question;
    this->answer=answer;
    this->fingerprint=fingerprint;
}

bool Employee::addEmployee()
{
    QSqlQuery query;
    query.prepare("INSERT INTO EMPLOYEE(EMPLOYEE_ID, NIC, FIRST_NAME, LAST_NAME, POSITION, EMAIL) VALUES (EMPLOYEE_SEQ.NEXTVAL, :nic, :first, :last, :position, :email)");
    query.bindValue(":nic", nic);
    query.bindValue(":first", firstName);
    query.bindValue(":last", lastName);
    query.bindValue(":position", position);
    query.bindValue(":email", email);
    return query.exec();
}

QSqlQueryModel * Employee::displayEmployees()
{
    QSqlQueryModel * model=new QSqlQueryModel();
    model->setQuery("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME, NIC, POSITION, EMAIL FROM EMPLOYEE ORDER BY EMPLOYEE_ID");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("First Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Last Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("NIC"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Position"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    return model;
}

QSqlQueryModel * Employee::displaySortedEmployees()
{
    QSqlQueryModel * model=new QSqlQueryModel();
    model->setQuery("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME, NIC, POSITION, EMAIL FROM EMPLOYEE ORDER BY POSITION");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("First Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Last Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("NIC"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Position"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    return model;
}

bool Employee::getEmployeeByID(int id)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEE WHERE EMPLOYEE_ID=:id");
    query.bindValue(":id", id);
    if(query.exec() && query.next())
    {
        employeeID=query.value("EMPLOYEE_ID").toInt();
        nic=query.value("NIC").toString();
        firstName=query.value("FIRST_NAME").toString();
        lastName=query.value("LAST_NAME").toString();
        position=query.value("POSITION").toString();
        email=query.value("EMAIL").toString();
        password=query.value("PASSWORD").toString();
        question=query.value("QUESTION").toString();
        answer=query.value("ANSWER").toString();
        fingerprint=query.value("FINGERPRINT").toInt();
        return true;
    }
    return false;
}

bool Employee::getEmployeeByNic(QString nic)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEE WHERE NIC=:nic");
    query.bindValue(":nic", nic);
    if(query.exec() && query.next())
    {
        employeeID=query.value("EMPLOYEE_ID").toInt();
        this->nic=query.value("NIC").toString();
        firstName=query.value("FIRST_NAME").toString();
        lastName=query.value("LAST_NAME").toString();
        position=query.value("POSITION").toString();
        email=query.value("EMAIL").toString();
        password=query.value("PASSWORD").toString();
        question=query.value("QUESTION").toString();
        answer=query.value("ANSWER").toString();
        fingerprint=query.value("FINGERPRINT").toInt();
        return true;
    }
    return false;
}

bool Employee::getEmployeeByEmail(QString email)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEE WHERE EMAIL=:email");
    query.bindValue(":email", email);
    if(query.exec() && query.next())
    {
        employeeID=query.value("EMPLOYEE_ID").toInt();
        nic=query.value("NIC").toString();
        firstName=query.value("FIRST_NAME").toString();
        lastName=query.value("LAST_NAME").toString();
        position=query.value("POSITION").toString();
        this->email=query.value("EMAIL").toString();
        password=query.value("PASSWORD").toString();
        question=query.value("QUESTION").toString();
        answer=query.value("ANSWER").toString();
        fingerprint=query.value("FINGERPRINT").toInt();
        return true;
    }
    return false;
}

bool Employee::getEmployeeByFingerprint(int print)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYEE WHERE FINGERPRINT=:fingerprint");
    QString fingerprint = QString::number(print);
    query.bindValue(":fingerprint", fingerprint);
    if(query.exec() && query.next())
    {
        employeeID=query.value("EMPLOYEE_ID").toInt();
        nic=query.value("NIC").toString();
        firstName=query.value("FIRST_NAME").toString();
        lastName=query.value("LAST_NAME").toString();
        position=query.value("POSITION").toString();
        email=query.value("EMAIL").toString();
        password=query.value("PASSWORD").toString();
        question=query.value("QUESTION").toString();
        answer=query.value("ANSWER").toString();
        this->fingerprint=query.value("FINGERPRINT").toInt();
        return true;
    }
    return false;
}

QVector<int> Employee::getAllFingerprintIDs()
{
    QVector<int> ids;
    QSqlQuery query("SELECT FINGERPRINT FROM EMPLOYEE WHERE FINGERPRINT IS NOT NULL");
    while (query.next()) {
        ids.append(query.value(0).toInt());
    }
    return ids;
}

QSqlQueryModel * Employee::getNbEmployeesByPosition()
{
    QSqlQueryModel * model=new QSqlQueryModel();
    model->setQuery("SELECT POSITION, COUNT(*) FROM EMPLOYEE GROUP BY POSITION");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Position"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Number of employees"));
    return model;
}

QStringList Employee::getOtherQuestions()
{
    QStringList questions;
    QSqlQuery query;
    query.prepare("SELECT QUESTION FROM EMPLOYEE WHERE EMPLOYEE_ID!=:id");
    query.bindValue(":id", employeeID);
    if (query.exec()) {
        while (query.next()) {
            QVariant value = query.value(0);
            if (!value.isNull() && !value.toString().isEmpty()) {
                questions << value.toString();
            }
        }
    }
    return questions;
}

bool Employee::updateEmployee()
{
    QSqlQuery query;
    query.prepare("update EMPLOYEE set NIC=:nic, FIRST_NAME=:first, LAST_NAME=:last, POSITION=:position, EMAIL=:email where EMPLOYEE_ID=:id");
    query.bindValue(":nic", nic);
    query.bindValue(":first", firstName);
    query.bindValue(":last", lastName);
    query.bindValue(":position", position);
    query.bindValue(":email", email);
    query.bindValue(":id", employeeID);
    return query.exec();
}

bool Employee::addFingerprint()
{
    QSqlQuery query;
    query.prepare("update EMPLOYEE set FINGERPRINT=:fingerprint where EMPLOYEE_ID=:id");
    query.bindValue(":fingerprint", fingerprint);
    query.bindValue(":id", employeeID);
    return query.exec();
}

bool Employee::setupEmployee()
{
    QSqlQuery query;
    query.prepare("update EMPLOYEE set PASSWORD=:password, QUESTION=:question, ANSWER=:answer where EMPLOYEE_ID=:id");
    query.bindValue(":password", password);
    query.bindValue(":question", question);
    query.bindValue(":answer", answer);
    query.bindValue(":id", employeeID);
    return query.exec();
}

bool Employee::resetPassword()
{
    QSqlQuery query;
    query.prepare("update EMPLOYEE set PASSWORD=:password where EMPLOYEE_ID=:id");
    query.bindValue(":password", password);
    query.bindValue(":id", employeeID);
    return query.exec();
}

bool Employee::deleteEmployee(int id)
{
    QSqlQuery query;
    query.prepare("delete from EMPLOYEE where EMPLOYEE_ID=:id");
    query.bindValue(":id",id);
    return query.exec();
}

void Employee::clearEmployee()
{
    employeeID=0;
    nic="";
    firstName="";
    lastName="";
    position="";
    email="";
    password="";
    question="";
    answer="";
    fingerprint=0;
}
