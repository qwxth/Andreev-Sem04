#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QStackedWidget>
#include <QGroupBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStyleFactory>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QVector>

static int     g_userId    = -1;
static int     g_roleId    = -1;
static QString g_username, g_firstName, g_lastName;
// Все каналы сотрудника (M:M)
static QVector<int> g_channelIds;

static QSqlDatabase getDB(){ return QSqlDatabase::database("main"); }

static QString hashPassword(const QString &p){
    return QCryptographicHash::hash(p.toUtf8(),QCryptographicHash::Sha256).toHex();
}
static void styleBtn(QPushButton *b, const QString &c="#2979FF"){
    b->setStyleSheet(QString(
        "QPushButton{background:%1;color:white;border:none;"
        "padding:7px 14px;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:%1;}").arg(c));
}
static void styleDanger(QPushButton *b){ styleBtn(b,"#D32F2F"); }
static void styleTable(QTableWidget *t){
    t->horizontalHeader()->setStretchLastSection(true);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setSortingEnabled(false);
}
static bool askCombo(QWidget *parent, const QString &title,
                     const QString &label, QComboBox *c, int &out)
{
    QDialog dlg(parent); dlg.setWindowTitle(title); dlg.setMinimumWidth(300);
    auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20);
    fl->addRow(label,c);
    auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    fl->addRow(btns);
    QObject::connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
    QObject::connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    if(dlg.exec()!=QDialog::Accepted) return false;
    out=c->currentData().toInt(); return true;
}

// Строит IN-условие для всех каналов сотрудника
static QString channelInClause(const QString &col="b.channel_id"){
    if(g_channelIds.isEmpty()) return " AND 1=0";
    QStringList ids;
    for(int id:g_channelIds) ids<<QString::number(id);
    return QString(" AND %1 IN (%2)").arg(col,ids.join(","));
}

// ===================== REGISTER =====================
class RegisterDialog : public QDialog {
    Q_OBJECT
    QLineEdit *eUser,*ePass,*eFirst,*eLast,*ePhone,*eEmail,*ePassport;
    QDateEdit *eBirth;
public:
    RegisterDialog(QWidget *p=nullptr):QDialog(p){
        setWindowTitle("Регистрация зрителя"); setMinimumWidth(360);
        auto *fl=new QFormLayout(this);
        fl->setContentsMargins(20,20,20,20); fl->setSpacing(10);
        eUser=new QLineEdit; ePass=new QLineEdit;
        ePass->setEchoMode(QLineEdit::Password);
        eFirst=new QLineEdit; eLast=new QLineEdit;
        eBirth=new QDateEdit(QDate(2000,1,1)); eBirth->setCalendarPopup(true);
        ePassport=new QLineEdit; ePhone=new QLineEdit; eEmail=new QLineEdit;
        fl->addRow("Логин:",eUser); fl->addRow("Пароль:",ePass);
        fl->addRow("Имя:",eFirst); fl->addRow("Фамилия:",eLast);
        fl->addRow("Дата рождения:",eBirth); fl->addRow("Паспорт:",ePassport);
        fl->addRow("Телефон:",ePhone); fl->addRow("Email:",eEmail);
        auto *btn=new QPushButton("Зарегистрироваться"); styleBtn(btn);
        fl->addRow(btn);
        connect(btn,&QPushButton::clicked,this,&RegisterDialog::doReg);
    }
private slots:
    void doReg(){
        if(eUser->text().isEmpty()||ePass->text().isEmpty()){
            QMessageBox::warning(this,"","Логин и пароль обязательны"); return; }
        QSqlQuery q(getDB());
        q.prepare("INSERT INTO tv.users(username,password_hash,role_id,first_name,last_name,"
                  "birth_date,passport_data,subscription_status,phone,email)"
                  " VALUES(:u,:p,3,:fn,:ln,:bd,:pp,'active',:ph,:em)");
        q.bindValue(":u",eUser->text().trimmed());
        q.bindValue(":p",hashPassword(ePass->text()));
        q.bindValue(":fn",eFirst->text()); q.bindValue(":ln",eLast->text());
        q.bindValue(":bd",eBirth->date().toString("yyyy-MM-dd"));
        q.bindValue(":pp",ePassport->text());
        q.bindValue(":ph",ePhone->text()); q.bindValue(":em",eEmail->text());
        if(q.exec()){ QMessageBox::information(this,"","Аккаунт создан!"); accept(); }
        else QMessageBox::critical(this,"Ошибка",q.lastError().text());
    }
};

// ===================== LOGIN =====================
class LoginWindow : public QWidget {
    Q_OBJECT
public:
    QLineEdit *edUser, *edPass;
    QLabel    *lblErr;
    LoginWindow(QWidget *p=nullptr):QWidget(p){
        auto *outer=new QVBoxLayout(this);
        outer->addStretch();
        auto *row=new QHBoxLayout; outer->addLayout(row); outer->addStretch();
        row->addStretch();
        auto *box=new QWidget; box->setFixedSize(380,400);
        auto *vl=new QVBoxLayout(box);
        vl->setContentsMargins(32,32,32,32); vl->setSpacing(12);
        auto *logo=new QLabel("📺 TV System"); logo->setAlignment(Qt::AlignCenter);
        logo->setStyleSheet("font-size:22px;font-weight:bold;");
        auto *sub=new QLabel("Система управления телевещанием");
        sub->setAlignment(Qt::AlignCenter); sub->setStyleSheet("color:gray;font-size:11px;");
        vl->addWidget(logo); vl->addWidget(sub); vl->addSpacing(8);
        edUser=new QLineEdit; edUser->setPlaceholderText("Логин"); edUser->setMinimumHeight(34);
        edPass=new QLineEdit; edPass->setPlaceholderText("Пароль");
        edPass->setEchoMode(QLineEdit::Password); edPass->setMinimumHeight(34);
        lblErr=new QLabel; lblErr->setStyleSheet("color:red;font-size:11px;");
        lblErr->setAlignment(Qt::AlignCenter);
        vl->addWidget(edUser); vl->addWidget(edPass); vl->addWidget(lblErr);
        auto *btnIn=new QPushButton("Войти"); styleBtn(btnIn);
        auto *btnReg=new QPushButton("Создать аккаунт");
        btnReg->setStyleSheet("QPushButton{background:transparent;color:#2979FF;"
            "border:1px solid #2979FF;padding:7px 14px;border-radius:4px;}"
            "QPushButton:hover{background:#e3f2fd;}");
        vl->addWidget(btnIn); vl->addWidget(btnReg);
        row->addWidget(box); row->addStretch();
        connect(btnIn, &QPushButton::clicked,this,&LoginWindow::doLogin);
        connect(edPass,&QLineEdit::returnPressed,this,&LoginWindow::doLogin);
        connect(btnReg,&QPushButton::clicked,this,[this]{RegisterDialog d(this);d.exec();});
    }
signals:
    void loginSuccess();
private slots:
    void doLogin(){
        lblErr->clear();
        QSqlQuery q(getDB());
        q.prepare("SELECT user_id,role_id,first_name,last_name,subscription_status"
                  " FROM tv.users WHERE username=:u AND password_hash=:p");
        q.bindValue(":u",edUser->text().trimmed());
        q.bindValue(":p",hashPassword(edPass->text()));
        q.exec();
        if(!q.next()){ lblErr->setText("Неверный логин или пароль"); return; }
        if(q.value(4).toString()=="blocked"){ lblErr->setText("Аккаунт заблокирован"); return; }
        g_userId   =q.value(0).toInt();
        g_roleId   =q.value(1).toInt();
        g_firstName=q.value(2).toString();
        g_lastName =q.value(3).toString();
        g_username =edUser->text().trimmed();
        g_channelIds.clear();
        // Загружаем ВСЕ каналы сотрудника
        if(g_roleId==2){
            QSqlQuery qc(getDB());
            qc.prepare("SELECT channel_id FROM tv.channel_employees WHERE user_id=:id");
            qc.bindValue(":id",g_userId); qc.exec();
            while(qc.next()) g_channelIds.append(qc.value(0).toInt());
        }
        emit loginSuccess();
    }
};

