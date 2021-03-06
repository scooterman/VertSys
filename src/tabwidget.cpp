#include "tabwidget.h"
#include "mainwindow.h"
#include "payment.h"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setupModel();
    setupTabs();
    connect(this, SIGNAL(currentChanged(int)),
           this, SLOT(updateIdx()), Qt::UniqueConnection);

    connect(this, SIGNAL(updateClimberInfo(Climber*&)),
            static_cast<QMainWindow*>(parent->parent()), SLOT(recvClimberInfo(Climber*&)), Qt::UniqueConnection);

    connect(this, SIGNAL(updateActivateOption(int)),
            static_cast<QMainWindow*>(parent->parent()), SLOT(updateActivateOption(int)), Qt::UniqueConnection);
}

void TabWidget::setupModel()
{
    climberModel = new ClimberModel(this);
    climberModel->setTable("climber");
    climberModel->select();
    climberModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Nome"));
    climberModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Telefone"));
    climberModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    climberModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Vencimento"));
    proxyTextModel = new QSortFilterProxyModel(this);
    proxyTextModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyTextModel->setSourceModel(climberModel);


    paymentModel = new PaymentModel(this);
    paymentModel->setTable("payment");
}

void TabWidget::setupTabs()
{
    QStringList groups;
    QList<QString> charNames;
    groups << tr("Mensalistas") << tr("Todos") << tr("Diarios");
    charNames << "A" << "" << "D";

    for (int i = 0; i < groups.size(); ++i) {
        QString str = groups.at(i);

        proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(proxyTextModel);
        proxyModel->setDynamicSortFilter(true);

        QTableView *tableView = new QTableView;

        tableView->setModel(proxyModel);
        tableView->setSortingEnabled(true);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        //tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->verticalHeader()->hide();
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        //Date_Start
        tableView->hideColumn(5);
        //Status
        tableView->hideColumn(6);
        //Address
        tableView->hideColumn(2);

        //Connect the select to disable buttons
        QItemSelectionModel *sm = tableView->selectionModel();
        connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                parentWidget()->parentWidget(), SLOT(rowSelected(QModelIndex,QModelIndex)), Qt::UniqueConnection);

        proxyModel->setFilterRegExp(QRegExp(charNames[i], Qt::CaseInsensitive));
        proxyModel->setFilterKeyColumn(6);
        proxyModel->sort(4, Qt::AscendingOrder);
        addTab(tableView, str);
    }
}

void TabWidget::updateFilter(QString str)
{
    proxyTextModel->setFilterRegExp(str);
}

void TabWidget::insertClimberInDB(Climber *&climber)
{
    qDebug() << "INSERTED: " << climber->getName();
    climberModel->insertClimber(climber);
    updateIdx();
}

void TabWidget::removeClimber()
{
    int row = selectedRow();
    climberModel->removeClimber(row);
}

//FIXME: Not really working
void TabWidget::updateIdx()
{
    QTableView *temp = static_cast<QTableView*>(currentWidget());
    emit updateActivateOption(currentIndex());
    temp->setCurrentIndex(QModelIndex());
}

void TabWidget::toggleActivity()
{
    int row = selectedRow();
    climberModel->toggleActivity(row);
}

int TabWidget::selectedRow()
{
    QTableView *temp = static_cast<QTableView*>(currentWidget());
    QItemSelectionModel *selectionModel = temp->selectionModel();
    QModelIndexList indexes = selectionModel->selectedRows();
    QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(temp->model());
    QModelIndex idx;
    int row;
    foreach (idx, indexes)
        row = proxy->mapToSource(idx).row();
    return row;
}

void TabWidget::updateClimberInfo()
{
    int row = selectedRow();
    emit updateClimberInfo(climberModel->getClimber(row));
}

void TabWidget::setPayment(QDate expirationDate, double value)
{
    int row = selectedRow();
    Climber *c = climberModel->getClimber(row);
    Payment payment(c->getEmail(), QDate::currentDate(), expirationDate, value);
    if (climberModel->updateExpirationDate(row, expirationDate) && paymentModel->insertPayment(payment))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Pagamento do escalador " + c->getName() + " efetuado com sucesso!\nVencimento \
na data: " + expirationDate.toString("dd/MM/yyyy") + "\nValor: R$ " + QString::number(value));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Não foi possível efetuar o pagamento!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
    delete c;
}
