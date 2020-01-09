/* This file is part of kdesdk / KBugBuster.

   Copyright 2008  KBugBuster Authors <kde-ev-board@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "messageeditor.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include <kcombobox.h>
#include <ktextedit.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kbbprefs.h"
#include "messageeditor.moc"

MessageEditor::MessageEditor( QWidget *parent )
  : KDialog(parent)
{
    setCaption( i18n("Edit Message Buttons") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    QFrame *topFrame = new QFrame( this );
    setMainWidget( topFrame );
  QBoxLayout *topLayout = new QVBoxLayout(topFrame);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(0);

  QBoxLayout *selectionLayout = new QHBoxLayout;
  topLayout->addLayout(selectionLayout);

  QLabel *selectionLabel = new QLabel(i18n("Button:"), topFrame);
  selectionLayout->addWidget(selectionLabel);

  mSelectionCombo = new KComboBox(topFrame);
  selectionLayout->addWidget(mSelectionCombo);
  connect(mSelectionCombo,SIGNAL(activated(int)),SLOT(changeMessage()));

  QPushButton *addButton = new QPushButton(i18n("Add Button..."),topFrame);
  selectionLayout->addWidget(addButton);
  connect(addButton,SIGNAL(clicked()),SLOT(addButton()));

  QPushButton *removeButton = new QPushButton(i18n("Remove Button"),topFrame);
  selectionLayout->addWidget(removeButton);
  connect(removeButton,SIGNAL(clicked()),SLOT(removeButton()));
  connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
  mMessageEdit = new KTextEdit(topFrame);
  topLayout->addWidget(mMessageEdit,1);

  updateConfig();
}

void MessageEditor::updateConfig()
{
  mMessageButtons = KBBPrefs::instance()->mMessageButtons;

  mSelectionCombo->clear();

  QMap<QString,QString>::ConstIterator it;
  for(it = mMessageButtons.constBegin();it != mMessageButtons.constEnd();++it) {
    mSelectionCombo->addItem(it.key());
  }

  updateMessage();
}

void MessageEditor::addButton()
{
  QString txt;
  txt = KInputDialog::getText(i18n("Add Message Button"),
	i18n("Enter button name:"), QString(),
	NULL, this );

  if ( !txt.isNull() ) {
    saveMessage();
    mSelectionCombo->addItem(txt);
    mMessageButtons.insert(txt,"");
    mSelectionCombo->setCurrentIndex(mSelectionCombo->count()-1);
    updateMessage();
  }

}

void MessageEditor::removeButton()
{
  int result = KMessageBox::warningContinueCancel(this,
      i18n("Remove the button %1?", mSelectionCombo->currentText()),
      i18n("Remove"), KGuiItem( i18n("Delete"), "edit-delete") );

  if (result == KMessageBox::Continue) {
    mMessageButtons.remove(mSelectionCombo->currentText());
    mSelectionCombo->removeItem(mSelectionCombo->currentIndex());
    mSelectionCombo->setCurrentIndex(0);
    updateMessage();
  }
}

void MessageEditor::changeMessage()
{
  saveMessage();
  updateMessage();
}

void MessageEditor::updateMessage()
{
  mCurrentButton = mSelectionCombo->currentText();

  mMessageEdit->setPlainText(mMessageButtons[mCurrentButton]);
}

void MessageEditor::saveMessage()
{
  mMessageButtons.insert(mCurrentButton,mMessageEdit->toPlainText());
}

void MessageEditor::slotOk()
{
  saveMessage();

  KBBPrefs::instance()->mMessageButtons = mMessageButtons;
  accept();
}