// ===================== CHANNELS =====================
class ChannelsTab : public QWidget {
    Q_OBJECT
    bool m_ro, m_my;
    QTableWidget *table;
    QLineEdit    *edSearch;
public:
    ChannelsTab(bool ro=false, bool my=false, QWidget *p=nullptr)
        :QWidget(p),m_ro(ro),m_my(my)
    {
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        edSearch=new QLineEdit; edSearch->setPlaceholderText("Поиск по названию...");
        auto *btnS=new QPushButton("Найти");    styleBtn(btnS);
        auto *btnR=new QPushButton("Обновить"); styleBtn(btnR,"#388E3C");
        hl->addWidget(edSearch); hl->addWidget(btnS); hl->addStretch(); hl->addWidget(btnR);
        if(!m_ro){
            auto *btnA=new QPushButton("Добавить");      styleBtn(btnA);
            auto *btnE=new QPushButton("Редактировать"); styleBtn(btnE,"#F57F17");
            auto *btnD=new QPushButton("Удалить");       styleDanger(btnD);
            hl->addWidget(btnA); hl->addWidget(btnE); hl->addWidget(btnD);
            connect(btnA,&QPushButton::clicked,this,[this]{editCh(-1);});
            connect(btnE,&QPushButton::clicked,this,&ChannelsTab::onEdit);
            connect(btnD,&QPushButton::clicked,this,&ChannelsTab::onDel);
        } else if(m_my){
            auto *btnE=new QPushButton("Редактировать контакты"); styleBtn(btnE,"#F57F17");
            hl->addWidget(btnE);
            connect(btnE,&QPushButton::clicked,this,&ChannelsTab::onEdit);
        }
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnS,    &QPushButton::clicked,this,[this]{load(edSearch->text());});
        connect(btnR,    &QPushButton::clicked,this,[this]{load();});
        connect(edSearch,&QLineEdit::returnPressed,this,[this]{load(edSearch->text());});
        load();
    }
    void load(const QString &s=""){
        QString sql="SELECT channel_id,full_name,short_name,country,city,owner,"
                    "contact_phone,contact_email FROM tv.channels WHERE 1=1";
        if(!s.isEmpty())
            sql+=" AND (full_name ILIKE '%"+s+"%' OR short_name ILIKE '%"+s+"%')";
        if(m_my){
            if(g_channelIds.isEmpty()){ table->setRowCount(0); return; }
            QStringList ids; for(int id:g_channelIds) ids<<QString::number(id);
            sql+=QString(" AND channel_id IN (%1)").arg(ids.join(","));
        }
        sql+=" ORDER BY channel_id";
        QSqlQuery q(getDB()); q.exec(sql);
        table->setColumnCount(8);
        table->setHorizontalHeaderLabels(
            {"ID","Полное название","Краткое","Страна","Город","Владелец","Телефон","Email"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            for(int i=0;i<8;i++) table->setItem(r,i,new QTableWidgetItem(q.value(i).toString()));
        }
        table->resizeColumnsToContents();
    }
    void editCh(int id){
        QDialog dlg(this);
        dlg.setWindowTitle(id<0?"Добавить канал":"Редактировать канал");
        dlg.setMinimumWidth(400);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20); fl->setSpacing(8);
        auto *eFull=new QLineEdit,*eShort=new QLineEdit,*eCountry=new QLineEdit;
        auto *eCity=new QLineEdit,*eOwner=new QLineEdit;
        auto *ePhone=new QLineEdit,*eEmail=new QLineEdit;
        if(id>0){
            QSqlQuery q(getDB());
            q.prepare("SELECT full_name,short_name,country,city,owner,contact_phone,contact_email"
                      " FROM tv.channels WHERE channel_id=:id");
            q.bindValue(":id",id); q.exec();
            if(q.next()){
                eFull->setText(q.value(0).toString()); eShort->setText(q.value(1).toString());
                eCountry->setText(q.value(2).toString()); eCity->setText(q.value(3).toString());
                eOwner->setText(q.value(4).toString()); ePhone->setText(q.value(5).toString());
                eEmail->setText(q.value(6).toString());
            }
        }
        bool co=m_ro&&m_my;
        if(!co){
            fl->addRow("Полное название:",eFull); fl->addRow("Краткое:",eShort);
            fl->addRow("Страна:",eCountry); fl->addRow("Город:",eCity); fl->addRow("Владелец:",eOwner);
        }
        fl->addRow("Телефон:",ePhone); fl->addRow("Email:",eEmail);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted) return;
        QSqlQuery q(getDB());
        if(id<0){
            q.prepare("INSERT INTO tv.channels(full_name,short_name,country,city,owner,"
                      "contact_phone,contact_email) VALUES(:fn,:sn,:co,:ci,:ow,:ph,:em)");
            q.bindValue(":fn",eFull->text()); q.bindValue(":sn",eShort->text());
            q.bindValue(":co",eCountry->text()); q.bindValue(":ci",eCity->text());
            q.bindValue(":ow",eOwner->text());
        } else {
            if(co)
                q.prepare("UPDATE tv.channels SET contact_phone=:ph,contact_email=:em"
                          " WHERE channel_id=:id");
            else{
                q.prepare("UPDATE tv.channels SET full_name=:fn,short_name=:sn,country=:co,"
                          "city=:ci,owner=:ow,contact_phone=:ph,contact_email=:em"
                          " WHERE channel_id=:id");
                q.bindValue(":fn",eFull->text()); q.bindValue(":sn",eShort->text());
                q.bindValue(":co",eCountry->text()); q.bindValue(":ci",eCity->text());
                q.bindValue(":ow",eOwner->text());
            }
            q.bindValue(":id",id);
        }
        q.bindValue(":ph",ePhone->text()); q.bindValue(":em",eEmail->text());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
