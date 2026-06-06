#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Employee
{
    int employeeID, fingerprint;
    QString nic, firstName, lastName, position, email, password, question, answer;
public:

    //Constructors
    Employee(){}
    Employee(int, QString, QString, QString, QString, QString, QString, QString, QString, int);

    //Getters
    int getEmployeeID(){return employeeID;}
    QString getNic(){return nic;}
    QString getFirstName(){return firstName;}
    QString getLastName(){return lastName;}
    QString getPosition(){return position;}
    QString getEmail(){return email;}
    QString getPassword(){return password;}
    QString getQuestion(){return question;}
    QString getAnswer(){return answer;}
    int getFingerprint(){return fingerprint;}

    //Setters
    void setEmployeeID(int id){employeeID=id;}
    void setNic(QString nic){this->nic=nic;}
    void setFirstName(QString name){firstName=name;}
    void setLastName(QString name){lastName=name;}
    void setPosition(QString position){this->position=position;}
    void setEmail(QString email){this->email=email;}
    void setPassword(QString password){this->password=password;}
    void setQuestion(QString question){this->question=question;}
    void setAnswer(QString answer){this->answer=answer;}
    void setFingerprint(int fingerprint){this->fingerprint=fingerprint;}

    //CRUD
    bool addEmployee();
    QSqlQueryModel * displayEmployees();
    QSqlQueryModel * displaySortedEmployees();
    bool getEmployeeByID(int);
    bool getEmployeeByNic(QString);
    bool getEmployeeByEmail(QString);
    bool getEmployeeByFingerprint(int);
    QVector<int> getAllFingerprintIDs();
    QSqlQueryModel * getNbEmployeesByPosition();
    QStringList getOtherQuestions();
    bool updateEmployee();
    bool addFingerprint();
    bool setupEmployee();
    bool resetPassword();
    bool deleteEmployee(int);

    void clearEmployee();
};

#endif // EMPLOYEE_H
