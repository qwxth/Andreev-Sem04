import re
with open('main.cpp','r') as f:
    txt=f.read()
txt=txt.replace(
    'if(QMessageBox::question(this,"","Удалить связь user_id=%1 ↔ channel_id=%2?"\n               .arg(uid).arg(cid))!=QMessageBox::Yes)return;',
    'if(QMessageBox::question(this,"",QString("Удалить связь user_id=%1 <-> channel_id=%2?").arg(uid).arg(cid))!=QMessageBox::Yes)return;'
)
with open('main.cpp','w') as f:
    f.write(txt)