private slots:
    void onEdit(){
        int r=table->currentRow();
        if(r<0){QMessageBox::warning(this,"","Выберите строку");return;}
        editCh(table->item(r,0)->text().toInt());
    }
    void onDel(){
        int r=table->currentRow(); if(r<0)return;
        if(QMessageBox::question(this,"","Удалить канал?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.channels WHERE channel_id=:id");
        q.bindValue(":id",table->item(r,0)->text().toInt());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
};

// ===================== CHANNEL EMPLOYEES =====================
class ChannelEmployeesTab : public QWidget {
    Q_OBJECT
    QTableWidget *table;
public:
    ChannelEmployeesTab(QWidget *p=nullptr):QWidget(p){
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        auto *btnA=new QPushButton("Добавить связь");  styleBtn(btnA);
        auto *btnD=new QPushButton("Удалить связь");   styleDanger(btnD);
        auto *btnR=new QPushButton("Обновить");        styleBtn(btnR,"#388E3C");
        hl->addWidget(btnA); hl->addWidget(btnD); hl->addStretch(); hl->addWidget(btnR);
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnA,&QPushButton::clicked,this,&ChannelEmployeesTab::onAdd);
        connect(btnD,&QPushButton::clicked,this,&ChannelEmployeesTab::onDel);
        connect(btnR,&QPushButton::clicked,this,[this]{load();});
        load();
    }
    void load(){
        QString sql=
            "SELECT ce.user_id,"
            "       u.username,"
            "       u.last_name||' '||u.first_name AS fio,"
            "       r.role_name,"
            "       ce.channel_id,"
            "       c.full_name,"
            "       c.short_name "
            "FROM tv.channel_employees ce "
            "JOIN tv.users    u ON ce.user_id   = u.user_id "
            "JOIN tv.roles    r ON u.role_id     = r.role_id "
            "JOIN tv.channels c ON ce.channel_id = c.channel_id "
            "ORDER BY ce.channel_id, ce.user_id";
        QSqlQuery q(getDB()); q.exec(sql);
        table->setColumnCount(7);
        table->setHorizontalHeaderLabels(
            {"user_id","Логин","ФИО","Роль","channel_id","Канал (полное)","Краткое"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            for(int i=0;i<7;i++) table->setItem(r,i,new QTableWidgetItem(q.value(i).toString()));
        }
        table->resizeColumnsToContents();
    }
private slots:
    void onAdd(){
        QDialog dlg(this); dlg.setWindowTitle("Добавить сотрудника канала");
        dlg.setMinimumWidth(380);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20); fl->setSpacing(8);
        auto *cUser=new QComboBox;
        auto *cChan=new QComboBox;
        QSqlQuery qu(getDB());
        qu.exec("SELECT user_id, username||' ('||last_name||' '||first_name||')'"
                " FROM tv.users ORDER BY username");
        while(qu.next()) cUser->addItem(qu.value(1).toString(),qu.value(0));
        QSqlQuery qc(getDB());
        qc.exec("SELECT channel_id,full_name FROM tv.channels ORDER BY full_name");
        while(qc.next()) cChan->addItem(qc.value(1).toString(),qc.value(0));
        auto *chkRole=new QComboBox;
        chkRole->addItem("Оставить текущую роль",-1);
        chkRole->addItem("Назначить роль channel_employee (role_id=2)",2);
        fl->addRow("Пользователь:",cUser);
        fl->addRow("Канал:",cChan);
        fl->addRow("Роль после:",chkRole);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted) return;
        int uid=cUser->currentData().toInt();
        int cid=cChan->currentData().toInt();
        int newRole=chkRole->currentData().toInt();
        QSqlQuery ins(getDB());
        ins.prepare("INSERT INTO tv.channel_employees(user_id,channel_id)"
                    " VALUES(:u,:c) ON CONFLICT DO NOTHING");
        ins.bindValue(":u",uid); ins.bindValue(":c",cid);
        if(!ins.exec()){
            QMessageBox::critical(this,"Ошибка",ins.lastError().text()); return;
        }
        if(newRole==2){
            QSqlQuery ur(getDB());
            ur.prepare("UPDATE tv.users SET role_id=2 WHERE user_id=:id AND role_id!=1");
            ur.bindValue(":id",uid); ur.exec();
        }
        QMessageBox::information(this,"","Связь добавлена");
        load();
    }
    void onDel(){
        int r=table->currentRow();
        if(r<0){QMessageBox::warning(this,"","Выберите строку");return;}
        int uid=table->item(r,0)->text().toInt();
        int cid=table->item(r,4)->text().toInt();
        if(QMessageBox::question(this,"",
               QString("Удалить связь user_id=%1 <-> channel_id=%2?")
               .arg(uid).arg(cid))!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.channel_employees WHERE user_id=:u AND channel_id=:c");
        q.bindValue(":u",uid); q.bindValue(":c",cid);
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
};

// ===================== GENRES =====================
class GenresTab : public QWidget {
    Q_OBJECT
    QTableWidget *table;
public:
    GenresTab(QWidget *p=nullptr):QWidget(p){
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        auto *btnA=new QPushButton("Добавить");     styleBtn(btnA);
        auto *btnE=new QPushButton("Редактировать");styleBtn(btnE,"#F57F17");
        auto *btnD=new QPushButton("Удалить");      styleDanger(btnD);
        auto *btnR=new QPushButton("Обновить");     styleBtn(btnR,"#388E3C");
        hl->addWidget(btnA);hl->addWidget(btnE);hl->addWidget(btnD);
        hl->addStretch();hl->addWidget(btnR);
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnA,&QPushButton::clicked,this,[this]{edit(-1);});
        connect(btnE,&QPushButton::clicked,this,&GenresTab::onEdit);
        connect(btnD,&QPushButton::clicked,this,&GenresTab::onDel);
        connect(btnR,&QPushButton::clicked,this,[this]{load();});
        load();
    }
    void load(){
        QSqlQuery q(getDB()); q.exec("SELECT genre_id,genre_name FROM tv.genres ORDER BY genre_id");
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({"ID","Название"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            table->setItem(r,0,new QTableWidgetItem(q.value(0).toString()));
            table->setItem(r,1,new QTableWidgetItem(q.value(1).toString()));
        }
        table->resizeColumnsToContents();
    }
    void edit(int id){
        QDialog dlg(this); dlg.setWindowTitle("Жанр"); dlg.setMinimumWidth(280);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20);
        auto *e=new QLineEdit;
        if(id>0){
            QSqlQuery q(getDB());
            q.prepare("SELECT genre_name FROM tv.genres WHERE genre_id=:id");
            q.bindValue(":id",id); q.exec();
            if(q.next()) e->setText(q.value(0).toString());
        }
        fl->addRow("Название:",e);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        if(id<0) q.prepare("INSERT INTO tv.genres(genre_name) VALUES(:n)");
        else{ q.prepare("UPDATE tv.genres SET genre_name=:n WHERE genre_id=:id"); q.bindValue(":id",id); }
        q.bindValue(":n",e->text());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
private slots:
    void onEdit(){int r=table->currentRow();if(r<0)return;edit(table->item(r,0)->text().toInt());}
    void onDel(){
        int r=table->currentRow(); if(r<0)return;
        if(QMessageBox::question(this,"","Удалить?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.genres WHERE genre_id=:id");
        q.bindValue(":id",table->item(r,0)->text().toInt());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
};

// ===================== ROLES =====================
class RolesTab : public QWidget {
    Q_OBJECT
    QTableWidget *table;
public:
    RolesTab(QWidget *p=nullptr):QWidget(p){
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        auto *btnA=new QPushButton("Добавить");     styleBtn(btnA);
        auto *btnE=new QPushButton("Редактировать");styleBtn(btnE,"#F57F17");
        auto *btnD=new QPushButton("Удалить");      styleDanger(btnD);
        auto *btnR=new QPushButton("Обновить");     styleBtn(btnR,"#388E3C");
        hl->addWidget(btnA);hl->addWidget(btnE);hl->addWidget(btnD);
        hl->addStretch();hl->addWidget(btnR);
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnA,&QPushButton::clicked,this,[this]{edit(-1);});
        connect(btnE,&QPushButton::clicked,this,&RolesTab::onEdit);
        connect(btnD,&QPushButton::clicked,this,&RolesTab::onDel);
        connect(btnR,&QPushButton::clicked,this,[this]{load();});
        load();
    }
    void load(){
        QSqlQuery q(getDB()); q.exec("SELECT role_id,role_name FROM tv.roles ORDER BY role_id");
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({"ID","Роль"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            table->setItem(r,0,new QTableWidgetItem(q.value(0).toString()));
            table->setItem(r,1,new QTableWidgetItem(q.value(1).toString()));
        }
        table->resizeColumnsToContents();
    }
    void edit(int id){
        QDialog dlg(this); dlg.setWindowTitle("Роль"); dlg.setMinimumWidth(280);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20);
        auto *e=new QLineEdit;
        if(id>0){
            QSqlQuery q(getDB());
            q.prepare("SELECT role_name FROM tv.roles WHERE role_id=:id");
            q.bindValue(":id",id); q.exec();
            if(q.next()) e->setText(q.value(0).toString());
        }
        fl->addRow("Название:",e);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        if(id<0) q.prepare("INSERT INTO tv.roles(role_name) VALUES(:n)");
        else{ q.prepare("UPDATE tv.roles SET role_name=:n WHERE role_id=:id"); q.bindValue(":id",id); }
        q.bindValue(":n",e->text());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
private slots:
    void onEdit(){int r=table->currentRow();if(r<0)return;edit(table->item(r,0)->text().toInt());}
    void onDel(){
        int r=table->currentRow(); if(r<0)return;
        if(QMessageBox::question(this,"","Удалить?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.roles WHERE role_id=:id");
        q.bindValue(":id",table->item(r,0)->text().toInt());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
};

// ===================== BROADCASTS =====================
class BroadcastsTab : public QWidget {
    Q_OBJECT
    bool m_ro, m_my;
    QTableWidget   *table;
    QLineEdit      *edSearch;
    QDateEdit      *edFrom, *edTo;
    QDoubleSpinBox *edRMin, *edRMax;
    QComboBox      *cGenreF;
public:
    BroadcastsTab(bool ro=false, bool my=false, QWidget *p=nullptr)
        :QWidget(p),m_ro(ro),m_my(my)
    {
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        edSearch=new QLineEdit; edSearch->setPlaceholderText("ID или название...");
        edSearch->setMinimumWidth(130);
        cGenreF=new QComboBox; cGenreF->setMinimumWidth(120);
        cGenreF->addItem("Все жанры",0);
        {QSqlQuery qg(getDB()); qg.exec("SELECT genre_id,genre_name FROM tv.genres ORDER BY genre_name");
        while(qg.next()) cGenreF->addItem(qg.value(1).toString(),qg.value(0));}
        edFrom=new QDateEdit(QDate::currentDate().addMonths(-3)); edFrom->setCalendarPopup(true);
        edTo  =new QDateEdit(QDate::currentDate().addMonths(3));  edTo->setCalendarPopup(true);
        edRMin=new QDoubleSpinBox; edRMin->setRange(0,10); edRMin->setValue(0);
        edRMax=new QDoubleSpinBox; edRMax->setRange(0,10); edRMax->setValue(10);
        auto *btnS=new QPushButton("Найти");    styleBtn(btnS);
        auto *btnR=new QPushButton("Обновить"); styleBtn(btnR,"#388E3C");
        hl->addWidget(new QLabel("Поиск:")); hl->addWidget(edSearch);
        hl->addWidget(new QLabel("Жанр:"));  hl->addWidget(cGenreF);
        hl->addWidget(new QLabel("С:"));     hl->addWidget(edFrom);
        hl->addWidget(new QLabel("По:"));    hl->addWidget(edTo);
        hl->addWidget(new QLabel("Рейт:"));  hl->addWidget(edRMin);
        hl->addWidget(new QLabel("-"));      hl->addWidget(edRMax);
        hl->addWidget(btnS); hl->addStretch(); hl->addWidget(btnR);
        if(!m_ro){
            auto *btnA=new QPushButton("Добавить");      styleBtn(btnA);
            auto *btnE=new QPushButton("Редактировать"); styleBtn(btnE,"#F57F17");
            auto *btnD=new QPushButton("Удалить");       styleDanger(btnD);
            hl->addWidget(btnA); hl->addWidget(btnE); hl->addWidget(btnD);
            connect(btnA,&QPushButton::clicked,this,[this]{editBr(-1);});
            connect(btnE,&QPushButton::clicked,this,&BroadcastsTab::onEdit);
            connect(btnD,&QPushButton::clicked,this,&BroadcastsTab::onDel);
        }
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnS,    &QPushButton::clicked,this,[this]{load(edSearch->text());});
        connect(btnR,    &QPushButton::clicked,this,[this]{load();});
        connect(edSearch,&QLineEdit::returnPressed,this,[this]{load(edSearch->text());});
        load();
    }
    void load(const QString &s=""){
        QString sql=
            "SELECT b.broadcast_id,b.title,g.genre_name,b.duration_minutes,"
            "b.air_date_time,b.rating,c.short_name "
            "FROM tv.broadcasts b "
            "LEFT JOIN tv.genres   g ON b.genre_id   =g.genre_id "
            "LEFT JOIN tv.channels c ON b.channel_id  =c.channel_id WHERE 1=1";
        if(!s.isEmpty()){
            bool ok; int id=s.toInt(&ok);
            if(ok) sql+=QString(" AND b.broadcast_id=%1").arg(id);
            else   sql+=" AND b.title ILIKE '%"+s+"%'";
        }
        int gid=cGenreF->currentData().toInt();
        if(gid>0) sql+=QString(" AND b.genre_id=%1").arg(gid);
        sql+=QString(" AND b.air_date_time>='%1' AND b.air_date_time<='%2'")
             .arg(edFrom->date().toString("yyyy-MM-dd"),edTo->date().toString("yyyy-MM-dd"));
        sql+=QString(" AND b.rating>=%1 AND b.rating<=%2")
             .arg(edRMin->value()).arg(edRMax->value());
        // Фильтр по ВСЕМ каналам сотрудника
        if(m_my) sql+=channelInClause("b.channel_id");
        sql+=" ORDER BY b.air_date_time";
        QSqlQuery q(getDB()); q.exec(sql);
        table->setColumnCount(7);
        table->setHorizontalHeaderLabels(
            {"ID","Название","Жанр","Длит.(мин)","Дата эфира","Рейтинг","Канал"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            for(int i=0;i<7;i++) table->setItem(r,i,new QTableWidgetItem(q.value(i).toString()));
        }
        table->resizeColumnsToContents();
    }
    void editBr(int id){
        QDialog dlg(this);
        dlg.setWindowTitle(id<0?"Добавить передачу":"Редактировать передачу");
        dlg.setMinimumWidth(420);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20); fl->setSpacing(8);
        auto *eTitle=new QLineEdit;
        auto *cGen  =new QComboBox;
        {QSqlQuery qg(getDB()); qg.exec("SELECT genre_id,genre_name FROM tv.genres ORDER BY genre_name");
        while(qg.next()) cGen->addItem(qg.value(1).toString(),qg.value(0));}
        auto *eDur =new QSpinBox; eDur->setRange(1,600);
        auto *eDT  =new QDateTimeEdit(QDateTime::currentDateTime()); eDT->setCalendarPopup(true);
        auto *eRat =new QDoubleSpinBox; eRat->setRange(0,10); eRat->setSingleStep(0.1);
        auto *cChan=new QComboBox;
        // Для сотрудника — только его каналы
        if(m_my&&!g_channelIds.isEmpty()){
            QStringList ids; for(int id2:g_channelIds) ids<<QString::number(id2);
            QSqlQuery qc(getDB());
            qc.exec("SELECT channel_id,short_name FROM tv.channels"
                    " WHERE channel_id IN ("+ids.join(",")+") ORDER BY short_name");
            while(qc.next()) cChan->addItem(qc.value(1).toString(),qc.value(0));
        } else {
            QSqlQuery qc(getDB()); qc.exec("SELECT channel_id,short_name FROM tv.channels ORDER BY short_name");
            while(qc.next()) cChan->addItem(qc.value(1).toString(),qc.value(0));
        }
        if(id>0){
            QSqlQuery q(getDB());
            q.prepare("SELECT title,genre_id,duration_minutes,air_date_time,rating,channel_id"
                      " FROM tv.broadcasts WHERE broadcast_id=:id");
            q.bindValue(":id",id); q.exec();
            if(q.next()){
                eTitle->setText(q.value(0).toString());
                for(int i=0;i<cGen->count();i++)
                    if(cGen->itemData(i).toInt()==q.value(1).toInt()) cGen->setCurrentIndex(i);
                eDur->setValue(q.value(2).toInt());
                eDT->setDateTime(q.value(3).toDateTime());
                eRat->setValue(q.value(4).toDouble());
                for(int i=0;i<cChan->count();i++)
                    if(cChan->itemData(i).toInt()==q.value(5).toInt()) cChan->setCurrentIndex(i);
            }
        }
        fl->addRow("Название:",eTitle); fl->addRow("Жанр:",cGen); fl->addRow("Длит.(мин):",eDur);
        fl->addRow("Дата/время эфира:",eDT); fl->addRow("Рейтинг:",eRat); fl->addRow("Канал:",cChan);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        if(id<0)
            q.prepare("INSERT INTO tv.broadcasts(title,genre_id,duration_minutes,"
                      "air_date_time,rating,channel_id) VALUES(:t,:g,:d,:dt,:r,:c)");
        else{
            q.prepare("UPDATE tv.broadcasts SET title=:t,genre_id=:g,duration_minutes=:d,"
                      "air_date_time=:dt,rating=:r,channel_id=:c WHERE broadcast_id=:id");
            q.bindValue(":id",id);
        }
        q.bindValue(":t",eTitle->text()); q.bindValue(":g",cGen->currentData());
        q.bindValue(":d",eDur->value());  q.bindValue(":dt",eDT->dateTime());
        q.bindValue(":r",eRat->value());  q.bindValue(":c",cChan->currentData());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
private slots:
    void onEdit(){
        int r=table->currentRow();
        if(r<0){QMessageBox::warning(this,"","Выберите строку");return;}
        editBr(table->item(r,0)->text().toInt());
    }
    void onDel(){
        int r=table->currentRow(); if(r<0)return;
        if(QMessageBox::question(this,"","Удалить передачу?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.broadcasts WHERE broadcast_id=:id");
        q.bindValue(":id",table->item(r,0)->text().toInt());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
};

// ===================== USERS (admin) =====================
class UsersTab : public QWidget {
    Q_OBJECT
    QTableWidget *table;
    QLineEdit    *edSearch;
    QComboBox    *cRoleF;
public:
    UsersTab(QWidget *p=nullptr):QWidget(p){
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        edSearch=new QLineEdit; edSearch->setPlaceholderText("Поиск по фамилии...");
        cRoleF=new QComboBox; cRoleF->addItem("Все роли",0);
        {QSqlQuery q(getDB()); q.exec("SELECT role_id,role_name FROM tv.roles ORDER BY role_id");
        while(q.next()) cRoleF->addItem(q.value(1).toString(),q.value(0));}
        auto *btnS  =new QPushButton("Найти");         styleBtn(btnS);
        auto *btnR  =new QPushButton("Обновить");      styleBtn(btnR,"#388E3C");
        auto *btnE  =new QPushButton("Редактировать"); styleBtn(btnE,"#F57F17");
        auto *btnRol=new QPushButton("Сменить роль");  styleBtn(btnRol,"#6A1B9A");
        auto *btnBlk=new QPushButton("Блок/Разблок");  styleBtn(btnBlk,"#E65100");
        hl->addWidget(edSearch); hl->addWidget(new QLabel("Роль:")); hl->addWidget(cRoleF);
        hl->addWidget(btnS); hl->addStretch(); hl->addWidget(btnR);
        hl->addWidget(btnE); hl->addWidget(btnRol); hl->addWidget(btnBlk);
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnS,  &QPushButton::clicked,this,
                [this]{load(edSearch->text(),cRoleF->currentData().toInt());});
        connect(btnR,  &QPushButton::clicked,this,[this]{load();});
        connect(btnE,  &QPushButton::clicked,this,&UsersTab::onEdit);
        connect(btnRol,&QPushButton::clicked,this,&UsersTab::onChangeRole);
        connect(btnBlk,&QPushButton::clicked,this,&UsersTab::onToggleBlock);
        connect(edSearch,&QLineEdit::returnPressed,this,
                [this]{load(edSearch->text(),cRoleF->currentData().toInt());});
        load();
    }
    void load(const QString &s="", int roleId=0){
        QString sql=
            "SELECT u.user_id,u.username,r.role_name,u.last_name,u.first_name,"
            "u.subscription_status,u.phone,u.email,"
            "COALESCE((SELECT string_agg(c.short_name,', ')"
            " FROM tv.channel_employees ce"
            " JOIN tv.channels c ON ce.channel_id=c.channel_id"
            " WHERE ce.user_id=u.user_id),'—') AS channels "
            "FROM tv.users u "
            "JOIN tv.roles r ON u.role_id=r.role_id WHERE 1=1";
        if(!s.isEmpty())  sql+=" AND u.last_name ILIKE '%"+s+"%'";
        if(roleId>0)       sql+=QString(" AND u.role_id=%1").arg(roleId);
        sql+=" ORDER BY u.user_id";
        QSqlQuery q(getDB()); q.exec(sql);
        table->setColumnCount(9);
        table->setHorizontalHeaderLabels(
            {"ID","Логин","Роль","Фамилия","Имя","Статус","Телефон","Email","Каналы"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            for(int i=0;i<9;i++) table->setItem(r,i,new QTableWidgetItem(q.value(i).toString()));
        }
        table->resizeColumnsToContents();
    }
    int selectedId(){
        int r=table->currentRow();
        if(r<0){QMessageBox::warning(this,"","Выберите строку");return -1;}
        return table->item(r,0)->text().toInt();
    }
    void editUser(int id){
        QDialog dlg(this); dlg.setWindowTitle("Редактировать пользователя"); dlg.setMinimumWidth(400);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20); fl->setSpacing(8);
        auto *eLast=new QLineEdit,*eFirst=new QLineEdit,*ePhone=new QLineEdit;
        auto *eEmail=new QLineEdit,*ePassport=new QLineEdit;
        auto *eBirth=new QDateEdit(QDate(1990,1,1)); eBirth->setCalendarPopup(true);
        auto *cSub=new QComboBox; cSub->addItems({"active","inactive","blocked","VIP"});
        QSqlQuery q(getDB());
        q.prepare("SELECT last_name,first_name,birth_date,passport_data,"
                  "subscription_status,phone,email FROM tv.users WHERE user_id=:id");
        q.bindValue(":id",id); q.exec();
        if(q.next()){
            eLast->setText(q.value(0).toString()); eFirst->setText(q.value(1).toString());
            eBirth->setDate(q.value(2).toDate());  ePassport->setText(q.value(3).toString());
            cSub->setCurrentText(q.value(4).toString());
            ePhone->setText(q.value(5).toString()); eEmail->setText(q.value(6).toString());
        }
        fl->addRow("Фамилия:",eLast); fl->addRow("Имя:",eFirst);
        fl->addRow("Дата рождения:",eBirth); fl->addRow("Паспорт:",ePassport);
        fl->addRow("Статус подписки:",cSub);
        fl->addRow("Телефон:",ePhone); fl->addRow("Email:",eEmail);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery uq(getDB());
        uq.prepare("UPDATE tv.users SET last_name=:ln,first_name=:fn,birth_date=:bd,"
                   "passport_data=:pp,subscription_status=:ss,phone=:ph,email=:em"
                   " WHERE user_id=:id");
        uq.bindValue(":ln",eLast->text()); uq.bindValue(":fn",eFirst->text());
        uq.bindValue(":bd",eBirth->date()); uq.bindValue(":pp",ePassport->text());
        uq.bindValue(":ss",cSub->currentText());
        uq.bindValue(":ph",ePhone->text()); uq.bindValue(":em",eEmail->text());
        uq.bindValue(":id",id);
        if(!uq.exec()) QMessageBox::critical(this,"Ошибка",uq.lastError().text());
        load();
    }
private slots:
    void onEdit(){int id=selectedId();if(id<0)return;editUser(id);}
    void onChangeRole(){
        int id=selectedId(); if(id<0)return;
        auto *c=new QComboBox;
        QSqlQuery qr(getDB()); qr.exec("SELECT role_id,role_name FROM tv.roles ORDER BY role_id");
        while(qr.next()) c->addItem(qr.value(1).toString(),qr.value(0));
        int rid; if(!askCombo(this,"Сменить роль","Новая роль:",c,rid))return;
        QSqlQuery q(getDB());
        q.prepare("UPDATE tv.users SET role_id=:r WHERE user_id=:id");
        q.bindValue(":r",rid); q.bindValue(":id",id);
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        else QMessageBox::information(this,"","Роль обновлена");
        load();
    }
    void onToggleBlock(){
        int id=selectedId(); if(id<0)return;
        QSqlQuery q(getDB());
        q.prepare("SELECT subscription_status FROM tv.users WHERE user_id=:id");
        q.bindValue(":id",id); q.exec();
        if(!q.next())return;
        QString nxt=(q.value(0).toString()=="blocked")?"active":"blocked";
        QSqlQuery uq(getDB());
        uq.prepare("UPDATE tv.users SET subscription_status=:s WHERE user_id=:id");
        uq.bindValue(":s",nxt); uq.bindValue(":id",id);
        if(!uq.exec()) QMessageBox::critical(this,"Ошибка",uq.lastError().text());
        else QMessageBox::information(this,"",QString("Статус -> %1").arg(nxt));
        load();
    }
};

// ===================== VIEW HISTORY =====================
class ViewHistoryTab : public QWidget {
    Q_OBJECT
    bool m_ro, m_my;
    QTableWidget *table;
    QLineEdit    *edSearch;
    QComboBox    *cVSt;
public:
    ViewHistoryTab(bool ro=false, bool my=false, QWidget *p=nullptr)
        :QWidget(p),m_ro(ro),m_my(my)
    {
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        edSearch=new QLineEdit; edSearch->setMinimumWidth(160);
        edSearch->setPlaceholderText(m_my?"Поиск по дате...":"user_id зрителя...");
        cVSt=new QComboBox;
        cVSt->addItems({"Все","Просмотрено полностью","Не досмотрено"});
        auto *btnS=new QPushButton("Найти");    styleBtn(btnS);
        auto *btnR=new QPushButton("Обновить"); styleBtn(btnR,"#388E3C");
        hl->addWidget(edSearch);
        if(m_my){ hl->addWidget(new QLabel("Статус:")); hl->addWidget(cVSt); }
        hl->addWidget(btnS); hl->addStretch(); hl->addWidget(btnR);
        if(!m_ro){
            auto *btnA=new QPushButton("Добавить");      styleBtn(btnA);
            auto *btnE=new QPushButton("Редактировать"); styleBtn(btnE,"#F57F17");
            auto *btnD=new QPushButton("Удалить");       styleDanger(btnD);
            hl->addWidget(btnA); hl->addWidget(btnE); hl->addWidget(btnD);
            connect(btnA,&QPushButton::clicked,this,[this]{editV(-1);});
            connect(btnE,&QPushButton::clicked,this,&ViewHistoryTab::onEdit);
            connect(btnD,&QPushButton::clicked,this,&ViewHistoryTab::onDel);
        }
        if(m_my){
            auto *btnCl=new QPushButton("Очистить историю"); styleDanger(btnCl);
            hl->addWidget(btnCl);
            connect(btnCl,&QPushButton::clicked,this,&ViewHistoryTab::clearH);
        }
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnS,&QPushButton::clicked,this,
                [this]{load(edSearch->text(),cVSt->currentText());});
        connect(btnR,&QPushButton::clicked,this,[this]{load();});
        load();
    }
    void load(const QString &s="", const QString &vs="Все"){
        QString sql=
            "SELECT vh.view_id,u.last_name||' '||u.first_name,b.title,"
            "vh.view_date,vh.watch_duration_minutes,vh.device,b.duration_minutes "
            "FROM tv.view_history vh "
            "JOIN tv.users      u ON vh.user_id     =u.user_id "
            "JOIN tv.broadcasts b ON vh.broadcast_id=b.broadcast_id WHERE 1=1";
        if(m_my) sql+=QString(" AND vh.user_id=%1").arg(g_userId);
        if(!s.isEmpty()){
            if(m_my) sql+=" AND CAST(vh.view_date AS TEXT) ILIKE '%"+s+"%'";
            else{ bool ok; int id=s.toInt(&ok);
                if(ok) sql+=QString(" AND vh.user_id=%1").arg(id); }
        }
        if(vs=="Просмотрено полностью") sql+=" AND vh.watch_duration_minutes>=b.duration_minutes";
        else if(vs=="Не досмотрено")    sql+=" AND vh.watch_duration_minutes<b.duration_minutes";
        sql+=" ORDER BY vh.view_date DESC";
        QSqlQuery q(getDB()); q.exec(sql);
        table->setColumnCount(7);
        table->setHorizontalHeaderLabels(
            {"ID","Зритель","Передача","Дата","Длит.просм.","Устройство","Длит.перед."});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            for(int i=0;i<7;i++) table->setItem(r,i,new QTableWidgetItem(q.value(i).toString()));
        }
        table->resizeColumnsToContents();
    }
    void editV(int id){
        QDialog dlg(this);
        dlg.setWindowTitle(id<0?"Добавить просмотр":"Редактировать просмотр");
        dlg.setMinimumWidth(420);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20); fl->setSpacing(8);
        auto *cUser=new QComboBox, *cBr=new QComboBox;
        auto *eDT =new QDateTimeEdit(QDateTime::currentDateTime()); eDT->setCalendarPopup(true);
        auto *eDur=new QSpinBox; eDur->setRange(0,600);
        auto *eDev=new QLineEdit;
        {QSqlQuery qu(getDB());
        qu.exec("SELECT user_id,last_name||' '||first_name FROM tv.users WHERE role_id=3");
        while(qu.next()) cUser->addItem(qu.value(1).toString(),qu.value(0));}
        {QSqlQuery qb(getDB());
        qb.exec("SELECT broadcast_id,title FROM tv.broadcasts ORDER BY air_date_time DESC");
        while(qb.next()) cBr->addItem(qb.value(1).toString(),qb.value(0));}
        if(id>0){
            QSqlQuery q(getDB());
            q.prepare("SELECT user_id,broadcast_id,view_date,watch_duration_minutes,device"
                      " FROM tv.view_history WHERE view_id=:id");
            q.bindValue(":id",id); q.exec();
            if(q.next()){
                for(int i=0;i<cUser->count();i++)
                    if(cUser->itemData(i).toInt()==q.value(0).toInt()) cUser->setCurrentIndex(i);
                for(int i=0;i<cBr->count();i++)
                    if(cBr->itemData(i).toInt()==q.value(1).toInt()) cBr->setCurrentIndex(i);
                eDT->setDateTime(q.value(2).toDateTime());
                eDur->setValue(q.value(3).toInt());
                eDev->setText(q.value(4).toString());
            }
        }
        fl->addRow("Зритель:",cUser); fl->addRow("Передача:",cBr);
        fl->addRow("Дата:",eDT); fl->addRow("Длит.(мин):",eDur); fl->addRow("Устройство:",eDev);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        if(id<0)
            q.prepare("INSERT INTO tv.view_history(user_id,broadcast_id,view_date,"
                      "watch_duration_minutes,device) VALUES(:u,:b,:dt,:d,:dev)");
        else{
            q.prepare("UPDATE tv.view_history SET user_id=:u,broadcast_id=:b,"
                      "view_date=:dt,watch_duration_minutes=:d,device=:dev WHERE view_id=:id");
            q.bindValue(":id",id);
        }
        q.bindValue(":u",cUser->currentData()); q.bindValue(":b",cBr->currentData());
        q.bindValue(":dt",eDT->dateTime()); q.bindValue(":d",eDur->value());
        q.bindValue(":dev",eDev->text());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
private slots:
    void onEdit(){int r=table->currentRow();if(r<0)return;editV(table->item(r,0)->text().toInt());}
    void onDel(){
        int r=table->currentRow(); if(r<0)return;
        if(QMessageBox::question(this,"","Удалить?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.view_history WHERE view_id=:id");
        q.bindValue(":id",table->item(r,0)->text().toInt());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
    void clearH(){
        if(QMessageBox::question(this,"","Очистить всю историю?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.view_history WHERE user_id=:id");
        q.bindValue(":id",g_userId);
        if(q.exec()) QMessageBox::information(this,"","Очищено");
        else QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
};

// ===================== SUBSCRIPTION HISTORY =====================
class SubHistoryTab : public QWidget {
    Q_OBJECT
    bool m_admin, m_my;
    QTableWidget *table;
public:
    SubHistoryTab(bool admin=false, bool my=false, QWidget *p=nullptr)
        :QWidget(p),m_admin(admin),m_my(my)
    {
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        auto *btnR=new QPushButton("Обновить"); styleBtn(btnR,"#388E3C");
        if(m_admin){
            auto *btnA=new QPushButton("Добавить");      styleBtn(btnA);
            auto *btnE=new QPushButton("Редактировать"); styleBtn(btnE,"#F57F17");
            auto *btnD=new QPushButton("Удалить");       styleDanger(btnD);
            hl->addWidget(btnA); hl->addWidget(btnE); hl->addWidget(btnD);
            connect(btnA,&QPushButton::clicked,this,[this]{editS(-1);});
            connect(btnE,&QPushButton::clicked,this,&SubHistoryTab::onEdit);
            connect(btnD,&QPushButton::clicked,this,&SubHistoryTab::onDel);
        }
        if(m_my){
            auto *btnBuy=new QPushButton("Купить подписку"); styleBtn(btnBuy,"#388E3C");
            auto *btnCan=new QPushButton("Отменить");        styleDanger(btnCan);
            hl->addWidget(btnBuy); hl->addWidget(btnCan);
            connect(btnBuy,&QPushButton::clicked,this,&SubHistoryTab::buySub);
            connect(btnCan,&QPushButton::clicked,this,&SubHistoryTab::cancelSub);
        }
        hl->addStretch(); hl->addWidget(btnR);
        vl->addLayout(hl);
        table=new QTableWidget; styleTable(table); vl->addWidget(table);
        connect(btnR,&QPushButton::clicked,this,[this]{load();});
        load();
    }
    void load(){
        QString sql=
            "SELECT sh.history_id,u.last_name||' '||u.first_name,"
            "sh.change_date,sh.old_status,sh.new_status,sh.comment "
            "FROM tv.subscription_history sh "
            "JOIN tv.users u ON sh.user_id=u.user_id";
        if(m_my) sql+=QString(" WHERE sh.user_id=%1").arg(g_userId);
        sql+=" ORDER BY sh.change_date DESC";
        QSqlQuery q(getDB()); q.exec(sql);
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels(
            {"ID","Пользователь","Дата","Старый","Новый","Комментарий"});
        table->setRowCount(0);
        while(q.next()){
            int r=table->rowCount(); table->insertRow(r);
            for(int i=0;i<6;i++) table->setItem(r,i,new QTableWidgetItem(q.value(i).toString()));
        }
        table->resizeColumnsToContents();
    }
    void editS(int id){
        QDialog dlg(this); dlg.setWindowTitle("Запись подписки"); dlg.setMinimumWidth(380);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20); fl->setSpacing(8);
        auto *cUser=new QComboBox;
        {QSqlQuery q(getDB()); q.exec("SELECT user_id,last_name||' '||first_name FROM tv.users");
        while(q.next()) cUser->addItem(q.value(1).toString(),q.value(0));}
        auto *eDT =new QDateTimeEdit(QDateTime::currentDateTime()); eDT->setCalendarPopup(true);
        auto *cOld=new QComboBox; cOld->addItems({"active","inactive","blocked","VIP"});
        auto *cNew=new QComboBox; cNew->addItems({"active","inactive","blocked","VIP"});
        auto *eCom=new QLineEdit;
        if(id>0){
            QSqlQuery q(getDB());
            q.prepare("SELECT user_id,change_date,old_status,new_status,comment"
                      " FROM tv.subscription_history WHERE history_id=:id");
            q.bindValue(":id",id); q.exec();
            if(q.next()){
                for(int i=0;i<cUser->count();i++)
                    if(cUser->itemData(i).toInt()==q.value(0).toInt()) cUser->setCurrentIndex(i);
                eDT->setDateTime(q.value(1).toDateTime());
                cOld->setCurrentText(q.value(2).toString());
                cNew->setCurrentText(q.value(3).toString());
                eCom->setText(q.value(4).toString());
            }
        }
        fl->addRow("Пользователь:",cUser); fl->addRow("Дата:",eDT);
        fl->addRow("Старый:",cOld); fl->addRow("Новый:",cNew); fl->addRow("Комментарий:",eCom);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        if(id<0)
            q.prepare("INSERT INTO tv.subscription_history(user_id,change_date,"
                      "old_status,new_status,comment) VALUES(:u,:dt,:os,:ns,:c)");
        else{
            q.prepare("UPDATE tv.subscription_history SET user_id=:u,change_date=:dt,"
                      "old_status=:os,new_status=:ns,comment=:c WHERE history_id=:id");
            q.bindValue(":id",id);
        }
        q.bindValue(":u",cUser->currentData()); q.bindValue(":dt",eDT->dateTime());
        q.bindValue(":os",cOld->currentText()); q.bindValue(":ns",cNew->currentText());
        q.bindValue(":c",eCom->text());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
private:
    void doChange(const QString &newSt, const QString &comment){
        QSqlDatabase db=getDB();
        db.transaction();
        QSqlQuery qs(db);
        qs.prepare("SELECT subscription_status FROM tv.users WHERE user_id=:id");
        qs.bindValue(":id",g_userId);
        if(!qs.exec()||!qs.next()){ db.rollback(); return; }
        QString old=qs.value(0).toString();
        QSqlQuery qu(db);
        qu.prepare("UPDATE tv.users SET subscription_status=:s WHERE user_id=:id");
        qu.bindValue(":s",newSt); qu.bindValue(":id",g_userId);
        if(!qu.exec()){ db.rollback();
            QMessageBox::critical(this,"Ошибка",qu.lastError().text()); return; }
        QSqlQuery qh(db);
        qh.prepare("INSERT INTO tv.subscription_history"
                   "(user_id,change_date,old_status,new_status,comment)"
                   " VALUES(:u,NOW(),:os,:ns,:c)");
        qh.bindValue(":u",g_userId); qh.bindValue(":os",old);
        qh.bindValue(":ns",newSt);   qh.bindValue(":c",comment);
        if(!qh.exec()){ db.rollback();
            QMessageBox::critical(this,"Ошибка",qh.lastError().text()); return; }
        db.commit();
        QMessageBox::information(this,"","Подписка: "+newSt);
        load();
    }
private slots:
    void onEdit(){int r=table->currentRow();if(r<0)return;editS(table->item(r,0)->text().toInt());}
    void onDel(){
        int r=table->currentRow(); if(r<0)return;
        if(QMessageBox::question(this,"","Удалить?")!=QMessageBox::Yes)return;
        QSqlQuery q(getDB());
        q.prepare("DELETE FROM tv.subscription_history WHERE history_id=:id");
        q.bindValue(":id",table->item(r,0)->text().toInt());
        if(!q.exec()) QMessageBox::critical(this,"Ошибка",q.lastError().text());
        load();
    }
    void buySub(){
        QDialog dlg(this); dlg.setWindowTitle("Купить подписку"); dlg.setMinimumWidth(260);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20);
        auto *c=new QComboBox; c->addItems({"active","VIP"}); fl->addRow("Тип:",c);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        doChange(c->currentText(),"Покупка подписки");
    }
    void cancelSub(){ doChange("inactive","Отмена подписки"); }
};

// ===================== PROFILE =====================
class ProfileTab : public QWidget {
    Q_OBJECT
public:
    ProfileTab(QWidget *p=nullptr):QWidget(p){
        auto *vl=new QVBoxLayout(this);
        auto *grp=new QGroupBox("Личные данные");
        auto *fl=new QFormLayout(grp); fl->setSpacing(10);
        QSqlQuery q(getDB());
        q.prepare("SELECT last_name,first_name,phone,email,passport_data"
                  " FROM tv.users WHERE user_id=:id");
        q.bindValue(":id",g_userId); q.exec();
        QString ln,fn,ph,em,pp;
        if(q.next()){
            ln=q.value(0).toString(); fn=q.value(1).toString();
            ph=q.value(2).toString(); em=q.value(3).toString(); pp=q.value(4).toString();
        }
        auto *eLast    =new QLineEdit(ln);
        auto *eFirst   =new QLineEdit(fn);
        auto *ePhone   =new QLineEdit(ph);
        auto *eEmail   =new QLineEdit(em);
        auto *ePassport=new QLineEdit(pp);
        fl->addRow("Фамилия:",eLast); fl->addRow("Имя:",eFirst);
        fl->addRow("Телефон:",ePhone); fl->addRow("Email:",eEmail);
        fl->addRow("Паспорт:",ePassport);
        auto *btn=new QPushButton("Сохранить"); styleBtn(btn); fl->addRow(btn);
        connect(btn,&QPushButton::clicked,this,[=]{
            QSqlQuery uq(getDB());
            uq.prepare("UPDATE tv.users SET last_name=:ln,first_name=:fn,"
                       "phone=:ph,email=:em,passport_data=:pp WHERE user_id=:id");
            uq.bindValue(":ln",eLast->text());    uq.bindValue(":fn",eFirst->text());
            uq.bindValue(":ph",ePhone->text());   uq.bindValue(":em",eEmail->text());
            uq.bindValue(":pp",ePassport->text()); uq.bindValue(":id",g_userId);
            if(uq.exec()) QMessageBox::information(this,"","Сохранено");
            else QMessageBox::critical(this,"Ошибка",uq.lastError().text());
        });
        vl->addWidget(grp); vl->addStretch();
    }
};

// ===================== REPORTS =====================
class ReportsTab : public QWidget {
    Q_OBJECT
    int m_role;
    QTextEdit *report;
public:
    ReportsTab(int role=1, QWidget *p=nullptr):QWidget(p),m_role(role){
        auto *vl=new QVBoxLayout(this);
        auto *hl=new QHBoxLayout;
        auto addBtn=[&](const QString &name, auto slot){
            auto *btn=new QPushButton(name); styleBtn(btn,"#6A1B9A");
            hl->addWidget(btn); connect(btn,&QPushButton::clicked,this,slot);
        };
        if(m_role==1){
            addBtn("Каналы по городу",   [this]{rptChCity();});
            addBtn("Канал + передачи",   [this]{rptChBr();});
            addBtn("Зрители по статусу", [this]{rptVwSt();});
            addBtn("Зритель + просмотры",[this]{rptVwH();});
            addBtn("Передача + зрители", [this]{rptBrVw();});
        } else if(m_role==2){
            addBtn("Передачи + просмотры",[this]{rptMyChBrVw();});
            addBtn("Зрители по передачам",[this]{rptMyChVwBr();});
        } else if(m_role==3){
            addBtn("Мои передачи",  [this]{rptMyBr();});
            addBtn("История счёта", [this]{rptMyAcc();});
        }
        auto *btnPr=new QPushButton("Печать"); styleBtn(btnPr,"#00695C");
        hl->addStretch(); hl->addWidget(btnPr);
        connect(btnPr,&QPushButton::clicked,this,&ReportsTab::doPrint);
        vl->addLayout(hl);
        report=new QTextEdit; report->setReadOnly(true);
        report->setStyleSheet("font-family:monospace;font-size:13px;");
        vl->addWidget(report);
    }
private slots:
    void doPrint(){
        if(report->toPlainText().isEmpty()){
            QMessageBox::warning(this,"","Сначала сформируйте отчёт"); return; }
        QPrinter pr; QPrintDialog dlg(&pr,this);
        if(dlg.exec()==QDialog::Accepted){
            QTextDocument doc; doc.setPlainText(report->toPlainText()); doc.print(&pr);
        }
    }
private:
    QComboBox* mkUserC(){
        auto *c=new QComboBox;
        QSqlQuery q(getDB());
        q.exec("SELECT user_id,last_name||' '||first_name FROM tv.users WHERE role_id=3");
        while(q.next()) c->addItem(q.value(1).toString(),q.value(0));
        return c;
    }
    QComboBox* mkChC(){
        auto *c=new QComboBox;
        QSqlQuery q(getDB());
        q.exec("SELECT channel_id,full_name FROM tv.channels ORDER BY full_name");
        while(q.next()) c->addItem(q.value(1).toString(),q.value(0));
        return c;
    }
    QComboBox* mkBrC(){
        auto *c=new QComboBox;
        QSqlQuery q(getDB());
        q.exec("SELECT broadcast_id,title FROM tv.broadcasts ORDER BY title");
        while(q.next()) c->addItem(q.value(1).toString(),q.value(0));
        return c;
    }
    void rptChCity(){
        QDialog dlg(this); dlg.setWindowTitle("Город"); dlg.setMinimumWidth(260);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20);
        auto *e=new QLineEdit; fl->addRow("Город:",e);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        q.prepare("SELECT channel_id,full_name,short_name,country,owner,"
                  "contact_phone,contact_email FROM tv.channels WHERE city ILIKE :c");
        q.bindValue(":c","%"+e->text()+"%"); q.exec();
        QString t=QString("=== Каналы города: %1 ===\n\n").arg(e->text());
        while(q.next())
            t+=QString("ID:%1 | %2 (%3) | %4 | Вл:%5\nТел:%6 | %7\n\n")
               .arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString(),
                    q.value(3).toString(),q.value(4).toString(),
                    q.value(5).toString(),q.value(6).toString());
        report->setPlainText(t);
    }
    void rptChBr(){
        int cid; auto *c=mkChC();
        if(!askCombo(this,"Канал","Канал:",c,cid))return;
        QSqlQuery q(getDB());
        q.prepare("SELECT full_name,short_name,country,city,owner,contact_phone,contact_email"
                  " FROM tv.channels WHERE channel_id=:id");
        q.bindValue(":id",cid); q.exec();
        QString t;
        if(q.next())
            t=QString("=== %1 (%2) ===\n%3/%4 | Вл:%5 | Тел:%6 | %7\n\n--- Передачи ---\n")
              .arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString(),
                   q.value(3).toString(),q.value(4).toString(),
                   q.value(5).toString(),q.value(6).toString());
        QSqlQuery qb(getDB());
        qb.prepare("SELECT b.broadcast_id,b.title,g.genre_name,b.air_date_time,"
                   "b.duration_minutes,b.rating FROM tv.broadcasts b "
                   "LEFT JOIN tv.genres g ON b.genre_id=g.genre_id "
                   "WHERE b.channel_id=:id ORDER BY b.air_date_time");
        qb.bindValue(":id",cid); qb.exec();
        while(qb.next())
            t+=QString("[%1] %2 | %3 | %4 | %5min | R:%6\n")
               .arg(qb.value(0).toString(),qb.value(1).toString(),qb.value(2).toString(),
                    qb.value(3).toString(),qb.value(4).toString(),qb.value(5).toString());
        report->setPlainText(t);
    }
    void rptVwSt(){
        QDialog dlg(this); dlg.setWindowTitle("Статус"); dlg.setMinimumWidth(240);
        auto *fl=new QFormLayout(&dlg); fl->setContentsMargins(20,20,20,20);
        auto *c=new QComboBox; c->addItems({"active","inactive","blocked","VIP"});
        fl->addRow("Статус:",c);
        auto *btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        fl->addRow(btns);
        connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
        connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if(dlg.exec()!=QDialog::Accepted)return;
        QSqlQuery q(getDB());
        q.prepare("SELECT user_id,last_name,first_name,phone,email FROM tv.users"
                  " WHERE role_id=3 AND subscription_status=:s");
        q.bindValue(":s",c->currentText()); q.exec();
        QString t=QString("=== Зрители [%1] ===\n\n").arg(c->currentText());
        while(q.next())
            t+=QString("ID:%1 | %2 %3 | %4 | %5\n")
               .arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString(),
                    q.value(3).toString(),q.value(4).toString());
        report->setPlainText(t);
    }
    void rptVwH(){
        int uid; auto *c=mkUserC();
        if(!askCombo(this,"Зритель","Зритель:",c,uid))return;
        QSqlQuery q(getDB());
        q.prepare("SELECT last_name,first_name,phone,email,subscription_status"
                  " FROM tv.users WHERE user_id=:id");
        q.bindValue(":id",uid); q.exec();
        QString t;
        if(q.next())
            t=QString("=== %1 %2 | %3 | %4 | %5 ===\n\n")
              .arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString(),
                   q.value(3).toString(),q.value(4).toString());
        QSqlQuery qh(getDB());
        qh.prepare("SELECT vh.view_date,b.title,c.short_name,vh.watch_duration_minutes,vh.device"
                   " FROM tv.view_history vh"
                   " JOIN tv.broadcasts b ON vh.broadcast_id=b.broadcast_id"
                   " JOIN tv.channels   c ON b.channel_id=c.channel_id"
                   " WHERE vh.user_id=:id ORDER BY vh.view_date DESC");
        qh.bindValue(":id",uid); qh.exec();
        while(qh.next())
            t+=QString("%1 | %2 [%3] | %4min | %5\n")
               .arg(qh.value(0).toString(),qh.value(1).toString(),qh.value(2).toString(),
                    qh.value(3).toString(),qh.value(4).toString());
        report->setPlainText(t);
    }
    void rptBrVw(){
        int bid; auto *c=mkBrC();
        if(!askCombo(this,"Передача","Передача:",c,bid))return;
        QSqlQuery q(getDB());
        q.prepare("SELECT b.title,g.genre_name,b.air_date_time,b.rating,c.short_name"
                  " FROM tv.broadcasts b"
                  " LEFT JOIN tv.genres   g ON b.genre_id=g.genre_id"
                  " JOIN tv.channels      c ON b.channel_id=c.channel_id"
                  " WHERE b.broadcast_id=:id");
        q.bindValue(":id",bid); q.exec();
        QString t;
        if(q.next())
            t=QString("=== %1 | %2 | %3 | R:%4 | %5 ===\n\n--- Зрители ---\n")
              .arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString(),
                   q.value(3).toString(),q.value(4).toString());
        QSqlQuery qv(getDB());
        qv.prepare("SELECT u.last_name||' '||u.first_name,vh.view_date,"
                   "vh.watch_duration_minutes,vh.device"
                   " FROM tv.view_history vh"
                   " JOIN tv.users u ON vh.user_id=u.user_id"
                   " WHERE vh.broadcast_id=:id ORDER BY vh.view_date");
        qv.bindValue(":id",bid); qv.exec();
        int cnt=0;
        while(qv.next()){cnt++;
            t+=QString("%1 | %2 | %3min | %4\n")
               .arg(qv.value(0).toString(),qv.value(1).toString(),
                    qv.value(2).toString(),qv.value(3).toString());}
        t+=QString("\nВсего: %1").arg(cnt);
        report->setPlainText(t);
    }
    void rptMyChBrVw(){
        if(g_channelIds.isEmpty()){ report->setPlainText("Нет каналов"); return; }
        QStringList ids; for(int id:g_channelIds) ids<<QString::number(id);
        QSqlQuery q(getDB());
        q.exec("SELECT b.broadcast_id,b.title,c.short_name,b.air_date_time,b.rating,"
               "(SELECT COUNT(*) FROM tv.view_history vh WHERE vh.broadcast_id=b.broadcast_id)"
               " FROM tv.broadcasts b"
               " JOIN tv.channels c ON b.channel_id=c.channel_id"
               " WHERE b.channel_id IN ("+ids.join(",")+") ORDER BY c.short_name,b.air_date_time");
        QString t="=== Передачи моих каналов ===\n\n";
        QString lastCh;
        while(q.next()){
            if(q.value(2).toString()!=lastCh){
                lastCh=q.value(2).toString();
                t+=QString("\n--- %1 ---\n").arg(lastCh);
            }
            t+=QString("[%1] %2 | %3 | R:%4 | Просм:%5\n")
               .arg(q.value(0).toString(),q.value(1).toString(),q.value(3).toString(),
                    q.value(4).toString(),q.value(5).toString());
        }
        report->setPlainText(t);
    }
    void rptMyChVwBr(){
        if(g_channelIds.isEmpty()){ report->setPlainText("Нет каналов"); return; }
        QStringList ids; for(int id:g_channelIds) ids<<QString::number(id);
        QSqlQuery q(getDB());
        q.exec("SELECT c.short_name,b.title,u.last_name||' '||u.first_name,"
               "vh.view_date,vh.watch_duration_minutes"
               " FROM tv.view_history vh"
               " JOIN tv.broadcasts b ON vh.broadcast_id=b.broadcast_id"
               " JOIN tv.channels   c ON b.channel_id=c.channel_id"
               " JOIN tv.users      u ON vh.user_id=u.user_id"
               " WHERE b.channel_id IN ("+ids.join(",")+
               ") ORDER BY c.short_name,b.title,vh.view_date");
        QString t="=== Зрители по передачам моих каналов ===\n\n";
        QString lastCh,lastBr;
        while(q.next()){
            if(q.value(0).toString()!=lastCh){
                lastCh=q.value(0).toString(); lastBr="";
                t+=QString("\n=== %1 ===\n").arg(lastCh);
            }
            if(q.value(1).toString()!=lastBr){
                lastBr=q.value(1).toString();
                t+=QString("  [%1]\n").arg(lastBr);
            }
            t+=QString("    %1 | %2 | %3min\n")
               .arg(q.value(2).toString(),q.value(3).toString(),q.value(4).toString());
        }
        report->setPlainText(t);
    }
    void rptMyBr(){
        QSqlQuery q(getDB());
        q.prepare("SELECT b.title,c.short_name,vh.view_date,vh.watch_duration_minutes,g.genre_name"
                  " FROM tv.view_history vh"
                  " JOIN tv.broadcasts b ON vh.broadcast_id=b.broadcast_id"
                  " JOIN tv.channels   c ON b.channel_id=c.channel_id"
                  " LEFT JOIN tv.genres g ON b.genre_id=g.genre_id"
                  " WHERE vh.user_id=:id ORDER BY vh.view_date DESC");
        q.bindValue(":id",g_userId); q.exec();
        QString t="=== Мои просмотренные передачи ===\n\n";
        while(q.next())
            t+=QString("%1 [%2] | %3 | %4 | %5min\n")
               .arg(q.value(0).toString(),q.value(1).toString(),q.value(2).toString(),
                    q.value(3).toString(),q.value(4).toString());
        report->setPlainText(t);
    }
    void rptMyAcc(){
        QSqlQuery q(getDB());
        q.prepare("SELECT change_date,old_status,new_status,comment"
                  " FROM tv.subscription_history WHERE user_id=:id ORDER BY change_date DESC");
        q.bindValue(":id",g_userId); q.exec();
        QString t="=== История счёта ===\n\n";
        while(q.next())
            t+=QString("%1 | %2 -> %3 | %4\n")
               .arg(q.value(0).toString(),q.value(1).toString(),
                    q.value(2).toString(),q.value(3).toString());
        report->setPlainText(t);
    }
};

// ===================== MAIN WINDOW =====================
class MainWindow : public QMainWindow {
    Q_OBJECT
    QStackedWidget *stack;
    LoginWindow    *loginWin;
    QWidget        *mainPage=nullptr;
public:
    MainWindow(QWidget *p=nullptr):QMainWindow(p){
        setWindowTitle("TV System"); resize(1280,800);
        stack=new QStackedWidget(this); setCentralWidget(stack);
        loginWin=new LoginWindow; stack->addWidget(loginWin);
        connect(loginWin,&LoginWindow::loginSuccess,this,&MainWindow::buildMain);
    }
private slots:
    void buildMain(){
        if(mainPage){stack->removeWidget(mainPage);delete mainPage;mainPage=nullptr;}
        mainPage=new QWidget;
        auto *vl=new QVBoxLayout(mainPage);
        vl->setContentsMargins(0,0,0,0); vl->setSpacing(0);
        // header
        auto *hdr=new QWidget; hdr->setFixedHeight(46);
        hdr->setStyleSheet("background:#f0f0f0;border-bottom:1px solid #ccc;");
        auto *hl=new QHBoxLayout(hdr); hl->setContentsMargins(12,0,12,0);
        auto *lblT=new QLabel("TV System");
        lblT->setStyleSheet("font-size:16px;font-weight:bold;");
        QString rs;
        if(g_roleId==1)      rs="Администратор";
        else if(g_roleId==2){
            rs=QString("Сотрудник канала (%1 кан.)").arg(g_channelIds.size());
        }
        else if(g_roleId==3) rs="Зритель";
        else                 rs=QString("Роль #%1").arg(g_roleId);
        auto *lblU=new QLabel(QString("%1 %2  [%3]").arg(g_firstName,g_lastName,rs));
        auto *btnOut=new QPushButton("Выйти"); styleDanger(btnOut);
        hl->addWidget(lblT); hl->addStretch(); hl->addWidget(lblU);
        hl->addSpacing(8); hl->addWidget(btnOut);
        connect(btnOut,&QPushButton::clicked,this,&MainWindow::doLogout);
        vl->addWidget(hdr);
        auto *tabs=new QTabWidget;
        if(g_roleId==1){
            tabs->addTab(new ChannelsTab(false,false),    "Телеканалы");
            tabs->addTab(new BroadcastsTab(false,false),  "Передачи");
            tabs->addTab(new UsersTab,                    "Пользователи");
            tabs->addTab(new ChannelEmployeesTab,         "Сотрудники каналов");
            tabs->addTab(new ViewHistoryTab(false,false), "История просмотров");
            tabs->addTab(new SubHistoryTab(true,false),   "История подписок");
            tabs->addTab(new GenresTab,                   "Жанры");
            tabs->addTab(new RolesTab,                    "Роли");
            tabs->addTab(new ReportsTab(1),               "Отчёты");
        } else if(g_roleId==2){
            tabs->addTab(new ChannelsTab(true,true),      "Мои каналы");
            tabs->addTab(new BroadcastsTab(false,true),   "Передачи каналов");
            tabs->addTab(new ViewHistoryTab(true,false),  "История просмотров");
            tabs->addTab(new ReportsTab(2),               "Отчёты");
        } else if(g_roleId==3){
            tabs->addTab(new ChannelsTab(true,false),     "Телеканалы");
            tabs->addTab(new BroadcastsTab(true,false),   "Передачи");
            tabs->addTab(new ViewHistoryTab(true,true),   "Мои просмотры");
            tabs->addTab(new SubHistoryTab(false,true),   "Подписка");
            tabs->addTab(new ProfileTab,                  "Профиль");
            tabs->addTab(new ReportsTab(3),               "Отчёты");
        } else {
            tabs->addTab(new ChannelsTab(true,false),     "Телеканалы");
            tabs->addTab(new BroadcastsTab(true,false),   "Передачи");
        }
        vl->addWidget(tabs);
        stack->addWidget(mainPage);
        stack->setCurrentWidget(mainPage);
    }
    void doLogout(){
        g_userId=-1; g_roleId=-1; g_channelIds.clear();
        g_username=g_firstName=g_lastName="";
        loginWin->edUser->clear(); loginWin->edPass->clear(); loginWin->lblErr->clear();
        stack->setCurrentWidget(loginWin);
        if(mainPage){stack->removeWidget(mainPage);delete mainPage;mainPage=nullptr;}
    }
};

int main(int argc,char *argv[]){
    QApplication app(argc,argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    QSqlDatabase db=QSqlDatabase::addDatabase("QPSQL","main");
    db.setHostName("localhost"); db.setPort(5432);
    db.setDatabaseName("tv_system");
    db.setUserName("postgres"); db.setPassword("labs");
    if(!db.open()){
        QMessageBox::critical(nullptr,"Ошибка подключения",db.lastError().text());
        return 1;
    }
    MainWindow w; w.show();
    return app.exec();
}
#include "main.moc"
