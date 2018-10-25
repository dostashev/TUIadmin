#include "statisticsengine.h"
#include <QFileDialog>

StatisticsEngine::StatisticsEngine(QObject *parent, JsonDownloader *jsonLoader)
    : QObject(parent)
    , loader(jsonLoader)
{
    if(!loader){
        loader = new JsonDownloader;
        loader->start();
    }

}

StatisticsEngine::~StatisticsEngine()
{

}

QChart *StatisticsEngine::getRecentDishOrdersChart(int id, int n_days)
{
    QDate cur_date = QDate::currentDate();
    cur_date = cur_date.addDays(-n_days);
    QBarSet *barSet = new QBarSet("Количество заказов за день");
    QStringList categories;
    for (int i = 0; i < n_days; i++) {
        cur_date = cur_date.addDays(1);
        categories << cur_date.toString("dd.MM.yy");
        *barSet << getDishOrdersByDate(id, cur_date.toString("dd-MM-yyyy"));
    }
    QBarSeries *series = new QBarSeries;
    series->append(barSet);
    QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("Статистика блюда");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    return chart;
}

QChart *StatisticsEngine::getRecentIncomeChart(int n_days)
{
    QDate cur_date = QDate::currentDate();
    cur_date = cur_date.addDays(-n_days);
    QBarSet *barSet = new QBarSet("Прибыль за день");
    QStringList categories;
    for (int i = 0; i < n_days; i++) {
        cur_date = cur_date.addDays(1);
        categories << cur_date.toString("dd.MM.yy");
        *barSet << getIncomeByDate(cur_date.toString("dd-MM-yyyy"));
    }
    QBarSeries *series = new QBarSeries;
    series->append(barSet);
    QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    return chart;
}

void StatisticsEngine::exportRecentIncomeStatsAsCSV(int n_days)
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Экспортироваь в CSV", "", "Comma-separated values (*.csv);;All files (*)");
    if(fileName.isEmpty())
        return;
    else{
        QFile csvFile(fileName);
        if(csvFile.open(QIODevice::WriteOnly)){
            QTextStream textStream (&csvFile);
            QStringList row;
            QDate cur_date = QDate::currentDate();
            cur_date = cur_date.addDays(-n_days);
            textStream << QString("Дата;Прибыль\n");
            for (int i = 0; i < n_days; i++) {
                row.clear();
                cur_date = cur_date.addDays(1);
                row << cur_date.toString("dd.MM.yy") << QString::number(getIncomeByDate(cur_date.toString("dd-MM-yyyy")));
                textStream << row.join(';')+"\n";
            }
            csvFile.close();
        }
    }
}

int StatisticsEngine::getDishOrdersByDate(int id, QString date)
{
    int cou = 0;
    QJsonArray orders = loader->getOrders();
    for (int i = 0; i < orders.size(); i++) {
        if (orders[i].toObject()["date"].toString() != date)
            continue;
        QJsonArray curDishes = orders[i].toObject()["dishes"].toArray();
        for (int j = 0; j < curDishes.size(); j++) {
            QJsonObject dish = curDishes[j].toObject();
            if (dish["id"].toInt() == id)
                cou += dish["number"].toInt();
        }
    }
    return cou;
}

double StatisticsEngine::getIncomeByDate(QString date)
{
    double income = 0;
    QJsonArray orders = loader->getOrders();
    for (int i = 0; i < orders.size(); i++) {
        if (orders[i].toObject()["date"].toString() != date)
            continue;
        QJsonArray curDishes = orders[i].toObject()["dishes"].toArray();
        for (int j = 0; j < curDishes.size(); j++) {
            QJsonObject dish = curDishes[j].toObject();
            income += loader->getDishById(dish["id"].toInt())["cost"].toDouble() * dish["number"].toInt();
        }
    }
    return income;
}
