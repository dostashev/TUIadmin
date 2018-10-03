#ifndef ORDERTABLE_H
#define ORDERTABLE_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTreeWidget>
#include <QHeaderView>
#include <QJsonObject>
#include <QJsonArray>
#include "orderwidget.h"
#include "dishwidget.h"
#include "jsondownloader.h"

class OrderTable : public QWidget
{
    Q_OBJECT
public:
    explicit OrderTable(QWidget *parent = nullptr, JsonDownloader *loader = nullptr);

    void reloadOrders();
    void setDisplayedOrder(int displayedOrder);
signals:

public slots:
    void onNewOrders();
private:
    QVBoxLayout *mainLayout;
    QTreeWidget *treeWidget;
    QScrollArea * scrollArea;
    JsonDownloader *loader;
    QJsonArray orders;
    int displayed_order_id;
};

#endif // ORDERTABLE_H
