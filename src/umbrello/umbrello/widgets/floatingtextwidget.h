/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FLOATINGTEXTWIDGET_H
#define FLOATINGTEXTWIDGET_H

#include "basictypes.h"
#include "umlwidget.h"

class FloatingTextWidgetController;
class LinkWidget;
class UMLScene;

/**
 * @short Displays a line of text or an operation.
 *
 * This is a multipurpose class. In its simplest form it will display a
 * line of text.
 * It can also be setup to be the text for an operation with regard to the
 * @ref MessageWidget on the sequence diagram.
 * It is also used for the text required for an association.
 *
 * The differences between all these different uses will be the popup menu
 * that is associated with it.
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class FloatingTextWidget : public UMLWidget
{
    Q_OBJECT
public:
    friend class FloatingTextWidgetController;

    explicit FloatingTextWidget(UMLScene * scene, Uml::TextRole::Enum role = Uml::TextRole::Floating,
                                const QString& text = "", Uml::ID::Type id = Uml::ID::None);
    virtual ~FloatingTextWidget();

    QString text() const;
    void setText(const QString &t);

    void setTextcmd(const QString &t);

    QString preText() const;
    void setPreText(const QString &t);

    QString postText() const;
    void setPostText(const QString &t);

    QString displayText() const;

    void showChangeTextDialog();
    void showOperationDialog();
    virtual void showPropertiesDialog();

    LinkWidget* link() const;
    void setLink(LinkWidget * l);

    bool activate( IDChangeLog* ChangeLog = 0 );

    Uml::TextRole::Enum textRole() const;
    void setTextRole(Uml::TextRole::Enum role);

    void handleRename();
    void changeName(const QString& newText);

    static bool isTextValid(const QString &text);

    void draw(QPainter & p, int offsetX, int offsetY);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    virtual bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);
    void setMessageText();

protected:
    UMLSceneSize minimumSize();

private:
    /// The association or message widget we may be linked to.
    LinkWidget * m_linkWidget;

    //////////////////// Data loaded/saved:

    /// Prepended text (such as for scope of association Role or method)
    QString m_preText;
    /// Ending text (such as bracket on changability notation for association Role)
    QString m_postText;
    /// The role the text widget will enact.
    Uml::TextRole::Enum m_textRole;

};

#endif
