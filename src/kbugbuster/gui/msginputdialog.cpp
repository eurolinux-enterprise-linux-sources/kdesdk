/* This file is part of kdesdk / KBugBuster.

   Copyright 2001  Cornelius Schumacher <schumacher@kde.org>

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
#include "msginputdialog.h"

#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QTextDocument>
#include <QTextCursor>

#include <kcombobox.h>
#include <ktextedit.h>
#include <kstandardguiitem.h>
#include <klocale.h>
#include <kdebug.h>
#include <KListWidget>

#include "messageeditor.h"
#include "kbbprefs.h"
#include "bugsystem.h"
#include "bugcommand.h"

#include "msginputdialog.moc"

MsgInputDialog::MsgInputDialog(MsgInputDialog::MessageType type, const Bug &bug,
                               const Package &package, const QString &quotedMsg,
                               QWidget *parent)
  : KDialog(parent),
    mBug( bug ),
    mPackage( package ),
    mType( type )
{
    setButtons( User1|User2|Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    setButtonGuiItem( User2, KStandardGuiItem::clear() );
    setButtonGuiItem( User1, KGuiItem(i18n( "&Edit Presets..." )) );
  switch ( mType ) {
    case Close:
      setCaption( i18n("Close Bug %1", mBug.number() ) );
      break;
    case Reply:
      setCaption( i18n("Reply to Bug") );
      break;
    case ReplyPrivate:
      setCaption( i18n("Reply Privately to Bug") );
      break;
    default:
      break;
  }

  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );
  QGridLayout *gl = new QGridLayout( topFrame );

  mSplitter = new QSplitter( Qt::Horizontal, topFrame );
  gl->addWidget( mSplitter );

  QWidget *w = new QWidget( mSplitter );
  QVBoxLayout *vb = new QVBoxLayout( w );
  vb->setSpacing( spacingHint() );

  QLabel *l = new QLabel( i18n( "&Message" ), w );
  QFont f = l->font();
  f.setBold( true );
  l->setFont( f );
  vb->addWidget( l );

  mMessageEdit = new KTextEdit( w );
  mMessageEdit->setMinimumWidth( mMessageEdit->fontMetrics().width('x') * 73 );
  mMessageEdit->setLineWrapMode( QTextEdit::FixedColumnWidth );
  mMessageEdit->setLineWrapColumnOrWidth( 72 );
  l->setBuddy( mMessageEdit );
  vb->addWidget( mMessageEdit );

  // Reply currently only replies to the bug tracking system
  if ( mType == Reply && false ) {
    QHBoxLayout* rlayout = new QHBoxLayout;
    vb->addItem( rlayout );

    QLabel *rlabel = new QLabel( i18n("&Recipient:"), w );
    QFont f = rlabel->font();
    f.setBold( true );
    rlabel->setFont( f );
    rlayout->addWidget( rlabel );

    mRecipient = new KComboBox( w );
    mRecipient->insertItem( BugCommand::Normal, i18n("Normal (bugs.kde.org & Maintainer & kde-bugs-dist)") );
    mRecipient->insertItem( BugCommand::Maintonly, i18n("Maintonly (bugs.kde.org & Maintainer)") );
    mRecipient->insertItem( BugCommand::Quiet, i18n("Quiet (bugs.kde.org only)") );
    rlabel->setBuddy( mRecipient );
    rlayout->addWidget( mRecipient );

    QSpacerItem *rspacer = new QSpacerItem( 1, 1, QSizePolicy::Expanding );
    rlayout->addItem( rspacer );
  }

  w = new QWidget( mSplitter );
  vb = new QVBoxLayout( w );
  vb->setSpacing( spacingHint() );
  l = new QLabel( i18n( "&Preset Messages" ), w );
  l->setFont( f );
  vb->addWidget( l );

  mPresets = new KListWidget( w );
  updatePresets();
  l->setBuddy( mPresets );
  vb->addWidget( mPresets );

  connect( mPresets, SIGNAL( executed( QListWidgetItem * ) ),
           SLOT( slotPresetSelected( QListWidgetItem * ) ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( editPresets() ) );
  connect( this, SIGNAL( user2Clicked() ), SLOT( clearMessage() ) );
  connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
  connect(this,SIGNAL(cancelClicked()),SLOT(slotCancel()));
  mMessageEdit->setFocus();

  if ( !quotedMsg.isEmpty() )
    insertQuotedMessage( quotedMsg );

  readConfig();
}

MsgInputDialog::~MsgInputDialog()
{
  kDebug() << "MsgInputDialog::~MsgInputDialog()";
  writeConfig();
}

void MsgInputDialog::readConfig()
{
  resize( KBBPrefs::instance()->mMsgDlgWidth,
          KBBPrefs::instance()->mMsgDlgHeight );
  QList<int> sizes = KBBPrefs::instance()->mMsgDlgSplitter;
  if (!sizes.isEmpty())
    mSplitter->setSizes( sizes );
}

void MsgInputDialog::writeConfig()
{
  KBBPrefs::instance()->mMsgDlgWidth = width();
  KBBPrefs::instance()->mMsgDlgHeight = height();
  KBBPrefs::instance()->mMsgDlgSplitter = mSplitter->sizes();
}

void MsgInputDialog::updatePresets()
{
  mPresets->clear();

  QMap<QString,QString> messageButtons = KBBPrefs::instance()->mMessageButtons;

  int id = 0;
  QMap<QString,QString>::ConstIterator it;
  for( it = messageButtons.constBegin(); it != messageButtons.constEnd(); ++it )
    mPresets->insertItem( id, it.key() );
}

QString MsgInputDialog::message() const
{
  return mMessageEdit->toPlainText();
}

void MsgInputDialog::editPresets()
{
  MessageEditor *dlg = new MessageEditor(this);
  dlg->exec();
  delete dlg;

  updatePresets();
}

void MsgInputDialog::slotPresetSelected( QListWidgetItem *lbi )
{
  mMessageEdit->setPlainText( KBBPrefs::instance()->mMessageButtons[ lbi->text() ] );
}

void MsgInputDialog::clearMessage()
{
  mMessageEdit->setPlainText("");
}

void MsgInputDialog::queueCommand()
{
  switch ( mType ) {
    case Close:
      BugSystem::self()->queueCommand(
          new BugCommandClose( mBug, message(), mPackage ) );
      break;
    case Reply:
      // Reply currently only replies to the bug tracking system
      //BugSystem::self()->queueCommand( new BugCommandReply( mBug, message(), mRecipient->currentIndex() ) );
      BugSystem::self()->queueCommand( new BugCommandReply( mBug, message(), 0 ) );
      break;
    case ReplyPrivate:
      BugSystem::self()->queueCommand(
          new BugCommandReplyPrivate( mBug, mBug.submitter().email,
	                              message() ) );
      break;
    default:
      break;
  }
}

void MsgInputDialog::slotOk()
{
  queueCommand();
  accept();
}

void MsgInputDialog::slotCancel()
{
    reject();
}



void MsgInputDialog::insertQuotedMessage( const QString &msg )
{
	Q_ASSERT( mMessageEdit->lineWrapMode() == QTextEdit::FixedColumnWidth );

	const QString quotationMarker = "> ";
	const int wrapColumn = mMessageEdit->lineWrapColumnOrWidth();

	// ### Needs something more sophisticated than simplified to
	// handle quoting multiple paragraphs properly.
	QString line = msg.simplified();

	QString quotedMsg;
	while ( line.length() + quotationMarker.length() + 1 > wrapColumn ) {
		int pos = wrapColumn - quotationMarker.length() - 1;
		while ( pos > 0 && !line[ pos ].isSpace() )
			--pos;
		if ( pos == 0 )
			pos = wrapColumn;
		quotedMsg += quotationMarker + line.left( pos ) + '\n';
		line = line.mid( pos + 1 );
	}
	quotedMsg += quotationMarker + line + "\n\n";

    QTextDocument* document = new QTextDocument(mMessageEdit);
    document->setPlainText( quotedMsg );
    QTextCursor cursor(document);
    cursor.movePosition( QTextCursor::End );
    mMessageEdit->setDocument(document);
    mMessageEdit->setTextCursor(cursor);
}
