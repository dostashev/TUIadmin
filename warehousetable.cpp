#include "warehousetable.h"
#include <QPushButton>
#include <QDate>
#include <QHeaderView>

WarehouseTable::WarehouseTable(QWidget *parent, JsonDownloader *jsonLoader)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , tableWidget(new QTableWidget)
    , loader(jsonLoader)
    , jsonSender(new JsonSender)
{
    if(!loader){
        loader = new JsonDownloader;
        connect(loader, &JsonDownloader::updateReady, this, &WarehouseTable::onNewWarehouseInfo);
        loader->start();
    }
    else{
        connect(loader, &JsonDownloader::updateReady, this, &WarehouseTable::onNewWarehouseInfo);
    }

    setupContents();
    onNewWarehouseInfo();
}

void WarehouseTable::onNewWarehouseInfo()
{
    QJsonArray tableContent = loader->getWarehouseInfo();
    tableWidget->setRowCount(tableContent.size());
    for(int i = 0; i < tableContent.size(); i++){
        QJsonObject json = tableContent[i].toObject();

        int supply_id = json["id"].toInt();
        int ingredient_id = json["ingredient_id"].toInt();
        QJsonObject ingredientJson = loader->getIngredientById(ingredient_id);
        QString expiry_time_string = ingredientJson["expiry"].toString();
        QStringList expiry_dhm = expiry_time_string.split("-");
        QTime expiry_time(expiry_dhm[1].toInt(), expiry_dhm[2].toInt());
        QString ingredient_title = ingredientJson["title"].toString();


        QString arrival_date_string = json["date"].toString();
        QStringList dmy = arrival_date_string.split("-");
        QDateTime arrival_date(QDate(dmy[2].toInt(), dmy[1].toInt(), dmy[0].toInt()),QTime(0,0));
        QDateTime expiry_date = arrival_date;
        expiry_date = expiry_date.addDays(expiry_dhm[0].toInt());
        expiry_date = expiry_date.addSecs(expiry_time.hour()*3600 + expiry_time.minute()*60);

        int expiry_minutes = (expiry_dhm[0].toInt()*24 + expiry_dhm[1].toInt())*60 + expiry_dhm[2].toInt();
        QDateTime now_date_time = QDateTime::currentDateTime();
        int passed_minutes = int((now_date_time.toSecsSinceEpoch() - arrival_date.toSecsSinceEpoch())/60);

        double red_coef = std::min(1.0,passed_minutes*1.0/expiry_minutes);
        double green_coef = std::max(0.0,1.0 - passed_minutes*1.0/expiry_minutes);

        qDebug() << red_coef << " " << green_coef << "\n";

        QColor backgroundColor(int(255*red_coef), int(255*green_coef), 0, 127);

        tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(supply_id)));
        tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(ingredient_id)));
        tableWidget->setItem(i, 2, new QTableWidgetItem(ingredient_title));
        tableWidget->setItem(i, 3, new QTableWidgetItem(json["mass"].toString()));
        tableWidget->setItem(i, 4, new QTableWidgetItem(expiry_time_string));
        tableWidget->setItem(i, 5, new QTableWidgetItem(arrival_date_string));
        tableWidget->setItem(i, 6, new QTableWidgetItem(expiry_date.toString("dd-MM-yyyy hh:mm")));

        QPushButton *deleteThisButton = new QPushButton("Удалить");
        connect(deleteThisButton, &QPushButton::clicked, [supply_id,this](){
            jsonSender->deleteSupply(supply_id);
        });
        tableWidget->setCellWidget(i,7,deleteThisButton);


        for (int j = 0; j < tableWidget->columnCount(); j++){
            if(tableWidget->item(i,j))
                tableWidget->item(i,j)->setBackgroundColor(backgroundColor);
        }
    }
}

void WarehouseTable::setupContents()
{
    mainLayout->addWidget(tableWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    tableWidget->setColumnCount(8);
    tableWidget->setFont(QFont("Sans serif",12));
    for(int i = 0; i < tableWidget->columnCount(); i++){
        tableWidget->setColumnWidth(i, 170);
    }
    tableWidget->verticalHeader()->hide();
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("ID"));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("ID ингредиента"));
    tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Название"));
    tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Количество"));
    tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Срок годности"));
    tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem("Дата поставки"));
    tableWidget->setHorizontalHeaderItem(6, new QTableWidgetItem("Дата просрочки"));
    tableWidget->setHorizontalHeaderItem(7, new QTableWidgetItem());
    //tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem("Конечный срок"));
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->setFocusPolicy(Qt::NoFocus);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setLayout(mainLayout);
}
