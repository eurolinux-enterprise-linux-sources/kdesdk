/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "associationwidget.h"

// app includes
#include "association.h"
#include "assocpropdlg.h"
#include "assocrules.h"
#include "attribute.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "debug_utils.h"
#include "entity.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "operation.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"
#include "widget_utils.h"

// kde includes
#include <kinputdialog.h>
#include <klocale.h>
#include <kcolordialog.h>

// qt includes
#include <QPointer>
#include <QRegExpValidator>
#include <QApplication>

// system includes
#include <cmath>

DEBUG_REGISTER_DISABLED(AssociationWidget)

using namespace Uml;

/**
  * Constructor is private because the static create() methods shall
  * be used for constructing AssociationWidgets.
  *
  * @param scene              The parent view of this widget.
  */
AssociationWidget::AssociationWidget(UMLScene *scene)
  : WidgetBase(scene, WidgetBase::wt_Association)
{
}

/**
 * This constructor is really only for loading from XMI, otherwise it
 * should not be allowed as it creates an incomplete associationwidget.
  *
  * @param scene              The parent view of this widget.
 */
AssociationWidget* AssociationWidget::create(UMLScene *scene)
{
    AssociationWidget* instance = new AssociationWidget(scene);
    instance->init();
    return instance;
}

/**
  * Preferred constructor (static factory method.)
  *
  * @param scene      The parent view of this widget.
  * @param WidgetA    Pointer to the role A widget for the association.
  * @param assocType  The AssociationType::Enum for this association.
  * @param WidgetB    Pointer to the role B widget for the association.
  * @param umlobject  Pointer to the underlying UMLObject (if applicable.)
  */
AssociationWidget* AssociationWidget::create
                                    (UMLScene *scene, UMLWidget* pWidgetA,
                                     Uml::AssociationType::Enum assocType, UMLWidget* pWidgetB,
                                     UMLObject *umlobject /* = NULL */)
{
    AssociationWidget* instance = new AssociationWidget(scene);
    instance->init();
    if (umlobject) {
        instance->setUMLObject(umlobject);
    } else {
        // set up UMLAssociation obj if assoc is represented and both roles are UML objects
        if (Uml::AssociationType::hasUMLRepresentation(assocType)) {
            UMLObject* umlRoleA = pWidgetA->umlObject();
            UMLObject* umlRoleB = pWidgetB->umlObject();
            if (umlRoleA != NULL && umlRoleB != NULL) {
                bool swap;

                // THis isnt correct. We could very easily have more than one
                // of the same type of association between the same two objects.
                // Just create the association. This search should have been
                // done BEFORE creation of the widget, if it mattered to the code.
                // But lets leave check in here for the time being so that debugging
                // output is shown, in case there is a collision with code elsewhere.
                UMLDoc *doc = UMLApp::app()->document();
                UMLAssociation *myAssoc = doc->findAssociation( assocType, umlRoleA, umlRoleB, &swap );
                if (myAssoc != NULL) {
                    switch (assocType) {
                        case Uml::AssociationType::Generalization:
                        case Uml::AssociationType::Dependency:
                        case Uml::AssociationType::Association_Self:
                        case Uml::AssociationType::Coll_Message_Self:
                        case Uml::AssociationType::Seq_Message_Self:
                        case Uml::AssociationType::Containment:
                        case Uml::AssociationType::Realization:
                            DEBUG("AssociationWidget") << "Ignoring second construction of same assoctype "
                                     << assocType << " between " << umlRoleA->name()
                                     << " and " << umlRoleB->name();
                            break;
                        default:
                            DEBUG("AssociationWidget") << "constructing a similar or exact same assoctype "
                                     << assocType << " between " << umlRoleA->name() << " and "
                                     << umlRoleB->name() << "as an already existing assoc (swap="
                                     << swap << ")";
                            // now, just create a new association anyways
                            myAssoc = NULL;
                            break;
                    }
                }
                if (myAssoc == NULL) {
                    myAssoc = new UMLAssociation( assocType, umlRoleA, umlRoleB );
                    // CHECK: myAssoc is not yet inserted at any parent UMLPackage -
                    // need to check carefully that all callers do this, lest it be
                    // orphaned.
                    // ToolBarStateAssociation::addAssociationInViewAndDoc() is
                    // okay in this regard.
                }
                instance->setUMLAssociation(myAssoc);
            }
        }
    }

    instance->setWidgetForRole(pWidgetA, RoleType::A);
    instance->setWidgetForRole(pWidgetB, RoleType::B);

    instance->setAssociationType(assocType);

    instance->calculateEndingPoints();

    //The AssociationWidget is set to Activated because it already has its side widgets
    instance->setActivated(true);

    // sync UML meta-data to settings here
    instance->mergeAssociationDataIntoUMLRepresentation();

    // Collaboration messages need a name label because it's that
    // which lets operator== distinguish them, which in turn
    // permits us to have more than one message between two objects.
    if (instance->isCollaboration()) {
        // Create a temporary name to bring on setName()
        int collabID = instance->m_scene->generateCollaborationId();
        instance->setName('m' + QString::number(collabID));
    }

    return instance;
}

/**
 * Destructor.
 */
AssociationWidget::~AssociationWidget()
{
    delete m_associationLine;
}

/**
 * Overriding the method from WidgetBase because we need to do
 * something extra in case this AssociationWidget represents
 * an attribute of a classifier.
 */
void AssociationWidget::setUMLObject(UMLObject *obj)
{
    WidgetBase::setUMLObject(obj);
    if (obj == NULL)
        return;
    UMLClassifier *klass = NULL;
    UMLAttribute *attr = NULL;
    UMLEntity *ent = NULL;
    const UMLObject::ObjectType ot = obj->baseType();
    switch (ot) {
        case UMLObject::ot_Association:
            setUMLAssociation(dynamic_cast<UMLAssociation*>(obj));
            break;
        case UMLObject::ot_Operation:
            setOperation(dynamic_cast<UMLOperation*>(obj));
            break;
        case UMLObject::ot_Attribute:
            klass = static_cast<UMLClassifier*>(obj->parent());
            connect(klass, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                    this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            attr = static_cast<UMLAttribute*>(obj);
            connect(attr, SIGNAL(attributeChanged()), this, SLOT(slotAttributeChanged()));
            break;
        case UMLObject::ot_EntityAttribute:
            ent = static_cast<UMLEntity*>(obj->parent());
            connect(ent, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
                    this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            break;
        case UMLObject::ot_ForeignKeyConstraint:
            ent = static_cast<UMLEntity*>(obj->parent());
            connect(ent, SIGNAL(entityConstraintRemoved(UMLClassifierListItem*)),
                    this, SLOT(slotClassifierListItemRemoved(UMLClassifierListItem*)));
            break;
        default:
            uError() << "cannot associate UMLObject of type " << UMLObject::toString(ot);
            break;
    }
}

/**
 * Set all 'owned' child widgets to this font.
 */
void AssociationWidget::lwSetFont (QFont font)
{
    if( m_nameWidget) {
        m_nameWidget->setFont( font );
    }
    if( m_role[RoleType::A].roleWidget ) {
        m_role[RoleType::A].roleWidget->setFont( font );
    }
    if( m_role[RoleType::B].roleWidget ) {
        m_role[RoleType::B].roleWidget->setFont( font );
    }
    if( m_role[RoleType::A].multiplicityWidget ) {
        m_role[RoleType::A].multiplicityWidget->setFont( font );
    }
    if( m_role[RoleType::B].multiplicityWidget ) {
        m_role[RoleType::B].multiplicityWidget->setFont( font );
    }
    if( m_role[RoleType::A].changeabilityWidget)
        m_role[RoleType::A].changeabilityWidget->setFont( font );
    if( m_role[RoleType::B].changeabilityWidget)
        m_role[RoleType::B].changeabilityWidget->setFont( font );
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 * @todo Move to LinkWidget.
 */
UMLClassifier *AssociationWidget::operationOwner()
{
    Uml::RoleType::Enum role = (isCollaboration() ? Uml::RoleType::B : Uml::RoleType::A);
    UMLObject *o = widgetForRole(role)->umlObject();
    if (!o) {
        return 0;
    }
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
    if (!c) {
        uError() << "widgetForRole(" << role << ") is not a classifier";
    }
    return c;
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
UMLOperation *AssociationWidget::operation()
{
    return dynamic_cast<UMLOperation*>(m_umlObject);
}

/**
 * Implements operation from LinkWidget.
 * Motivated by FloatingTextWidget.
 */
void AssociationWidget::setOperation(UMLOperation *op)
{
    if (m_umlObject)
        disconnect(m_umlObject, SIGNAL(modified()), m_nameWidget, SLOT(setMessageText()));
    m_umlObject = op;
    if (m_umlObject)
        connect(m_umlObject, SIGNAL(modified()), m_nameWidget, SLOT(setMessageText()));
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
QString AssociationWidget::customOpText()
{
    return name();
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void AssociationWidget::setCustomOpText(const QString &opText)
{
    setName(opText);
}

/**
 * Calls setTextPosition on all the labels.
 * Overrides operation from LinkWidget.
 */
void AssociationWidget::resetTextPositions()
{
    if (m_role[RoleType::A].multiplicityWidget) {
        setTextPosition( TextRole::MultiA );
    }
    if (m_role[RoleType::B].multiplicityWidget) {
        setTextPosition( Uml::TextRole::MultiB );
    }
    if (m_role[RoleType::A].changeabilityWidget) {
        setTextPosition( Uml::TextRole::ChangeA );
    }
    if (m_role[RoleType::B].changeabilityWidget) {
        setTextPosition( Uml::TextRole::ChangeB );
    }
    if (m_nameWidget) {
        setTextPosition( Uml::TextRole::Name );
    }
    if (m_role[RoleType::A].roleWidget) {
        setTextPosition( Uml::TextRole::RoleAName );
    }
    if (m_role[RoleType::B].roleWidget) {
        setTextPosition( Uml::TextRole::RoleBName );
    }
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param ft        The text widget which to update.
 */
void AssociationWidget::setMessageText(FloatingTextWidget *ft)
{
    QString message;
    if (isCollaboration()) {
        if (m_umlObject != NULL) {
            message = multiplicity(RoleType::A) + ": " + operationText(m_scene);
        } else {
            message = multiplicity(RoleType::A) + ": " + name();
        }
    } else {
        message = name();
    }
    ft->setText(message);
}

/**
 * Sets the text of the given FloatingTextWidget.
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 */
void AssociationWidget::setText(FloatingTextWidget *ft, const QString &text)
{
    Uml::TextRole::Enum role = ft->textRole();
    switch (role) {
        case Uml::TextRole::Name:
            setName(text);
            break;
        case Uml::TextRole::RoleAName:
            setRoleName(text, RoleType::A);
            break;
        case Uml::TextRole::RoleBName:
            setRoleName(text, RoleType::B);
            break;
        case Uml::TextRole::MultiA:
            setMultiplicity(text, RoleType::A);
            break;
        case Uml::TextRole::MultiB:
            setMultiplicity(text, RoleType::B);
            break;
        default:
            uWarning() << "Unhandled TextRole: " << Uml::TextRole::toString(role);
            break;
    }
}

/**
 * Shows the association properties dialog and updates the
 * corresponding texts if its execution is successful.
 */
void AssociationWidget::showPropertiesDialog()
{
    QPointer<AssocPropDlg> dlg = new AssocPropDlg(static_cast<QWidget*>(m_scene->activeView()), this);
    if (dlg->exec()) {
        //rules built into these functions to stop updating incorrect values
        setName(name());

        setRoleName(roleName(RoleType::A), RoleType::A);
        setRoleName(roleName(RoleType::B), RoleType::B);

        setDocumentation(documentation());

        setRoleDocumentation(roleDocumentation(RoleType::A), RoleType::A);
        setRoleDocumentation(roleDocumentation(RoleType::B), RoleType::B);

        setMultiplicity(multiplicity(RoleType::A), RoleType::A);
        setMultiplicity(multiplicity(RoleType::B), RoleType::B);

        setVisibility(visibility(RoleType::A), RoleType::A);
        setVisibility(visibility(RoleType::B), RoleType::B);

        setChangeability(changeability(RoleType::A), RoleType::A);
        setChangeability(changeability(RoleType::B), RoleType::B);

        m_scene->showDocumentation(this, true);
    }
    delete dlg;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum    Return this AssociationWidget's sequence number string.
 * @param op        Return this AssociationWidget's operation string.
 */
UMLClassifier* AssociationWidget::seqNumAndOp(QString& seqNum, QString& op)
{
    seqNum = multiplicity(RoleType::A);
    op = name();
    UMLObject *o = widgetForRole(RoleType::B)->umlObject();
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(o);
    return c;
}

/**
 * Overrides operation from LinkWidget.
 * Required by FloatingTextWidget.
 *
 * @param seqNum    The new sequence number string to set.
 * @param op                The new operation string to set.
 */
void AssociationWidget::setSeqNumAndOp(const QString &seqNum, const QString &op)
{
    if (!op.isEmpty()) {
        setName(op);
    }
    setMultiplicity(seqNum, RoleType::A);
}

/**
 * Calculates the m_unNameLineSegment value according to the new
 * NameText topleft corner PT.
 * It iterates through all AssociationLine's segments and for each one
 * calculates the sum of PT's distance to the start point + PT's
 * distance to the end point. The segment with the smallest sum will
 * be the RoleTextSegment (if this segment moves then the RoleText
 * will move with it). It sets m_unNameLineSegment to the start point
 * of the chosen segment.
 *
 * Overrides operation from LinkWidget (i.e. this method is also
 * required by FloatingTextWidget.)
 */
void AssociationWidget::calculateNameTextSegment()
{
    if(!m_nameWidget) {
        return;
    }
    //changed to use the middle of the text
    //i think this will give a better result.
    //never know what sort of lines people come up with
    //and text could be long to give a false reading
    qreal xt = m_nameWidget->x();
    qreal yt = m_nameWidget->y();
    xt += m_nameWidget->width() / 2;
    yt += m_nameWidget->height() / 2;
    uint size = m_associationLine->count();
    //sum of length(PTP1) and length(PTP2)
    float total_length = 0;
    float smallest_length = 0;
    for(uint i = 0; i < size - 1; ++i) {
        UMLScenePoint pi = m_associationLine->point( i );
        UMLScenePoint pj = m_associationLine->point( i+1 );
        int xtiDiff = xt - pi.x();
        int xtjDiff = xt - pj.x();
        int ytiDiff = yt - pi.y();
        int ytjDiff = yt - pj.y();
        total_length =  sqrt( double(xtiDiff * xtiDiff + ytiDiff * ytiDiff) )
                        + sqrt( double(xtjDiff * xtjDiff + ytjDiff * ytjDiff) );
        //this gives the closest point
        if( total_length < smallest_length || i == 0) {
            smallest_length = total_length;
            m_unNameLineSegment = i;
        }
    }
}

/**
 * Returns the UMLAssociation representation of this object.
 *
 * @return  Pointer to the UMLAssociation that is represented by
 *          this AsociationWidget.
 */
UMLAssociation* AssociationWidget::association() const
{
    if (m_umlObject == NULL || m_umlObject->baseType() != UMLObject::ot_Association)
        return NULL;
    return static_cast<UMLAssociation*>(m_umlObject);
}

/**
 * Returns the UMLAttribute representation of this object.
 *
 * @return  Pointer to the UMLAttribute that is represented by
 *          this AsociationWidget.
 */
UMLAttribute* AssociationWidget::attribute() const
{
    if (m_umlObject == NULL)
        return NULL;
    UMLObject::ObjectType ot = m_umlObject->baseType();
    if (ot != UMLObject::ot_Attribute && ot != UMLObject::ot_EntityAttribute)
        return NULL;
    return static_cast<UMLAttribute*>(m_umlObject);
}

#if 0
/**
 * Overrides the assignment operator.
 */
AssociationWidget& AssociationWidget::operator=(const AssociationWidget & Other)
{
    *m_associationLine = *Other.m_associationLine;

    m_scene = Other.m_scene;

    if (Other.m_nameWidget) {
        m_nameWidget = new FloatingTextWidget(m_scene);
        *m_nameWidget = *(Other.m_nameWidget);
    } else {
        m_nameWidget = NULL;
    }

    for (unsigned r = (unsigned)A; r <= (unsigned)B; ++r) {
        WidgetRole& lhs = m_role[r];
        const WidgetRole& rhs = Other.m_role[r];
        lhs.m_nIndex = rhs.m_nIndex;
        lhs.m_nTotalCount = rhs.m_nTotalCount;

        if (rhs.multiplicityWidget) {
            lhs.multiplicityWidget = new FloatingTextWidget(m_scene);
            *(lhs.multiplicityWidget) = *(rhs.multiplicityWidget);
        } else {
            lhs.multiplicityWidget = NULL;
        }

        if (rhs.roleWidget) {
            lhs.roleWidget = new FloatingTextWidget(m_scene);
            *(lhs.roleWidget) = *(rhs.roleWidget);
        } else {
            lhs.roleWidget = NULL;
        }

        if (rhs.changeabilityWidget) {
            lhs.changeabilityWidget = new FloatingTextWidget(m_scene);
            *(lhs.changeabilityWidget) = *(rhs.changeabilityWidget);
        } else {
            lhs.changeabilityWidget = NULL;
        }

        lhs.umlWidget = rhs.umlWidget;
        lhs.m_OldCorner = rhs.m_OldCorner;
        lhs.m_WidgetRegion = rhs.m_WidgetRegion;
    }

    m_activated = Other.m_activated;
    m_unNameLineSegment = Other.m_unNameLineSegment;
    m_pMenu = Other.m_pMenu;
    setUMLAssociation(Other.getAssociation());
    m_selected = Other.m_selected;
    m_nMovingPoint = Other.m_nMovingPoint;

    return *this;
}
#endif

/**
 * Overrides the equality test operator.
 */
bool AssociationWidget::operator==(const AssociationWidget & Other) const
{
    if( this == &Other )
        return true;

    // if no model representation exists, then the widgets are not equal
    if ( association() == NULL && Other.association() == NULL )
        return false;

    if( !m_umlObject || !Other.m_umlObject ) {
        if( !Other.m_umlObject && m_umlObject )
            return false;
        if( Other.m_umlObject && !m_umlObject )
            return false;

    } else if( m_umlObject != Other.m_umlObject )
        return false;

    if (associationType() != Other.associationType())
        return false;

    if (widgetIDForRole(RoleType::A) != Other.widgetIDForRole(RoleType::A))
        return false;

    if (widgetIDForRole(RoleType::B) != Other.widgetIDForRole(RoleType::B))
        return false;

    if (widgetForRole(RoleType::A)->baseType() == WidgetBase::wt_Object &&
            Other.widgetForRole(RoleType::A)->baseType() == WidgetBase::wt_Object) {
        ObjectWidget *ownA = static_cast<ObjectWidget*>(widgetForRole(RoleType::A));
        ObjectWidget *otherA = static_cast<ObjectWidget*>(Other.widgetForRole(RoleType::A));
        if (ownA->localID() != otherA->localID())
            return false;
    }

    if (widgetForRole(RoleType::B)->baseType() == WidgetBase::wt_Object &&
            Other.widgetForRole(RoleType::B)->baseType() == WidgetBase::wt_Object) {
        ObjectWidget *ownB = static_cast<ObjectWidget*>(widgetForRole(RoleType::B));
        ObjectWidget *otherB = static_cast<ObjectWidget*>(Other.widgetForRole(RoleType::B));
        if (ownB->localID() != otherB->localID())
            return false;
    }

    // Two objects in a collaboration can have multiple messages between each other.
    // Here we depend on the messages having names, and the names must be different.
    // That is the reason why collaboration messages have strange initial names like
    // "m29997" or similar.
    return (name() == Other.name());
}

/**
 * Overrides the != operator.
 */
bool AssociationWidget::operator!=(AssociationWidget & Other) const
{
    return !(*this == Other);
}

/**
 * Returns a pointer to the association widget's line path.
 */
AssociationLine* AssociationWidget::associationLine() const
{
    return m_associationLine;
}

/**
 * Activates the AssociationWidget after a load.
 *
 * @return  true for success
 */
bool AssociationWidget::activate()
{
    if (m_umlObject == NULL &&
        AssociationType::hasUMLRepresentation(m_associationType)) {
        UMLObject *myObj = m_umldoc->findObjectById(m_nId);
        if (myObj == NULL) {
            uError() << "cannot find UMLObject " << ID2STR(m_nId);
            return false;
        } else {
            const UMLObject::ObjectType ot = myObj->baseType();
            if (ot == UMLObject::ot_Association) {
                UMLAssociation * myAssoc = static_cast<UMLAssociation*>(myObj);
                setUMLAssociation(myAssoc);
                m_associationLine->setAssocType( myAssoc->getAssocType() );
            } else {
                setUMLObject(myObj);
                setAssociationType(m_associationType);
            }
        }
    }

    if (m_activated)
        return true;

    Uml::AssociationType::Enum type = associationType();

    if (m_role[RoleType::A].umlWidget == NULL)
        setWidgetForRole(m_scene->findWidget(widgetIDForRole(RoleType::A)), RoleType::A);
    if (m_role[RoleType::B].umlWidget == NULL)
        setWidgetForRole(m_scene->findWidget(widgetIDForRole(RoleType::B)), RoleType::B);

    if(!m_role[RoleType::A].umlWidget || !m_role[RoleType::B].umlWidget) {
        DEBUG(DBG_SRC) << "Can not make association!";
        return false;
    }

    calculateEndingPoints();
    m_associationLine->activate();

    if (AssocRules::allowRole(type)) {
        for (unsigned r = RoleType::A; r <= RoleType::B; ++r) {
            WidgetRole& robj = m_role[r];
            if (robj.roleWidget == NULL)
                continue;
            robj.roleWidget->setLink(this);
            TextRole::Enum tr = (r == RoleType::A ? TextRole::RoleAName : TextRole::RoleBName);
            robj.roleWidget->setTextRole(tr);
            Uml::Visibility::Enum vis = visibility(Uml::RoleType::fromInt(r));
            robj.roleWidget->setPreText(Uml::Visibility::toString(vis, true));

            if (FloatingTextWidget::isTextValid(robj.roleWidget->text()))
                robj.roleWidget->show();
            else
                robj.roleWidget->hide();
            if (m_scene->type() == DiagramType::Collaboration)
                robj.roleWidget->setUMLObject(robj.umlWidget->umlObject());
            robj.roleWidget->activate();
        }
    }

    if( m_nameWidget != NULL ) {
        m_nameWidget->setLink(this);
        m_nameWidget->setTextRole( calculateNameType(TextRole::Name) );

        if ( FloatingTextWidget::isTextValid(m_nameWidget->text()) ) {
            m_nameWidget->show();
        } else {
            m_nameWidget->hide();
        }
        m_nameWidget->activate();
        calculateNameTextSegment();
    }

    for (unsigned r = RoleType::A; r <= RoleType::B; ++r) {
        WidgetRole& robj = m_role[r];

        FloatingTextWidget* pMulti = robj.multiplicityWidget;
        if (pMulti != NULL &&
                AssocRules::allowMultiplicity(type, robj.umlWidget->baseType())) {
            pMulti->setLink(this);
            TextRole::Enum tr = (r == RoleType::A ? TextRole::MultiA : TextRole::MultiB);
            pMulti->setTextRole(tr);
            if (FloatingTextWidget::isTextValid(pMulti->text()))
                pMulti->show();
            else
                pMulti->hide();
            pMulti->activate();
        }

        FloatingTextWidget* pChangeWidget = robj.changeabilityWidget;
        if (pChangeWidget != NULL ) {
            pChangeWidget->setLink(this);
            TextRole::Enum tr = (r == RoleType::A ? TextRole::ChangeA : TextRole::ChangeB);
            pChangeWidget->setTextRole(tr);
            if (FloatingTextWidget::isTextValid(pChangeWidget->text()))
                pChangeWidget->show();
            else
                pChangeWidget->hide ();
            pChangeWidget->activate();
        }
    }

    // Prepare the association class line if needed.
    if (m_associationClass && !m_pAssocClassLine) {
        createAssocClassLine();
    }

    m_activated = true;
    return true;
}

/**
 * Set the widget of the given role.
 * Add this AssociationWidget at the widget.
 * If this AssociationWidget has an underlying UMLAssociation then set
 * the widget's underlying UMLObject at the UMLAssociation's role object.
 *
 * @param widget    Pointer to the UMLWidget.
 * @param role      Role for which to set the widget.
 */
void AssociationWidget::setWidgetForRole(UMLWidget* widget, Uml::RoleType::Enum role)
{
    m_role[role].umlWidget = widget;
    if (widget) {
        m_role[role].umlWidget->addAssoc(this);
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
            association()->setObject(widget->umlObject(), role);
    }
}

/**
 * Return the multiplicity FloatingTextWidget widget of the given role.
 *
 * @return  Pointer to the multiplicity FloatingTextWidget object.
 */
FloatingTextWidget* AssociationWidget::multiplicityWidget(Uml::RoleType::Enum role) const
{
    return m_role[role].multiplicityWidget;
}

/**
 * Read property of FloatingTextWidget* m_nameWidget.
 *
 * @return  Pointer to the FloatingTextWidget name widget.
 */
FloatingTextWidget* AssociationWidget::nameWidget() const
{
    return m_nameWidget;
}

/**
 * Return the given role's FloatingTextWidget object.
 *
 * @return  Pointer to the role's FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::roleWidget(Uml::RoleType::Enum role) const
{
    return m_role[role].roleWidget;
}

/**
 * Return the given role's changeability FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::changeabilityWidget(Uml::RoleType::Enum role) const
{
    return m_role[role].changeabilityWidget;
}

/**
 * Return the FloatingTextWidget object indicated by the given TextRole::Enum.
 *
 * @return  Pointer to the text role's FloatingTextWidget widget.
 */
FloatingTextWidget* AssociationWidget::textWidgetByRole(Uml::TextRole::Enum tr) const
{
    switch (tr) {
        case Uml::TextRole::MultiA:
            return m_role[RoleType::A].multiplicityWidget;
        case Uml::TextRole::MultiB:
            return m_role[RoleType::B].multiplicityWidget;
        case Uml::TextRole::Name:
        case Uml::TextRole::Coll_Message:
            return m_nameWidget;
        case Uml::TextRole::RoleAName:
            return m_role[RoleType::A].roleWidget;
        case Uml::TextRole::RoleBName:
            return m_role[RoleType::B].roleWidget;
        case Uml::TextRole::ChangeA:
            return m_role[RoleType::A].changeabilityWidget;
        case Uml::TextRole::ChangeB:
            return m_role[RoleType::B].changeabilityWidget;
        default:
            break;
    }
    return NULL;
}

/**
 * Returns the m_nameWidget's text.
 *
 * @return  Text of the FloatingTextWidget name widget.
 */
QString AssociationWidget::name() const
{
    if (m_nameWidget == NULL)
        return QString();
    return m_nameWidget->text();
}

/**
 * Sets the text in the FloatingTextWidget widget representing the Name
 * of this association.
 */
void AssociationWidget::setName(const QString &strName)
{
    // set attribute of UMLAssociation associated with this associationwidget
    UMLAssociation *umla = association();
    if (umla)
        umla->setName(strName);

    bool newLabel = false;
    if(!m_nameWidget) {
        // Don't construct the FloatingTextWidget if the string is empty.
        if (! FloatingTextWidget::isTextValid(strName))
            return;

        newLabel = true;
        m_nameWidget = new FloatingTextWidget(m_scene, calculateNameType(Uml::TextRole::Name), strName);
        m_nameWidget->setLink(this);
    } else {
        m_nameWidget->setText(strName);
        if (! FloatingTextWidget::isTextValid(strName)) {
            //m_nameWidget->hide();
            m_scene->removeWidget(m_nameWidget);
            m_nameWidget = NULL;
            return;
        }
    }

    setTextPosition(Uml::TextRole::Name);
    if (newLabel) {
        m_nameWidget->setActivated();
        m_scene->addWidget(m_nameWidget);
    }

    m_nameWidget->show();
}

/**
 * Return the given role's FloatingTextWidget widget text.
 *
 * @return  The name set at the FloatingTextWidget.
 */
QString AssociationWidget::roleName(Uml::RoleType::Enum role) const
{
    if (m_role[role].roleWidget == NULL)
        return QString();
    return m_role[role].roleWidget->text();
}

/**
 * Sets the text to the FloatingTextWidget that display the Role text of this
 * association.
 * For this function to work properly, the associated widget
 *  should already be set.
 */
void AssociationWidget::setRoleName(const QString &strRole, Uml::RoleType::Enum role)
{
    Uml::AssociationType::Enum type = associationType();
    //if the association is not supposed to have a Role FloatingTextWidget
    if (!AssocRules::allowRole(type))  {
        return;
    }

    TextRole::Enum tr = (role == RoleType::A ? TextRole::RoleAName : TextRole::RoleBName);
    setFloatingText(tr, strRole, m_role[role].roleWidget);
    if (m_role[role].roleWidget) {
        Uml::Visibility::Enum vis = visibility(role);
        if (FloatingTextWidget::isTextValid(m_role[role].roleWidget->text())) {
            m_role[role].roleWidget->setPreText(Uml::Visibility::toString(vis, true));
            //m_role[role].roleWidget->show();
        } else {
            m_role[role].roleWidget->setPreText("");
            //m_role[role].roleWidget->hide();
        }
    }

    // set attribute of UMLAssociation associated with this associationwidget
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setRoleName(strRole, role);
}

/**
 * Set the documentation on the given role.
 */
void AssociationWidget::setRoleDocumentation(const QString &doc, Uml::RoleType::Enum role)
{
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setRoleDoc(doc, role);
    else
        m_role[role].roleDocumentation = doc;
}

/**
 * Returns the given role's documentation.
 */
QString AssociationWidget::roleDocumentation(Uml::RoleType::Enum role) const
{
    if (m_umlObject == NULL || m_umlObject->baseType() != UMLObject::ot_Association)
        return QString();
    UMLAssociation *umla = static_cast<UMLAssociation*>(m_umlObject);
    return umla->getRoleDoc(role);
}

/**
 * Change, create, or delete the FloatingTextWidget indicated by the given TextRole::Enum.
 *
 * @param tr    TextRole::Enum of the FloatingTextWidget to change or create.
 * @param text  Text string that controls the action:
 *              If empty and ft is NULL then setFloatingText() is a no-op.
 *              If empty and ft is non-NULL then the existing ft is deleted.
 *              If non-empty and ft is NULL then a new FloatingTextWidget is created
 *              and returned in ft with the text set.
 *              If non-empty and ft is non-NULL then the existing ft text is modified.
 * @param ft    Reference to the pointer to FloatingTextWidget to change or create.
 *              On creation/deletion, the pointer value will be changed.
 */
void AssociationWidget::setFloatingText(Uml::TextRole::Enum role,
                                        const QString &text,
                                        FloatingTextWidget* &ft)
{
    if (! FloatingTextWidget::isTextValid(text)) {
        if (ft) {
            // Remove preexisting FloatingTextWidget
            m_scene->removeWidget(ft);  // physically deletes ft
            ft = NULL;
        }
        return;
    }

    if (ft == NULL) {
        ft = new FloatingTextWidget(m_scene, role, text);
        ft->setLink(this);
        ft->activate();
        setTextPosition(role);
        m_scene->addWidget(ft);
    } else {
        bool newLabel = ft->text().isEmpty();
        ft->setText(text);
        if (newLabel)
            setTextPosition(role);
    }

    ft->show();
}

/**
 * Return the given role's multiplicity text.
 *
 * @return  Text of the given role's multiplicity widget.
 */
QString AssociationWidget::multiplicity(Uml::RoleType::Enum role) const
{
    if (m_role[role].multiplicityWidget == NULL)
        return QString();
    return m_role[role].multiplicityWidget->text();
}

/**
 * Sets the text in the FloatingTextWidget representing the multiplicity
 * at the given side of the association.
 */
void AssociationWidget::setMultiplicity(const QString& text, Uml::RoleType::Enum role)
{
    TextRole::Enum tr = (role == RoleType::A ? TextRole::MultiA : TextRole::MultiB);

    setFloatingText(tr, text, m_role[role].multiplicityWidget);

    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setMultiplicity(text, role);
}

/**
 * Gets the visibility on the given role of the association.
 */
Visibility::Enum AssociationWidget::visibility(Uml::RoleType::Enum role) const
{
    const UMLAssociation *assoc = association();
    if (assoc)
        return assoc->visibility(role);
    const UMLAttribute *attr = attribute();
    if (attr)
        return attr->visibility();
    return m_role[role].visibility;
}

/**
 * Sets the visibility on the given role of the association.
 */
void AssociationWidget::setVisibility(Visibility::Enum value, Uml::RoleType::Enum role)
{
    if (value == visibility(role)) {
        return;
    }
    if (m_umlObject) {
        // update our model object
        const UMLObject::ObjectType ot = m_umlObject->baseType();
        if (ot == UMLObject::ot_Association)
            association()->setVisibility(value, role);
        else if (ot == UMLObject::ot_Attribute)
            attribute()->setVisibility(value);
    }
    m_role[role].visibility = value;
    // update role pre-text attribute as appropriate
    if (m_role[role].roleWidget) {
        QString scopeString = Visibility::toString(value, true);
        m_role[role].roleWidget->setPreText(scopeString);
    }
}

/**
 * Gets the changeability on the the given end of the Association.
 */
Uml::Changeability::Enum AssociationWidget::changeability(Uml::RoleType::Enum role) const
{
    if (m_umlObject == NULL || m_umlObject->baseType() != UMLObject::ot_Association)
        return m_role[role].changeability;
    UMLAssociation *umla = static_cast<UMLAssociation*>(m_umlObject);
    return umla->changeability(role);
}

/**
 * Sets the changeability on the the given end of the Association.
 */
void AssociationWidget::setChangeability(Uml::Changeability::Enum value, Uml::RoleType::Enum role)
{
    if (value == changeability(role))
        return;
    QString changeString = Uml::Changeability::toString(value);
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)  // update our model object
        association()->setChangeability(value, role);
    m_role[role].changeability = value;
    // update our string representation
    setChangeWidget(changeString, role);
}

/**
 * For internal purposes only.
 * Other classes/users should use setChangeability() instead.
 */
void AssociationWidget::setChangeWidget(const QString &strChangeWidget, Uml::RoleType::Enum role)
{
    bool newLabel = false;
    TextRole::Enum tr = (role == RoleType::A ? TextRole::ChangeA : TextRole::ChangeB);

    if(!m_role[role].changeabilityWidget) {
        // Don't construct the FloatingTextWidget if the string is empty.
        if (strChangeWidget.isEmpty())
            return;

        newLabel = true;
        m_role[role].changeabilityWidget = new FloatingTextWidget(m_scene, tr, strChangeWidget);
        m_role[role].changeabilityWidget->setLink(this);
        m_scene->addWidget(m_role[role].changeabilityWidget);
        m_role[role].changeabilityWidget->setPreText("{"); // all types have this
        m_role[role].changeabilityWidget->setPostText("}"); // all types have this
    } else {
        if (m_role[role].changeabilityWidget->text().isEmpty()) {
            newLabel = true;
        }
        m_role[role].changeabilityWidget->setText(strChangeWidget);
    }
    m_role[role].changeabilityWidget->setActivated();

    if (newLabel) {
        setTextPosition( tr );
    }

    if(FloatingTextWidget::isTextValid(m_role[role].changeabilityWidget->text()))
        m_role[role].changeabilityWidget->show();
    else
        m_role[role].changeabilityWidget->hide();
}

/**
 * Returns true if the line path starts at the given widget.
 */
bool AssociationWidget::linePathStartsAt(const UMLWidget* widget)
{
    UMLScenePoint lpStart = m_associationLine->point(0);
    int startX = lpStart.x();
    int startY = lpStart.y();
    int wX = widget->x();
    int wY = widget->y();
    int wWidth = widget->width();
    int wHeight = widget->height();
    bool result = (startX >= wX && startX <= wX + wWidth &&
                   startY >= wY && startY <= wY + wHeight);
    return result;
}

/**
 * This function calculates which role should be set for the m_nameWidget FloatingTextWidget.
 */
Uml::TextRole::Enum AssociationWidget::calculateNameType(Uml::TextRole::Enum defaultRole)
{
    TextRole::Enum result = defaultRole;
    if( m_scene->type() == DiagramType::Collaboration ) {
        if(m_role[RoleType::A].umlWidget == m_role[RoleType::B].umlWidget) {
            result = TextRole::Coll_Message;//for now same as other Coll_Message
        } else {
            result = TextRole::Coll_Message;
        }
    } else if( m_scene->type() == DiagramType::Sequence ) {
        if(m_role[RoleType::A].umlWidget == m_role[RoleType::B].umlWidget) {
            result = TextRole::Seq_Message_Self;
        } else {
            result = TextRole::Seq_Message;
        }
    }

    return result;
}

/**
 * Gets the given role widget.
 *
 * @return  Pointer to the role's UMLWidget.
 */
UMLWidget* AssociationWidget::widgetForRole(Uml::RoleType::Enum role) const
{
    return m_role[role].umlWidget;
}

/**
 * Sets the associated widgets.
 *
 * @param widgetA   Pointer the role A widget for the association.
 * @param assocType The AssociationType::Enum for this association.
 * @param widgetB   Pointer the role B widget for the association.
 */
bool AssociationWidget::setWidgets( UMLWidget* widgetA,
                                    Uml::AssociationType::Enum assocType,
                                    UMLWidget* widgetB)
{
    //if the association already has a WidgetB or WidgetA associated, then
    //it cannot be changed to other widget, that would require a  deletion
    //of the association and the creation of a new one
    if ((m_role[RoleType::A].umlWidget && (m_role[RoleType::A].umlWidget != widgetA)) ||
            (m_role[RoleType::B].umlWidget && (m_role[RoleType::B].umlWidget != widgetB))) {
        return false;
    }
    setWidgetForRole(widgetA, RoleType::A);
    setAssociationType(assocType);
    setWidgetForRole(widgetB, RoleType::B);

    calculateEndingPoints();
    return true;
}

/**
 * Returns true if this association associates WidgetA to WidgetB,
 * otherwise it returns false.
 */
bool AssociationWidget::checkAssoc(UMLWidget * widgetA, UMLWidget *widgetB)
{
    return (widgetA == m_role[RoleType::A].umlWidget && widgetB == m_role[RoleType::B].umlWidget);
}

/**
 * CleansUp all the association's data in the related widgets.
 */
void AssociationWidget::cleanup()
{
    //let any other associations know we are going so they can tidy their positions up
    if(m_role[RoleType::A].m_nTotalCount > 2)
        updateAssociations(m_role[RoleType::A].m_nTotalCount - 1, m_role[RoleType::A].m_WidgetRegion, RoleType::A);
    if(m_role[RoleType::B].m_nTotalCount > 2)
        updateAssociations(m_role[RoleType::B].m_nTotalCount - 1, m_role[RoleType::B].m_WidgetRegion, RoleType::B);

    for (unsigned r = RoleType::A; r <= RoleType::B; ++r) {
        WidgetRole& robj = m_role[r];

        if(robj.umlWidget) {
            robj.umlWidget->removeAssoc(this);
            robj.umlWidget = 0;
        }
        if(robj.roleWidget) {
            m_scene->removeWidget(robj.roleWidget);
            robj.roleWidget = 0;
        }
        if(robj.multiplicityWidget) {
            m_scene->removeWidget(robj.multiplicityWidget);
            robj.multiplicityWidget = 0;
        }
        if(robj.changeabilityWidget) {
            m_scene->removeWidget(robj.changeabilityWidget);
            robj.changeabilityWidget = 0;
        }
    }

    if(m_nameWidget) {
        m_scene->removeWidget(m_nameWidget);
        m_nameWidget = 0;
    }

    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
        /*
           We do not remove the UMLAssociation from the document.
           Why? - Well, for example we might be in the middle of
           a cut/paste. If the UMLAssociation is removed by the cut
           then upon pasteing we have a problem.
           This is not quite clean yet - there should be a way to
           explicitly delete a UMLAssociation.  The Right Thing would
           be to have a ListView representation for UMLAssociation.
        `
                IF we are cut n pasting, why are we handling this association as a pointer?
                We should be using the XMI representation for a cut and paste. This
                allows us to be clean here, AND a choice of recreating the object
                w/ same id IF its a "cut", or a new object if its a "copy" operation
                (in which case we wouldnt be here, in cleanup()).
         */
        setUMLAssociation(0);
    }

    m_associationLine->cleanup();
    removeAssocClassLine();
}

/**
 * Set our internal umlAssociation.
 */
void AssociationWidget::setUMLAssociation (UMLAssociation * assoc)
{
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
        UMLAssociation *umla = association();

        // safety check. Did some num-nuts try to set the existing
        // association again? If so, just bail here
        if (umla == assoc)
            return;

        //umla->disconnect(this);  //Qt does disconnect automatically upon destruction.
        umla->nrof_parent_widgets--;

        // ANSWER: This is the wrong treatment of cut and paste. Associations that
        // are being cut/n pasted should be serialized to XMI, then reconstituted
        // (IF a paste operation) rather than passing around object pointers. Its
        // just too hard otherwise to prevent problems in the code. Bottom line: we need to
        // delete orphaned associations or we well get code crashes and memory leaks.
        if (umla->nrof_parent_widgets <= 0) {
            //umla->deleteLater();
        }

        m_umlObject = NULL;
    }

    if(assoc) {
        m_umlObject = assoc;

        // move counter to "0" from "-1" (which means, no assocwidgets)
        if(assoc->nrof_parent_widgets < 0)
            assoc->nrof_parent_widgets = 0;

        assoc->nrof_parent_widgets++;
        connect(assoc, SIGNAL(modified()), this, SLOT(syncToModel()));
    }

}

/** Returns true if the Widget is either at the starting or ending side of the association */
bool AssociationWidget::hasWidget(UMLWidget* widget)
{
    return (widget == m_role[RoleType::A].umlWidget || widget == m_role[RoleType::B].umlWidget);
}

/**
 * Returns true if this AssociationWidget represents a collaboration message.
 */
bool AssociationWidget::isCollaboration() const
{
    Uml::AssociationType::Enum at = associationType();
    return (at == AssociationType::Coll_Message || at == AssociationType::Coll_Message_Self);
}

/**
 * Gets the association's type.
 *
 * @return  This AssociationWidget's AssociationType::Enum.
 */
Uml::AssociationType::Enum AssociationWidget::associationType() const
{
    if (m_umlObject == NULL || m_umlObject->baseType() != UMLObject::ot_Association)
        return m_associationType;
    UMLAssociation *umla = static_cast<UMLAssociation*>(m_umlObject);
    return umla->getAssocType();
}

/**
 * Sets the association's type.
 *
 * @param type   The AssociationType::Enum to set.
 */
void AssociationWidget::setAssociationType(Uml::AssociationType::Enum type)
{
    if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        association()->setAssociationType(type);
    m_associationType = type;
    m_associationLine->setAssocType(type);
    // If the association new type is not supposed to have Multiplicity
    // FloatingTexts and a Role FloatingTextWidget then set the texts
    // to empty.
    // NB We do not physically delete the floatingtext widgets here because
    // those widgets are also stored in the UMLView::m_WidgetList.
    if( !AssocRules::allowMultiplicity(type, widgetForRole(RoleType::A)->baseType()) ) {
        if (m_role[RoleType::A].multiplicityWidget) {
            m_role[RoleType::A].multiplicityWidget->setName("");
        }
        if (m_role[RoleType::B].multiplicityWidget) {
            m_role[RoleType::B].multiplicityWidget->setName("");
        }
    }
    if( !AssocRules::allowRole( type ) ) {
        if (m_role[RoleType::A].roleWidget) {
            m_role[RoleType::A].roleWidget->setName("");
        }
        if (m_role[RoleType::B].roleWidget) {
            m_role[RoleType::B].roleWidget->setName("");
        }
        setRoleDocumentation("", RoleType::A);
        setRoleDocumentation("", RoleType::B);
    }
}

/**
 * Gets the ID of the given role widget.
 */
Uml::ID::Type AssociationWidget::widgetIDForRole(Uml::RoleType::Enum role) const
{
    if (m_role[role].umlWidget == NULL) {
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association) {
            UMLAssociation *umla = static_cast<UMLAssociation*>(m_umlObject);
            return umla->getObjectId(role);
        }
        uError() << "umlWidget is NULL";
        return Uml::ID::None;
    }
    if (m_role[role].umlWidget->baseType() == WidgetBase::wt_Object)
        return static_cast<ObjectWidget*>(m_role[role].umlWidget)->localID();
    Uml::ID::Type id = m_role[role].umlWidget->id();
    return id;
}

/**
 * Returns a QString Object representing this AssociationWidget.
 */
QString AssociationWidget::toString() const
{
    QString string;
    static const QChar colon(':');

    if (widgetForRole(RoleType::A)) {
        string = widgetForRole(RoleType::A)->name();
    }
    string.append(colon);

    if (m_role[RoleType::A].roleWidget) {
        string += m_role[RoleType::A].roleWidget->text();
    }
    string.append(colon);
    string.append(Uml::AssociationType::toStringI18n(associationType()));
    string.append(colon);

    if (widgetForRole(RoleType::B)) {
        string += widgetForRole(RoleType::B)->name();
    }

    string.append(colon);
    if (m_role[RoleType::B].roleWidget) {
        string += m_role[RoleType::B].roleWidget->text();
    }

    return string;
}

/**
 * Adds a break point (if left mouse button).
 */
void AssociationWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * me)
{
    if (me->button() != Qt::RightButton && me->button() != Qt::LeftButton)
        return;
    int i = m_associationLine->closestPointIndex(me->scenePos());
    if (i == -1) {
        m_associationLine->setSelected(false);
        return;
    }
    if (me->button() != Qt::LeftButton)
        return;
    const UMLScenePoint mp(me->scenePos());
    if (associationType() == AssociationType::Exception ){
        return;
    }
    /* if there is no point around the mouse pointer, we insert a new one */
    if (! m_associationLine->isPoint(i, mp, POINT_DELTA)) {
        m_associationLine->insertPoint(i + 1, mp);
        if (m_nLinePathSegmentIndex == i) {
            UMLScenePoint segStart = m_associationLine->point(i);
            UMLScenePoint segEnd = m_associationLine->point(i + 2);
            const int midSegX = segStart.x() + (segEnd.x() - segStart.x()) / 2;
            const int midSegY = segStart.y() + (segEnd.y() - segStart.y()) / 2;
            /*
            DEBUG(DBG_SRC) << "segStart=(" << segStart.x() << "," << segStart.y()
                  << "), segEnd=(" << segEnd.x() << "," << segEnd.y()
                  << "), midSeg=(" << midSegX << "," << midSegY
                  << "), mp=(" << mp.x() << "," << mp.y() << ")";
             */
            if (midSegX > mp.x() || midSegY < mp.y()) {
                m_nLinePathSegmentIndex++;
                DEBUG(DBG_SRC) << "setting m_nLinePathSegmentIndex to "
                    << m_nLinePathSegmentIndex;
                computeAssocClassLine();
            }
        }
    } else {
        /* deselect the line path */
        m_associationLine->setSelected( false );

        /* there was a point so we remove the point */
        if (m_associationLine->removePoint(i, mp, POINT_DELTA)) {
            /* Maybe reattach association class connecting line
               to different association linepath segment.  */
            const int numberOfLines = m_associationLine->count() - 1;
            if (m_nLinePathSegmentIndex >= numberOfLines) {
                m_nLinePathSegmentIndex = numberOfLines - 1;
                computeAssocClassLine();
            }
        }

        /* select the line path */
        m_associationLine->setSelected( true );
    }

    m_associationLine->update();

    calculateNameTextSegment();
    m_umldoc->setModified(true);
}

/**
 * Overrides moveEvent.
 */
void AssociationWidget::moveEvent(QGraphicsSceneMouseEvent* me)
{
    // 2004-04-30: Achim Spangler
    // Simple Approach to block moveEvent during load of
    // XMI
    /// @todo avoid trigger of this event during load

    if ( m_umldoc->loading() ) {
        // hmmh - change of position during load of XMI
        // -> there is something wrong
        // -> avoid movement during opening
        // -> print warn and stay at old position
        uWarning() << "called during load of XMI for ViewType: "
            << m_scene->type() << ", and BaseType: " << baseType();
        return;
    }
    /*to be here a line segment has moved.
      we need to see if the three text widgets needs to be moved.
      there are a few things to check first though:

      1) Do they exist
      2) does it need to move:
      2a) for the multi widgets only move if they changed region, otherwise they are close enough
      2b) for role name move if the segment it is on moves.
    */
    //first see if either the first or last segments moved, else no need to recalculate their point positions

    UMLScenePoint oldNamePoint = calculateTextPosition(TextRole::Name);
    UMLScenePoint oldMultiAPoint = calculateTextPosition(TextRole::MultiA);
    UMLScenePoint oldMultiBPoint = calculateTextPosition(TextRole::MultiB);
    UMLScenePoint oldChangeAPoint = calculateTextPosition(TextRole::ChangeA);
    UMLScenePoint oldChangeBPoint = calculateTextPosition(TextRole::ChangeB);
    UMLScenePoint oldRoleAPoint = calculateTextPosition(TextRole::RoleAName);
    UMLScenePoint oldRoleBPoint = calculateTextPosition(TextRole::RoleBName);

    m_associationLine->setPoint( m_nMovingPoint, me->scenePos());
    int pos = m_associationLine->count() - 1;//set to last point for widget b

    if ( m_nMovingPoint == 1 || (m_nMovingPoint == pos-1) ) {
        calculateEndingPoints();
    }
    if (m_role[RoleType::A].changeabilityWidget && (m_nMovingPoint == 1)) {
        setTextPositionRelatively(TextRole::ChangeA, oldChangeAPoint);
    }
    if (m_role[RoleType::B].changeabilityWidget && (m_nMovingPoint == 1)) {
        setTextPositionRelatively(TextRole::ChangeB, oldChangeBPoint);
    }
    if (m_role[RoleType::A].multiplicityWidget && (m_nMovingPoint == 1)) {
        setTextPositionRelatively(TextRole::MultiA, oldMultiAPoint);
    }
    if (m_role[RoleType::B].multiplicityWidget && (m_nMovingPoint == pos-1)) {
        setTextPositionRelatively(TextRole::MultiB, oldMultiBPoint);
    }

    if (m_nameWidget) {
        if(m_nMovingPoint == (int)m_unNameLineSegment ||
                m_nMovingPoint - 1 == (int)m_unNameLineSegment) {
            setTextPositionRelatively(TextRole::Name, oldNamePoint);
        }
    }

    if (m_role[RoleType::A].roleWidget) {
        setTextPositionRelatively(TextRole::RoleAName, oldRoleAPoint);
    }
    if (m_role[RoleType::B].roleWidget) {
        setTextPositionRelatively(TextRole::RoleBName, oldRoleBPoint);
    }
}


/** Calculates and sets the first and last point in the Association's AssociationLine
    Each point is a middle point of its respecting UMLWidget's Bounding rectangle
    or a corner of it
    This method picks which sides to use for the association */
void AssociationWidget::calculateEndingPoints()
{
    /*
     * For each UMLWidget the diagram is divided in four regions by its diagonals
     * as indicated below
     *                              Region 2
     *                         \                /
     *                           \            /
     *                             +--------+
     *                             | \    / |
     *                Region 1     |   ><   |    Region 3
     *                             | /    \ |
     *                             +--------+
     *                           /            \
     *                         /                \
     *                              Region 4
     *
     * Each diagonal is defined by two corners of the bounding rectangle
     *
     * To calculate the first point in the AssociationLine we have to find out in which
     * Region (defined by WidgetA's diagonals) is WidgetB's center
     * (let's call it Region M.) After that the first point will be the middle
     * point of the rectangle's side contained in Region M.
     *
     * To calculate the last point in the AssociationLine we repeat the above but
     * in the opposite direction (from widgetB to WidgetA)
     */

    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;
    UMLWidget *pWidgetB = m_role[RoleType::B].umlWidget;
    if (!pWidgetA || !pWidgetB)
        return;
    m_role[RoleType::A].m_OldCorner.setX( pWidgetA->x() );
    m_role[RoleType::A].m_OldCorner.setY( pWidgetA->y() );
    m_role[RoleType::B].m_OldCorner.setX( pWidgetB->x() );
    m_role[RoleType::B].m_OldCorner.setY( pWidgetB->y() );

    int size = m_associationLine->count();
    if(size < 2)
        m_associationLine->setEndPoints( m_role[RoleType::A].m_OldCorner, m_role[RoleType::B].m_OldCorner );

    // See if an association to self.
    // See if it needs to be set up before we continue:
    // If self association/message and doesn't have the minimum 4 points
    // then create it.  Make sure no points are out of bounds of viewing area.
    // This only happens on first time through that we are worried about.
    if (pWidgetA == pWidgetB && size < 4) {
        const int DISTANCE = 50;
        int x = pWidgetA->x();
        int y = pWidgetA->y();
        int h = pWidgetA->height();
        int w = pWidgetA->width();
        //see if above widget ok to start
        if( y - DISTANCE > 0 ) {
            m_associationLine->setEndPoints( UMLScenePoint( x + w / 4, y ) , UMLScenePoint( x + w * 3 / 4, y ) );
            m_associationLine->insertPoint( 1, UMLScenePoint( x + w / 4, y - DISTANCE ) );
            m_associationLine->insertPoint( 2 ,UMLScenePoint( x + w * 3 / 4, y - DISTANCE ) );
            m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = North;
        } else {
            m_associationLine->setEndPoints( UMLScenePoint( x + w / 4, y + h ), UMLScenePoint( x + w * 3 / 4, y + h ) );
            m_associationLine->insertPoint( 1, UMLScenePoint( x + w / 4, y + h + DISTANCE ) );
            m_associationLine->insertPoint( 2, UMLScenePoint( x + w * 3 / 4, y + h + DISTANCE ) );
            m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = South;
        }
        return;
    }//end a == b

    if (associationType() == AssociationType::Exception && size < 4) {
        int xa = pWidgetA->x();
        int ya = pWidgetA->y();
        int ha = pWidgetA->height();
        int wa = pWidgetA->width();

        int xb = pWidgetB->x();
        int yb = pWidgetB->y();
        int hb = pWidgetB->height();
        //int wb = pWidgetB->width();

        m_associationLine->setEndPoints( UMLScenePoint( xa + wa , ya + ha/2 ) , UMLScenePoint( xb , yb + hb/2 ) );
        m_associationLine->insertPoint( 1, UMLScenePoint( xa + wa , ya + ha/2 ));
        m_associationLine->insertPoint( 2 ,UMLScenePoint( xb , yb + hb/2 ));
        updatePointsException();
        return;
    }
    // If the line has more than one segment change the values to calculate
    // from widget to point 1.
    int xB = pWidgetB->x() + pWidgetB->width() / 2;
    int yB = pWidgetB->y() + pWidgetB->height() / 2;
    if( size > 2 ) {
        UMLScenePoint p = m_associationLine->point( 1 );
        xB = p.x();
        yB = p.y();
    }
    doUpdates(xB, yB, RoleType::A);

    // Now do the same for widgetB.
    // If the line has more than one segment change the values to calculate
    // from widgetB to the last point away from it.
    int xA = pWidgetA->x() + pWidgetA->width() / 2;
    int yA = pWidgetA->y() + pWidgetA->height() / 2;
    if (size > 2 ) {
        UMLScenePoint p = m_associationLine->point( size - 2 );
        xA = p.x();
        yA = p.y();
    }
    doUpdates( xA, yA, RoleType::B );

    computeAssocClassLine();
}

void AssociationWidget::doUpdates(int otherX, int otherY, Uml::RoleType::Enum role)
{
    // Find widget region.
    Region oldRegion = m_role[role].m_WidgetRegion;
    UMLWidget *pWidget = m_role[role].umlWidget;
    QRect rc(pWidget->x(), pWidget->y(),
             pWidget->width(), pWidget->height());
    Region& region = m_role[role].m_WidgetRegion;  // alias for brevity
    region = findPointRegion( rc, otherX, otherY);
    // Move some regions to the standard ones.
    switch( region ) {
    case NorthWest:
        region = North;
        break;
    case NorthEast:
        region = East;
        break;
    case SouthEast:
        region = South;
        break;
    case SouthWest:
    case Center:
        region = West;
        break;
    default:
        break;
    }
    int regionCount = getRegionCount(region, role) + 2;//+2 = (1 for this one and one to halve it)
    int totalCount = m_role[role].m_nTotalCount;
    if( oldRegion != region ) {
        updateRegionLineCount( regionCount - 1, regionCount, region, role );
        updateAssociations( totalCount - 1, oldRegion, role );
    } else if( totalCount != regionCount ) {
        updateRegionLineCount( regionCount - 1, regionCount, region, role );
    } else {
        updateRegionLineCount( m_role[role].m_nIndex, totalCount, region, role );
    }
    updateAssociations( regionCount, region, role );
}

/**
 * Read property of bool m_activated.
 */
bool AssociationWidget::isActivated()
{
    return m_activated;
}

/**
 * Set the m_activated flag of a widget but does not perform the Activate method.
 */
void AssociationWidget::setActivated(bool active /*=true*/)
{
    m_activated = active;
}

/**
 * Synchronize this widget from the UMLAssociation.
 */
void AssociationWidget::syncToModel()
{
    UMLAssociation *uml = association();

    if (uml == NULL) {
        UMLAttribute *attr = attribute();
        if (attr == NULL)
            return;
        setVisibility(attr->visibility(), RoleType::B);
        setRoleName(attr->name(), RoleType::B);
        return;
    }
    // block signals until finished
    uml->blockSignals(true);

    setName(uml->name());
    setRoleName(uml->getRoleName(RoleType::A), RoleType::A);
    setRoleName(uml->getRoleName(RoleType::B), RoleType::B);
    setVisibility(uml->visibility(RoleType::A), RoleType::A);
    setVisibility(uml->visibility(RoleType::B), RoleType::B);
    setChangeability(uml->changeability(RoleType::A), RoleType::A);
    setChangeability(uml->changeability(RoleType::B), RoleType::B);
    setMultiplicity(uml->getMultiplicity(RoleType::A), RoleType::A);
    setMultiplicity(uml->getMultiplicity(RoleType::B), RoleType::B);

    uml->blockSignals(false);
}

// this will synchronize UMLAssociation w/ this new Widget

/**
 * Merges/syncs the association widget data into UML object
 * representation.
 * CHECK: Can we get rid of this.
 */
void AssociationWidget::mergeAssociationDataIntoUMLRepresentation()
{
    UMLAssociation *umlassoc = association();
    UMLAttribute *umlattr = attribute();
    if (umlassoc == NULL && umlattr == NULL)
        return;

    // block emit modified signal, or we get a horrible loop
    m_umlObject->blockSignals(true);

    // would be desirable to do the following
    // so that we can be sure its back to initial state
    // in case we missed something here.
    //uml->init();

    // floating text widgets
    FloatingTextWidget *text = nameWidget();
    if (text)
        m_umlObject->setName(text->text());

    text = roleWidget(RoleType::A);
    if (text && umlassoc)
        umlassoc->setRoleName(text->text(), RoleType::A);

    text = roleWidget(RoleType::B);
    if (text) {
        if (umlassoc)
            umlassoc->setRoleName(text->text(), RoleType::B);
        else if (umlattr)
            umlattr->setName(text->text());
    }

    text = multiplicityWidget(RoleType::A);
    if (text && umlassoc)
        umlassoc->setMultiplicity(text->text(), RoleType::A);

    text = multiplicityWidget(RoleType::B);
    if (text && umlassoc)
        umlassoc->setMultiplicity(text->text(), RoleType::B);

    // unblock
    m_umlObject->blockSignals(false);
}

/**
 * Auxiliary method for widgetMoved():
 * Saves all ideally computed floatingtext positions before doing any
 * kind of change.  This is necessary because a single invocation of
 * calculateEndingPoints() modifies the AssociationLine ending points on ALL
 * AssociationWidgets.  This means that if we don't save the old ideal
 * positions then they are irretrievably lost as soon as
 * calculateEndingPoints() is invoked.
 */
void AssociationWidget::saveIdealTextPositions()
{
    m_oldNamePoint = calculateTextPosition(TextRole::Name);
    m_oldMultiAPoint = calculateTextPosition(TextRole::MultiA);
    m_oldMultiBPoint = calculateTextPosition(TextRole::MultiB);
    m_oldChangeAPoint = calculateTextPosition(TextRole::ChangeA);
    m_oldChangeBPoint = calculateTextPosition(TextRole::ChangeB);
    m_oldRoleAPoint = calculateTextPosition(TextRole::RoleAName);
    m_oldRoleBPoint = calculateTextPosition(TextRole::RoleBName);
}

/** Adjusts the ending point of the association that connects to Widget */
void AssociationWidget::widgetMoved(UMLWidget* widget, int x, int y )
{
    // 2004-04-30: Achim Spangler
    // Simple Approach to block moveEvent during load of
    // XMI

    /// @todo avoid trigger of this event during load
    if ( m_umldoc->loading() ) {
        // hmmh - change of position during load of XMI
        // -> there is something wrong
        // -> avoid movement during opening
        // -> print warn and stay at old position
        DEBUG(DBG_SRC) << "called during load of XMI for ViewType: " << m_scene->type()
            << ", and BaseType: " << baseType();
        return;
    }

    int dx = m_role[RoleType::A].m_OldCorner.x() - x;
    int dy = m_role[RoleType::A].m_OldCorner.y() - y;
    uint size = m_associationLine->count();
    uint pos = size - 1;
    if (associationType() == AssociationType::Exception) {
        updatePointsException();
        setTextPosition( TextRole::Name );
    }
    else
        calculateEndingPoints();

    // Assoc to self - move all points:
    if( m_role[RoleType::A].umlWidget == m_role[RoleType::B].umlWidget) {
        for (int i = 1; i < (int)pos; ++i) {
            UMLScenePoint p = m_associationLine->point( i );
            int newX = p.x() - dx;
            int newY = p.y() - dy;
            // safety. We DON'T want to go off the screen
            if(newX < 0)
                newX = 0;
            // safety. We DON'T want to go off the screen
            if(newY < 0)
                newY = 0;
            newX = m_scene->snappedX( newX );
            newY = m_scene->snappedY( newY );
            p.setX( newX );
            p.setY( newY );
            m_associationLine->setPoint( i, p );
        }

        if ( m_nameWidget && !m_nameWidget->isSelected() ) {
            setTextPositionRelatively(TextRole::Name, m_oldNamePoint);
        }

    }//end if widgetA = widgetB
    else if (m_role[RoleType::A].umlWidget==widget) {
        if (m_nameWidget && m_unNameLineSegment == 0 && !m_nameWidget->isSelected() ) {
            //only calculate position and move text if the segment it is on is moving
            setTextPositionRelatively(TextRole::Name, m_oldNamePoint);
        }
    }//end if widgetA moved
    else if (m_role[RoleType::B].umlWidget==widget) {
        if (m_nameWidget && (m_unNameLineSegment == pos-1) && !m_nameWidget->isSelected() ) {
            //only calculate position and move text if the segment it is on is moving
            setTextPositionRelatively(TextRole::Name, m_oldNamePoint);
        }
    }//end if widgetB moved

    if ( m_role[RoleType::A].roleWidget && !m_role[RoleType::A].roleWidget->isSelected() ) {
        setTextPositionRelatively(TextRole::RoleAName, m_oldRoleAPoint);
    }
    if ( m_role[RoleType::B].roleWidget && !m_role[RoleType::B].roleWidget->isSelected() ) {
        setTextPositionRelatively(TextRole::RoleBName, m_oldRoleBPoint);
    }
    if ( m_role[RoleType::A].multiplicityWidget && !m_role[RoleType::A].multiplicityWidget->isSelected() ) {
        setTextPositionRelatively(TextRole::MultiA, m_oldMultiAPoint);
    }
    if ( m_role[RoleType::B].multiplicityWidget && !m_role[RoleType::B].multiplicityWidget->isSelected() ) {
        setTextPositionRelatively(TextRole::MultiB, m_oldMultiBPoint);
    }
    if ( m_role[RoleType::A].changeabilityWidget && !m_role[RoleType::A].changeabilityWidget->isSelected() ) {
        setTextPositionRelatively(TextRole::ChangeA, m_oldChangeAPoint);
    }
    if ( m_role[RoleType::B].changeabilityWidget && !m_role[RoleType::B].changeabilityWidget->isSelected() ) {
        setTextPositionRelatively(TextRole::ChangeB, m_oldChangeBPoint);
    }
}//end method widgetMoved

/**
 * Adjusts the points of the association exception.
 * Method called when a widget was moved by widgetMoved(widget,x,y)
 */
void AssociationWidget::updatePointsException()
{
    UMLWidget *pWidgetA = m_role[RoleType::A].umlWidget;
    UMLWidget *pWidgetB = m_role[RoleType::B].umlWidget;

    int xa = pWidgetA->x();
    int ya = pWidgetA->y();
    int ha = pWidgetA->height();
    int wa = pWidgetA->width();

    int xb = pWidgetB->x();
    int yb = pWidgetB->y();
    int hb = pWidgetB->height();
    int wb = pWidgetB->width();
    int xmil, ymil;
    int xdeb, ydeb;
    int xfin, yfin;
    int ESPACEX, ESPACEY;
    UMLScenePoint p1;
    UMLScenePoint p2;
    //calcul des coordonnées au milieu de la flèche eclair
    if (xb - xa - wa >= 45) {
        ESPACEX = 0;
        xdeb = xa + wa;
        xfin = xb;
    } else if (xa - xb - wb > 45 ) {
        ESPACEX = 0;
        xdeb = xa;
        xfin = xb + wb;
    } else {
        ESPACEX = 15;
        xdeb = xa + wa/2;
        xfin = xb + wb/2;
    }

    xmil = xdeb + (xfin - xdeb)/2;

    if (yb - ya - ha >= 45  )  {
        ESPACEY = 0;
        ydeb = ya + ha;
        yfin = yb;
    } else if (ya - yb - hb > 45 ) {
        ESPACEY = 0;
        ydeb = ya;
        yfin = yb + hb;
    } else {
        ESPACEY = 15;
        ydeb = ya + ha/2;
        yfin = yb + hb/2;
    }

    ymil = ydeb + (yfin - ydeb)/2;

    p1.setX(xmil + (xfin - xmil)*1/2); p1.setY(ymil + (yfin - ymil)*1/3);
    p2.setX(xmil - (xmil - xdeb)*1/2); p2.setY(ymil - (ymil - ydeb)*1/3);

    if (abs(p1.x() - p2.x()) <= 10)
        ESPACEX = 15;
    if (abs(p1.y() - p2.y()) <= 10)
        ESPACEY = 15;

    m_associationLine->setEndPoints( UMLScenePoint( xdeb , ydeb ) , UMLScenePoint( xfin , yfin ) );
    m_associationLine->setPoint( 1, UMLScenePoint(p1.x() + ESPACEX,p1.y() + ESPACEY));
    m_associationLine->setPoint( 2 ,UMLScenePoint(p2.x() - ESPACEX,p2.y() - ESPACEY));

    m_role[RoleType::A].m_WidgetRegion = m_role[RoleType::B].m_WidgetRegion = North;
}


/** Finds out in which region of rectangle Rect contains the Point (PosX, PosY) and returns the region
    number:
    1 = Region 1
    2 = Region 2
    3 = Region 3
    4 = Region 4
    5 = On diagonal 2 between Region 1 and 2
    6 = On diagonal 1 between Region 2 and 3
    7 = On diagonal 2 between Region 3 and 4
    8 = On diagonal 1 between Region 4 and 1
    9 = On diagonal 1 and On diagonal 2 (the center)
*/
AssociationWidget::Region AssociationWidget::findPointRegion(const UMLSceneRect& Rect, int PosX, int PosY)
{
    float w = (float)Rect.width();
    float h = (float)Rect.height();
    float x = (float)Rect.x();
    float y = (float)Rect.y();
    float Slope2 = w / h;
    float Slope1 = Slope2*(float)(-1);
    float b1 = x + w - ( Slope1* y );
    float b2 = x - ( Slope2* y );

    float eval1 = Slope1 * (float)PosY + b1;
    float eval2 = Slope2  *(float)PosY + b2;

    Region result = Error;
    //if inside region 1
    if(eval1 > PosX && eval2 > PosX) {
        result = West;
    }
    //if inside region 2
    else if (eval1 > PosX && eval2 < PosX) {
        result = North;
    }
    //if inside region 3
    else if (eval1 < PosX && eval2 < PosX) {
        result = East;
    }
    //if inside region 4
    else if (eval1 < PosX && eval2 > PosX) {
        result = South;
    }
    //if inside region 5
    else if (eval1 == PosX && eval2 < PosX) {
        result = NorthWest;
    }
    //if inside region 6
    else if (eval1 < PosX && eval2 == PosX) {
        result = NorthEast;
    }
    //if inside region 7
    else if (eval1 == PosX && eval2 > PosX) {
        result = SouthEast;
    }
    //if inside region 8
    else if (eval1 > PosX && eval2 == PosX) {
        result = SouthWest;
    }
    //if inside region 9
    else if (eval1 == PosX && eval2 == PosX) {
        result = Center;
    }
    return result;
}

/**
 * Returns a point with interchanged X and Y coordinates.
 */
UMLScenePoint AssociationWidget::swapXY(const UMLScenePoint &p)
{
    UMLScenePoint swapped( p.y(), p.x() );
    return swapped;
}

/**
 * Returns the total length of the association's AssociationLine:
 * result = segment_1_length + segment_2_length + ... + segment_n_length
 */
//float AssociationWidget::totalLength()
//{
//    uint size = m_associationLine->count();
//    float total_length = 0;

//    for(uint i = 0; i < size - 1; ++i) {
//        UMLScenePoint pi = m_associationLine->point( i );
//        UMLScenePoint pj = m_associationLine->point( i+1 );
//        int xi = pi.y();
//        int xj = pj.y();
//        int yi = pi.x();
//        int yj = pj.x();
//        total_length +=  sqrt( double(((xj - xi)*(xj - xi)) + ((yj - yi)*(yj - yi))) );
//    }

//    return total_length;
//}

/**
 * Calculates which point of segment P1P2 has a distance equal to
 * Distance from P1.
 * Let's say such point is PX, the distance from P1 to PX must be equal
 * to Distance and if PX is not a point of the segment P1P2 then the
 * function returns (-1,-1).
 */
UMLScenePoint AssociationWidget::calculatePointAtDistance(const UMLScenePoint &P1, const UMLScenePoint &P2, float Distance)
{
    /*
      the distance D between points (x1, y1) and (x3, y3) has the following formula:
          ---     ------------------------------
      D =    \   /         2         2
              \ /   (x3 - x1)  +  (y3 - y1)

      D, x1 and y1 are known and the point (x3, y3) is inside line (x1,y1)(x2,y2), so if the
      that line has the formula y = mx + b
      then y3 = m*x3 + b

       2             2             2
      D   = (x3 - x1)  +  (y3 - y1)

       2       2                 2      2                 2
      D    = x3    - 2*x3*x1 + x1   + y3   - 2*y3*y1  + y1

       2       2       2       2                  2
      D    - x1    - y1    = x3    - 2*x3*x1  + y3   - 2*y3*y1

       2       2       2       2                          2
      D    - x1    - y1    = x3    - 2*x3*x1  + (m*x3 + b)  - 2*(m*x3 + b)*y1

       2       2       2              2       2 2
      D    - x1    - y1   + 2*b*y1 - b   =  (m  + 1)*x3   + (-2*x1 + 2*m*b -2*m*y1)*x3

       2      2       2       2
      C  = - D    + x1    + y1   - 2*b*y1 + b


       2
      A  = (m    + 1)

      B  = (-2*x1 + 2*m*b -2*m*y1)

      and we have
       2
      A * x3 + B * x3 - C = 0

                         ---------------
             -B +  ---  /  2
                      \/  B   - 4*A*C
      sol_1  = --------------------------------
                       2*A


                         ---------------
             -B -  ---  /  2
                      \/  B   - 4*A*C
      sol_2  = --------------------------------
                       2*A


      then in the distance formula we have only one variable x3 and that is easy
      to calculate
    */
    int x1 = P1.y();
    int y1 = P1.x();
    int x2 = P2.y();
    int y2 = P2.x();

    if(x2 == x1) {
        return UMLScenePoint(x1, y1 + (int)Distance);
    }
    float slope = ((float)y2 - (float)y1) / ((float)x2 - (float)x1);
    float b = (y1 - slope*x1);
    float A = (slope * slope) + 1;
    float B = (2*slope*b) - (2*x1)  - (2*slope*y1);
    float C = (b*b) - (Distance*Distance) + (x1*x1) + (y1*y1) - (2*b*y1);
    float t = B*B - 4*A*C;

    if(t < 0) {
        return UMLScenePoint(-1, -1);
    }
    float sol_1 = ((-1* B) + sqrt(t) ) / (2*A);
    float sol_2 = ((-1*B) - sqrt(t) ) / (2*A);

    if(sol_1 < 0.0 && sol_2 < 0.0) {
        return UMLScenePoint(-1, -1);
    }
    UMLScenePoint sol1Point((int)(slope*sol_1 + b), (int)(sol_1));
    UMLScenePoint sol2Point((int)(slope*sol_2 + b), (int)(sol_2));
    if(sol_1 < 0 && sol_2 >=0) {
        if(x2 > x1) {
            if(x1 <= sol_2 && sol_2 <= x2)
                return sol2Point;
        } else {
            if(x2 <= sol_2 && sol_2 <= x1)
                return sol2Point;
        }
    } else if(sol_1 >= 0 && sol_2 < 0) {
        if(x2 > x1) {
            if(x1 <= sol_1 && sol_1 <= x2)
                return sol1Point;
        } else {
            if(x2 <= sol_1 && sol_1 <= x1)
                return sol1Point;
        }
    } else {
        if(x2 > x1) {
            if(x1 <= sol_1 && sol_1 <= x2)
                return sol1Point;
            if(x1 <= sol_2 && sol_2 <= x2)
                return sol2Point;
        } else {
            if(x2 <= sol_1 && sol_1 <= x1)
                return sol1Point;
            if(x2 <= sol_2 && sol_2 <= x1)
                return sol2Point;
        }
    }
    return UMLScenePoint(-1, -1);
}

/**
 * Calculates which point of a perpendicular line to segment P1P2 that contains P2
 *  has a distance equal to Distance from P2,
 * Lets say such point is P3,  the distance from P2 to P3 must be equal to Distance
 */
UMLScenePoint AssociationWidget::calculatePointAtDistanceOnPerpendicular(const UMLScenePoint &P1, const UMLScenePoint &P2, float Distance)
{
    /*
      the distance D between points (x2, y2) and (x3, y3) has the following formula:

          ---     ------------------------------
      D =    \   /         2             2
              \ / (x3 - x2)  +  (y3 - y2)

      D, x2 and y2 are known and line P2P3 is perpendicular to line (x1,y1)(x2,y2), so if the
      line P1P2 has the formula y = m*x + b,
      then      (x1 - x2)
          m =  -----------    , because it is perpendicular to line P1P2
                (y2 - y1)

      also y2 = m*x2 + b
      => b = y2 - m*x2

      then P3 = (x3, m*x3 + b)

       2            2            2
      D  = (x3 - x2)  + (y3 - y2)

       2     2               2     2               2
      D  = x3  - 2*x3*x2 + x2  + y3  - 2*y3*y2 + y2

       2     2     2     2               2
      D  - x2  - y2  = x3  - 2*x3*x2 + y3  - 2*y3*y2



       2     2     2     2                       2
      D  - x2  - y2  = x3  - 2*x3*x2 + (m*x3 + b)  - 2*(m*x3 + b)*y2

       2     2     2                   2        2       2
      D  - x2  - y2  + 2*b*y2 - b  = (m  + 1)*x3  + (-2*x2 + 2*m*b -2*m*y2)*x3

              2       2       2              2
      C  = - D    + x2    + y2   - 2*b*y2 + b

             2
      A  = (m  + 1)

      B  = (-2*x2 + 2*m*b -2*m*y2)

      and we have
       2
      A * x3 + B * x3 - C = 0


                           ---------------
                     ---  /  2
                -B +    \/  B   - 4*A*C
      sol_1 = --------------------------------
                        2*A


                           ---------------
                     ---  /  2
                -B -    \/  B   - 4*A*C
      sol_2 = --------------------------------
                        2*A

      then in the distance formula we have only one variable x3 and that is easy
      to calculate
    */
    if (P1.x() == P2.x()) {
        return UMLScenePoint((int)(P2.x() + Distance), P2.y());
    }
    const int x1 = P1.y();
    const int y1 = P1.x();
    const int x2 = P2.y();
    const int y2 = P2.x();

    float slope = ((float)x1 - (float)x2) / ((float)y2 - (float)y1);
    float b = (y2 - slope*x2);
    float A = (slope * slope) + 1;
    float B = (2*slope*b) - (2*x2) - (2*slope*y2);
    float C = (b*b) - (Distance*Distance) + (x2*x2) + (y2*y2) - (2*b*y2);
    float t = B*B - 4*A*C;
    if (t < 0) {
        return UMLScenePoint(-1, -1);
    }
    float sol_1 = ((-1* B) + sqrt(t) ) / (2*A);

    float sol_2 = ((-1*B) - sqrt(t) ) / (2*A);

    if(sol_1 < 0 && sol_2 < 0) {
        return UMLScenePoint(-1, -1);
    }
    UMLScenePoint sol1Point((int)(slope*sol_1 + b), (int)sol_1);
    UMLScenePoint sol2Point((int)(slope*sol_2 + b), (int)sol_2);
    if(sol_1 < 0 && sol_2 >=0) {
        return sol2Point;
    } else if(sol_1 >= 0 && sol_2 < 0) {
        return sol1Point;
    } else {    // Choose one solution , either will work fine
        if(slope >= 0) {
            if(sol_1 <= sol_2)
                return sol2Point;
            else
                return sol1Point;
        } else {
            if(sol_1 <= sol_2)
                return sol1Point;
            else
                return sol2Point;
        }

    }
    return UMLScenePoint(-1, -1);  // never reached, just keep compilers happy
}

/** 
 * Calculates the intersection (PS) between line P1P2 and a perpendicular line containing
 * P3, the result is returned in ResultingPoint. and result value represents the distance
 * between ResultingPoint and P3; if this value is negative an error ocurred. 
 */
float AssociationWidget::perpendicularProjection(const UMLScenePoint& P1, const UMLScenePoint& P2, const UMLScenePoint& P3,
        UMLScenePoint& ResultingPoint)
{
    //line P1P2 is Line 1 = y=slope1*x + b1

    //line P3PS is Line 1 = y=slope2*x + b2

    float slope2 = 0;
    float slope1 = 0;
    float sx = 0, sy = 0;
    int y2 = P2.x();
    int y1 = P1.x();
    int x2 = P2.y();
    int x1 = P1.y();
    int y3 = P3.x();
    int x3 = P3.y();
    float distance = 0;
    float b1 = 0;

    float b2 = 0;

    if(x2 == x1) {
        sx = x2;
        sy = y3;
    } else if(y2 == y1) {
        sy = y2;
        sx = x3;
    } else {
        slope1 = (y2 - y1)/ (x2 - x1);
        slope2 = (x1 - x2)/ (y2 - y1);
        b1 = y2 - (slope1 * x2);
        b2 = y3 - (slope2 * x3);
        sx = (b2 - b1) / (slope1 - slope2);
        sy = slope1*sx + b1;
    }
    distance = (int)( sqrt( ((x3 - sx)*(x3 - sx)) + ((y3 - sy)*(y3 - sy)) ) );

    ResultingPoint.setX( (int)sy );
    ResultingPoint.setY( (int)sx );

    return distance;
}

/**
 * Calculates the position of the text widget depending on the role
 * that widget is playing.
 * Returns the point at which to put the widget.
 */
UMLScenePoint AssociationWidget::calculateTextPosition(Uml::TextRole::Enum role)
{
    const int SPACE = 2;
    UMLScenePoint p(-1, -1), q(-1, -1);

    // used to find out if association end point (p)
    // is at top or bottom edge of widget.

    if (role == TextRole::MultiA || role == TextRole::ChangeA || role == TextRole::RoleAName) {
        p = m_associationLine->point( 0 );
        q = m_associationLine->point( 1 );
    } else if (role == TextRole::MultiB || role == TextRole::ChangeB || role == TextRole::RoleBName) {
        const uint lastSegment = m_associationLine->count() - 1;
        p = m_associationLine->point(lastSegment);
        q = m_associationLine->point(lastSegment - 1);
    } else if (role != TextRole::Name) {
        uError() << "called with unsupported TextRole::Enum " << role;
        return UMLScenePoint(-1, -1);
    }

    FloatingTextWidget *text = textWidgetByRole(role);
    int textW = 0, textH = 0;
    if (text) {
        textW = text->width();
        textH = text->height();
    }

    UMLSceneValue x = 0, y = 0;

    if (role == TextRole::MultiA || role == TextRole::MultiB) {
        const bool isHorizontal = (p.y() == q.y());
        const int atBottom = p.y() + SPACE;
        const int atTop = p.y() - SPACE - textH;
        const int atLeft = p.x() - SPACE - textW;
        const int atRight = p.x() + SPACE;
        y = (p.y() > q.y()) == isHorizontal ? atBottom : atTop;
        x = (p.x() < q.x()) == isHorizontal ? atRight : atLeft;

    } else if (role == TextRole::ChangeA || role == TextRole::ChangeB) {

        if( p.y() > q.y() )
            y = p.y() - SPACE - (textH * 2);
        else
            y = p.y() + SPACE + textH;

        if( p.x() < q.x() )
            x = p.x() + SPACE;
        else
            x = p.x() - SPACE - textW;

    } else if (role == TextRole::RoleAName || role == TextRole::RoleBName) {

        if( p.y() > q.y() )
            y = p.y() - SPACE - textH;
        else
            y = p.y() + SPACE;

        if( p.x() < q.x() )
            x = p.x() + SPACE;
        else
            x = p.x() - SPACE - textW;

    } else if (role == TextRole::Name) {

        calculateNameTextSegment();
        x = (UMLSceneValue)( ( m_associationLine->point(m_unNameLineSegment).x() +
                     m_associationLine->point(m_unNameLineSegment + 1).x() ) / 2 );

        y = (UMLSceneValue)( ( m_associationLine->point(m_unNameLineSegment).y() +
                     m_associationLine->point(m_unNameLineSegment + 1).y() ) / 2 );
    }

    if (text) {
        constrainTextPos(x, y, textW, textH, role);
    }
    p = UMLScenePoint( x, y );
    return p;
}

/**
 * Return the mid point between p0 and p1
 */
UMLScenePoint AssociationWidget::midPoint(const UMLScenePoint& p0, const UMLScenePoint& p1)
{
    UMLScenePoint midP;
    if (p0.x() < p1.x())
        midP.setX(p0.x() + (p1.x() - p0.x()) / 2);
    else
        midP.setX(p1.x() + (p0.x() - p1.x()) / 2);
    if (p0.y() < p1.y())
        midP.setY(p0.y() + (p1.y() - p0.y()) / 2);
    else
        midP.setY(p1.y() + (p0.y() - p1.y()) / 2);
    return midP;
}

/**
 * Constrains the FloatingTextWidget X and Y values supplied.
 * Implements the abstract operation from LinkWidget.
 *
 * @param textX       Candidate X value (may be modified by the constraint.)
 * @param textY       Candidate Y value (may be modified by the constraint.)
 * @param textWidth   Width of the text.
 * @param textHeight  Height of the text.
 * @param tr          Uml::Text_Role of the text.
 */
void AssociationWidget::constrainTextPos(UMLSceneValue &textX, UMLSceneValue &textY,
                                         UMLSceneValue textWidth, UMLSceneValue textHeight,
                                         Uml::TextRole::Enum tr)
{
    const int textCenterX = textX + textWidth / 2;
    const int textCenterY = textY + textHeight / 2;
    const uint lastSegment = m_associationLine->count() - 1;
    UMLScenePoint p0, p1;
    switch (tr) {
        case TextRole::RoleAName:
        case TextRole::MultiA:
        case TextRole::ChangeA:
            p0 = m_associationLine->point(0);
            p1 = m_associationLine->point(1);
            // If we are dealing with a single line then tie the
            // role label to the proper half of the line, i.e.
            // the role label must be closer to the "other"
            // role object.
            if (lastSegment == 1)
                p1 = midPoint(p0, p1);
            break;
        case TextRole::RoleBName:
        case TextRole::MultiB:
        case TextRole::ChangeB:
            p0 = m_associationLine->point(lastSegment - 1);
            p1 = m_associationLine->point(lastSegment);
            if (lastSegment == 1)
                p0 = midPoint(p0, p1);
            break;
        case TextRole::Name:
        case TextRole::Coll_Message:  // CHECK: collab.msg texts seem to be TextRole::Name
        case TextRole::State:         // CHECK: is this used?
            // Find the linepath segment to which the (textX,textY) is closest
            // and constrain to the corridor of that segment (see farther below)
            {
                int minDistSquare = 100000;  // utopian initial value
                int lpIndex = 0;
                for (uint i = 0; i < lastSegment; ++i) {
                    p0 = m_associationLine->point(i);
                    p1 = m_associationLine->point(i + 1);
                    UMLScenePoint midP = midPoint(p0, p1);
                    const int deltaX = textCenterX - midP.x();
                    const int deltaY = textCenterY - midP.y();
                    const int cSquare = deltaX * deltaX + deltaY * deltaY;
                    if (cSquare < minDistSquare) {
                        minDistSquare = cSquare;
                        lpIndex = i;
                    }
                }
                p0 = m_associationLine->point(lpIndex);
                p1 = m_associationLine->point(lpIndex + 1);
            }
            break;
        default:
            uError() << "unexpected TextRole::Enum " << tr;
            return;
            break;
    }
    /* Constraint:
       The midpoint between p0 and p1 is taken to be the center of a circle
       with radius D/2 where D is the distance between p0 and p1.
       The text center needs to be within this circle else it is constrained
       to the nearest point on the circle.
     */
    p0 = swapXY(p0);    // go to the natural coordinate system
    p1 = swapXY(p1);    // with (0,0) in the lower left corner
    UMLScenePoint midP = midPoint(p0, p1);
    // If (textX,textY) is not inside the circle around midP then
    // constrain (textX,textY) to the nearest point on that circle.
    const int x0 = p0.x();
    const int y0 = p0.y();
    const int x1 = p1.x();
    const int y1 = p1.y();
    double r = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) / 2;
    if (textWidth > r)
        r = textWidth;
    // swap textCenter{X,Y} to convert from Qt coord.system.
    const UMLScenePoint origTextCenter(textCenterY, textCenterX);
    const int relX = abs(origTextCenter.x() - midP.x());
    const int relY = abs(origTextCenter.y() - midP.y());
    const double negativeWhenInsideCircle = relX * relX + relY * relY - r * r;
    if (negativeWhenInsideCircle <= 0.0) {
        return;
    }
    /*
     The original constraint was to snap the text position to the
     midpoint but that creates unpleasant visual jitter:
    textX = midP.y() - textWidth / 2;   // go back to Qt coord.sys.
    textY = midP.x() - textHeight / 2;  // go back to Qt coord.sys.

     Rather, we project the text position onto the closest point
     on the circle:

     Circle equation:
       relX^2 + relY^2 - r^2 = 0   , or in other words
       relY^2 = r^2 - relX^2       , or
       relY = sqrt(r^2 - relX^2)
     Line equation:
       relY = a * relX + b
         We can omit "b" because relX and relY are already relative to
         the circle origin, therefore we can also write:
       a = relY / relX
     To obtain the point of intersection between the circle of radius r
     and the line connecting the circle origin with the point (relX, relY),
     we equate the relY:
       a * x = sqrt(r^2 - x^2)     , or in other words
       a^2 * x^2 = r^2 - x^2       , or
       x^2 * (a^2 + 1) = r^2       , or
       x^2 = r^2 / (a^2 + 1)       , or
       x = sqrt(r^2 / (a^2 + 1))
     and then
       y = a * x
     The resulting x and y are relative to the circle origin so we just add
     the circle origin (X,Y) to obtain the constrained (textX,textY).
     */
    // Handle the special case, relX = 0.
    if (relX == 0) {
        if (origTextCenter.y() > midP.y())
            textX = midP.y() + (int)r;   // go back to Qt coord.sys.
        else
            textX = midP.y() - (int)r;   // go back to Qt coord.sys.
        textX -= textWidth / 2;
        return;
    }
    const double a = (double)relY / (double)relX;
    const double x = sqrt(r*r / (a*a + 1));
    const double y = a * x;
    if (origTextCenter.x() > midP.x())
        textY = midP.x() + (int)x;   // go back to Qt coord.sys.
    else
        textY = midP.x() - (int)x;   // go back to Qt coord.sys.
    textY -= textHeight / 2;
    if (origTextCenter.y() > midP.y())
        textX = midP.y() + (int)y;   // go back to Qt coord.sys.
    else
        textX = midP.y() - (int)y;   // go back to Qt coord.sys.
    textX -= textWidth / 2;
}

/**
 * Puts the text widget with the given role at the given position.
 * This method calls @ref calculateTextPostion to get the needed position.
 * I.e. the line segment it is on has moved and it should move the same
 * amount as the line.
 */
void AssociationWidget::setTextPosition(Uml::TextRole::Enum role)
{
    bool startMove = false;
    if( m_role[RoleType::A].multiplicityWidget && m_role[RoleType::A].multiplicityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::B].multiplicityWidget && m_role[RoleType::B].multiplicityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::A].changeabilityWidget && m_role[RoleType::A].changeabilityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::B].changeabilityWidget && m_role[RoleType::B].changeabilityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::A].roleWidget  && m_role[RoleType::A].roleWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::B].roleWidget  && m_role[RoleType::B].roleWidget->getStartMove() )
        startMove = true;
    else if( m_nameWidget && m_nameWidget->getStartMove() )
        startMove = true;
    if (startMove) {
        return;
    }
    FloatingTextWidget *ft = textWidgetByRole(role);
    if (ft == NULL)
        return;
    UMLScenePoint pos = calculateTextPosition(role);
    int x = pos.x();
    int y = pos.y();
    ft->setX( x );
    ft->setY( y );
}

/**
 * Moves the text widget with the given role by the difference between
 * the two points.
 */
void AssociationWidget::setTextPositionRelatively(Uml::TextRole::Enum role, const UMLScenePoint &oldPosition)
{
    bool startMove = false;
    if( m_role[RoleType::A].multiplicityWidget && m_role[RoleType::A].multiplicityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::B].multiplicityWidget && m_role[RoleType::B].multiplicityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::A].changeabilityWidget && m_role[RoleType::A].changeabilityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::B].changeabilityWidget && m_role[RoleType::B].changeabilityWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::A].roleWidget  && m_role[RoleType::A].roleWidget->getStartMove() )
        startMove = true;
    else if( m_role[RoleType::B].roleWidget  && m_role[RoleType::B].roleWidget->getStartMove() )
        startMove = true;
    else if( m_nameWidget && m_nameWidget->getStartMove() )
        startMove = true;

    if (startMove) {
        return;
    }
    FloatingTextWidget *ft = textWidgetByRole(role);
    if (ft == NULL)
        return;
    UMLSceneValue ftX = ft->x();
    UMLSceneValue ftY = ft->y();

    UMLScenePoint pos = calculateTextPosition(role);
    int relX = pos.x() - oldPosition.x();
    int relY = pos.y() - oldPosition.y();
    UMLSceneValue ftNewX = ftX + relX;
    UMLSceneValue ftNewY = ftY + relY;

    bool oldIgnoreSnapToGrid = ft->getIgnoreSnapToGrid();
    ft->setIgnoreSnapToGrid( true );
    ft->setX( ftNewX );
    ft->setY( ftNewY );
    ft->setIgnoreSnapToGrid( oldIgnoreSnapToGrid );
}

/**
 * Remove dashed connecting line for association class.
 */
void AssociationWidget::removeAssocClassLine()
{
    selectAssocClassLine(false);
    if (m_pAssocClassLine) {
        delete m_pAssocClassLine;
        m_pAssocClassLine = NULL;
    }
    if (m_associationClass) {
        m_associationClass->setClassAssociationWidget(NULL);
        m_associationClass = NULL;
    }
}

/**
 * Creates the association class connecting line.
 */
void AssociationWidget::createAssocClassLine()
{
    if (m_pAssocClassLine == NULL) {
        m_pAssocClassLine = new QGraphicsLineItem;
        m_scene->addItem(m_pAssocClassLine);
    }
    computeAssocClassLine();
    QPen pen(lineColor(), lineWidth(), Qt::DashLine);
    m_pAssocClassLine->setPen(pen);
    m_pAssocClassLine->setVisible(true);
}

/**
 * Creates the association class connecting line using the specified
 * ClassifierWidget.
 *
 * @param classifierWidget The ClassifierWidget to use.
 * @param linePathSegmentIndex The index of the segment where the
 *        association class is created.
 */
void AssociationWidget::createAssocClassLine(ClassifierWidget* classifier,
                                             int linePathSegmentIndex)
{
    m_nLinePathSegmentIndex = linePathSegmentIndex;

    if (m_nLinePathSegmentIndex < 0) {
        return;
    }

    m_associationClass = classifier;
    m_associationClass->setClassAssociationWidget(this);

    createAssocClassLine();
}

/**
 * Compute the end points of m_pAssocClassLine in case this
 * association has an attached association class.
 */
void AssociationWidget::computeAssocClassLine()
{
    if (m_associationClass == NULL || m_pAssocClassLine == NULL)
        return;
    if (m_nLinePathSegmentIndex < 0) {
        uError() << "m_nLinePathSegmentIndex is not set";
        return;
    }
    UMLScenePoint segStart = m_associationLine->point(m_nLinePathSegmentIndex);
    UMLScenePoint segEnd = m_associationLine->point(m_nLinePathSegmentIndex + 1);
    const int midSegX = segStart.x() + (segEnd.x() - segStart.x()) / 2;
    const int midSegY = segStart.y() + (segEnd.y() - segStart.y()) / 2;

    UMLScenePoint segmentMidPoint(midSegX, midSegY);
    UMLSceneRect classRectangle = m_associationClass->rect();
    UMLScenePoint cwEdgePoint = findIntercept(classRectangle, segmentMidPoint);
    int acwMinX = cwEdgePoint.x();
    int acwMinY = cwEdgePoint.y();

    m_pAssocClassLine->setLine(midSegX, midSegY, acwMinX, acwMinY);
}

/**
 * Renders the association class connecting line selected.
 */
void AssociationWidget::selectAssocClassLine(bool sel /* =true */)
{
    if (!sel) {
        if (m_pAssocClassLineSel0) {
            delete m_pAssocClassLineSel0;
            m_pAssocClassLineSel0 = NULL;
        }
        if (m_pAssocClassLineSel1) {
            delete m_pAssocClassLineSel1;
            m_pAssocClassLineSel1 = NULL;
        }
        return;
    }
    if (m_pAssocClassLine == NULL) {
        uError() << "cannot select because m_pAssocClassLine is NULL";
        return;
    }
    if (m_pAssocClassLineSel0)
        delete m_pAssocClassLineSel0;
    m_pAssocClassLineSel0 = Widget_Utils::decoratePoint(m_pAssocClassLine->line().p1());
    if (m_pAssocClassLineSel1)
        delete m_pAssocClassLineSel1;
    m_pAssocClassLineSel1 = Widget_Utils::decoratePoint(m_pAssocClassLine->line().p2());
}

/**
 * Sets the association to be selected.
 */
void AssociationWidget::mousePressEvent(QGraphicsSceneMouseEvent * me)
{
    // clear other selected stuff on the screen of ShiftKey
    if( me->modifiers() != Qt::ShiftModifier )
        m_scene->clearSelected();

    m_nMovingPoint = -1;
    //make sure we should be here depending on the button
    if(me->button() != Qt::RightButton && me->button() != Qt::LeftButton)
        return;
    UMLScenePoint mep = me->scenePos();
    // See if `mep' is on the connecting line to the association class
    if (onAssocClassLine(mep)) {
        m_selected = true;
        selectAssocClassLine();
        return;
    }
    // See if the user has clicked on a point to start moving the line segment
    // from that point
    checkPoints(mep);
    setSelected( !m_selected );
}

/**
 * Displays the right mouse buttom menu if right button is pressed.
 */
void AssociationWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent * me)
{
    if(me->button() != Qt::RightButton && me->button() != Qt::LeftButton) {
        setSelected( false );
        return;
    }

    // Check whether a point was moved and whether the moved point is
    // located on the straight line between its neighbours.
    // if yes, remove it
    ///@todo: check for non-horizontal / -vertical lines
    if (m_nMovingPoint > 0 && m_nMovingPoint < m_associationLine->count() - 1)
    {
        UMLScenePoint m = m_associationLine->point(m_nMovingPoint);
        UMLScenePoint b = m_associationLine->point(m_nMovingPoint - 1);
        UMLScenePoint a = m_associationLine->point(m_nMovingPoint + 1);
        if ( (b.x() == m.x() && a.x() == m.x()) ||
             (b.y() == m.y() && a.y() == m.y()) )
            m_associationLine->removePoint(m_nMovingPoint, m, POINT_DELTA);
    }
    m_nMovingPoint = -1;
    if (me->button() != Qt::RightButton) {
        return;
    }
    ListPopupMenu* menu = setupPopupMenu(0, me->scenePos());
    menu->popup(me->screenPos());
    setSelected();
}//end method mouseReleaseEvent

/**
 * Handles the selection from the popup menu.
 */
void AssociationWidget::slotMenuSelection(QAction* action)
{
    QString oldText, newText;
    QRegExpValidator v(QRegExp(".*"), 0);
    Uml::AssociationType::Enum atype = associationType();
    Uml::RoleType::Enum r = RoleType::B;
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);

    //if it's a collaboration message we now just use the code in floatingtextwidget
    //this means there's some redundant code below but that's better than duplicated code
    if (isCollaboration() && sel != ListPopupMenu::mt_Delete) {
        m_nameWidget->slotMenuSelection(action);
        return;
    }

    switch(sel) {
    case ListPopupMenu::mt_Properties:
        if(atype == AssociationType::Seq_Message || atype == AssociationType::Seq_Message_Self) {
            // show op dlg for seq. diagram here
            // don't worry about here, I don't think it can get here as
            // line is widget on seq. diagram
            // here just in case - remove later after testing
            DEBUG(DBG_SRC) << "mt_Properties: assoctype is " << atype;
        } else {  //standard assoc dialog
            m_scene->updateDocumentation( false );
            showPropertiesDialog();
        }
        break;

    case ListPopupMenu::mt_Delete:
        if (m_pAssocClassLineSel0)
            removeAssocClassLine();
        else if (association())
            m_scene->removeAssocInViewAndDoc(this);
        else
            m_scene->removeAssoc(this);
        break;

    case ListPopupMenu::mt_Rename_MultiA:
        r = RoleType::A;   // fall through
    case ListPopupMenu::mt_Rename_MultiB:
        if (m_role[r].multiplicityWidget)
            oldText = m_role[r].multiplicityWidget->text();
        else
            oldText = "";
        newText = KInputDialog::getText(i18n("Multiplicity"),
                                        i18n("Enter multiplicity:"),
                                        oldText, NULL, m_scene->activeView(),&v);
        if (newText != oldText) {
            if (FloatingTextWidget::isTextValid(newText)) {
                setMultiplicity(newText, r);
            } else {
                m_scene->removeWidget(m_role[r].multiplicityWidget);
                m_role[r].multiplicityWidget = NULL;
            }
        }
        break;

    case ListPopupMenu::mt_Rename_Name:
        if(m_nameWidget)
            oldText = m_nameWidget->text();
        else
            oldText = "";
        newText = KInputDialog::getText(i18n("Association Name"),
                                        i18n("Enter association name:"),
                                        oldText, NULL, m_scene->activeView(), &v);
        if (newText != oldText) {
            if (FloatingTextWidget::isTextValid(newText)) {
                setName(newText);
            } else {
                m_scene->removeWidget(m_nameWidget);
                m_nameWidget = NULL;
            }
        }
        break;

    case ListPopupMenu::mt_Rename_RoleAName:
        r = RoleType::A;   // fall through
    case ListPopupMenu::mt_Rename_RoleBName:
        if (m_role[r].roleWidget)
            oldText = m_role[r].roleWidget->text();
        else
            oldText = "";
        newText = KInputDialog::getText(i18n("Role Name"),
                                        i18n("Enter role name:"),
                                        oldText, NULL, m_scene->activeView(), &v);
        if (newText != oldText) {
            if (FloatingTextWidget::isTextValid(newText)) {
                setRoleName(newText, r);
            } else {
                m_scene->removeWidget(m_role[r].roleWidget);
                m_role[r].roleWidget = NULL;
            }
        }
        break;

    case ListPopupMenu::mt_Change_Font:
        {
            QFont fnt = font();
            if( KFontDialog::getFont( fnt, KFontChooser::NoDisplayFlags, m_scene->activeView() ) )
                lwSetFont(fnt);
        }
        break;

    case ListPopupMenu::mt_Change_Font_Selection:
        {
            QFont fnt = font();
            if( KFontDialog::getFont( fnt, KFontChooser::NoDisplayFlags, m_scene->activeView() ) ) {
                m_scene->selectionSetFont( fnt );
                m_umldoc->setModified(true);
            }
        }
        break;

    case ListPopupMenu::mt_Line_Color:
        {
            QColor newColor;
            if( KColorDialog::getColor(newColor) ) {
                m_scene->selectionSetLineColor(newColor);
                m_umldoc->setModified(true);
            }
        }
        break;

    case ListPopupMenu::mt_Cut:
        m_scene->setStartedCut();
        UMLApp::app()->slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        UMLApp::app()->slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app()->slotEditPaste();
        break;

    case ListPopupMenu::mt_Reset_Label_Positions:
        resetTextPositions();
        break;

    default:
        DEBUG(DBG_SRC) << "MenuType " << ListPopupMenu::toString(sel) << " not implemented";
        break;
    }//end switch
}


// find a general font for the association

/**
 * Return the first font found being used by any child widget. (They
 * could be different fonts, so this is a slightly misleading method.)
 */
QFont AssociationWidget::font() const
{
    QFont font;

    if( m_role[RoleType::A].roleWidget )
        font = m_role[RoleType::A].roleWidget->font( );
    else    if( m_role[RoleType::B].roleWidget)
        font = m_role[RoleType::B].roleWidget->font( );
    else    if( m_role[RoleType::A].multiplicityWidget )
        font = m_role[RoleType::A].multiplicityWidget->font( );
    else    if( m_role[RoleType::B].multiplicityWidget )
        font = m_role[RoleType::B].multiplicityWidget->font( );
    else    if( m_role[RoleType::A].changeabilityWidget)
        font = m_role[RoleType::A].changeabilityWidget->font( );
    else    if( m_role[RoleType::B].changeabilityWidget)
        font = m_role[RoleType::B].changeabilityWidget->font( );
    else    if( m_nameWidget)
        font = m_nameWidget->font( );
    else
        font = m_role[RoleType::A].umlWidget->font();

    return font;
}

/**
 * Set all 'owned' child widgets to this text color.
 */
void AssociationWidget::setTextColor(const QColor &color)
{
    WidgetBase::setTextColor(color);
    if( m_nameWidget) {
        m_nameWidget->setTextColor( color );
    }
    if( m_role[RoleType::A].roleWidget ) {
        m_role[RoleType::A].roleWidget->setTextColor( color );
    }
    if( m_role[RoleType::B].roleWidget ) {
        m_role[RoleType::B].roleWidget->setTextColor( color );
    }
    if( m_role[RoleType::A].multiplicityWidget ) {
        m_role[RoleType::A].multiplicityWidget->setTextColor( color );
    }
    if( m_role[RoleType::B].multiplicityWidget ) {
        m_role[RoleType::B].multiplicityWidget->setTextColor( color );
    }
    if( m_role[RoleType::A].changeabilityWidget)
        m_role[RoleType::A].changeabilityWidget->setTextColor( color );
    if( m_role[RoleType::B].changeabilityWidget)
        m_role[RoleType::B].changeabilityWidget->setTextColor( color );
}

/**
 * Overrides the method from WidgetBase.
 */
void AssociationWidget::setLineColor(const QColor &color)
{
    WidgetBase::setLineColor(color);
    m_associationLine->setLineColor(color);
}

/**
 * Overrides the method from WidgetBase.
 */
void AssociationWidget::setLineWidth(uint width)
{
    WidgetBase::setLineWidth(width);
    m_associationLine->setLineWidth(width);
}

void AssociationWidget::checkPoints(const UMLScenePoint &p)
{
    m_nMovingPoint = -1;
    //only check if more than the two endpoints
    int size = m_associationLine->count();
    if( size <= 2 )
        return;
    //check all points except the end points to see if we clicked on one of them
    UMLScenePoint tempPoint;
    int x, y;
    const int BOUNDARY = 4; // check for pixels around the point
    for (int i = 1; i < size - 1; ++i) {
        tempPoint = m_associationLine->point( i );
        x = tempPoint.x();
        y = tempPoint.y();
        if( x - BOUNDARY <= p.x() && x + BOUNDARY >= p.x() &&
                y - BOUNDARY <= p.y() && y + BOUNDARY >= p.y() ) {
            m_nMovingPoint = i;
            break; //no need to check the rest
        }//end if
    }//end for
}

/**
 * Moves the break point being dragged.
 */
void AssociationWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    if( me->buttons() != Qt::LeftButton) {
        return;
    }

    // if we have no moving point,create one
    if (m_nMovingPoint == -1)
    {
        //create moving point near the mouse on the line
        int i = m_associationLine->closestPointIndex(me->scenePos());

        if (i == -1)
            return;
        m_associationLine->insertPoint( i + 1, me->scenePos() );
        m_nMovingPoint = i + 1;
    }

    setSelected();
    //new position for point
    UMLScenePoint p = me->scenePos();

    if( m_scene->snapToGrid() ) {
        int newX = m_scene->snappedX( p.x() );
        int newY = m_scene->snappedY( p.y() );
        p.setX(newX);
        p.setY(newY);
    }

    // Prevent the moving vertex from disappearing underneath a widget
    // (else there's no way to get it back.)
    UMLWidget *onW = m_scene->widgetAt(p);
    if (onW && onW->baseType() != WidgetBase::wt_Box) {  // boxes are transparent
        const int pX = p.x();
        const int pY = p.y();
        const int wX = onW->x();
        const int wY = onW->y();
        const int wWidth = onW->width();
        const int wHeight = onW->height();
        if (pX > wX && pX < wX + wWidth) {
            const int midX = wX + wWidth / 2;
            if (pX <= midX)
                p.setX(wX);
            else
                p.setX(wX + wWidth);
        }
        if (pY > wY && pY < wY + wHeight) {
            const int midY = wY + wHeight / 2;
            if (pY <= midY)
                p.setY(wY);
            else
                p.setY(wY + wHeight);
        }
    }

    moveEvent(me);
    m_scene->resizeSceneToItems();
}

/**
 * Returns the Region the widget to line intersection is for the given
 * widget in this Association.  If the given widget is not in the
 * Association then Region::Error is returned.
 * Used by @ref calculateEndingPoints to work these positions out for
 * another Association - since the number of Associations on the same
 * region for the same widget will mean the lines will need to be
 * spread out across the region.
 */
//AssociationWidget::Region AssociationWidget::getWidgetRegion(AssociationWidget * widget) const
//{
//    if(widget->widgetForRole(RoleType::A) == m_role[RoleType::A].umlWidget)
//        return m_role[RoleType::A].m_WidgetRegion;
//    if(widget->widgetForRole(RoleType::B) == m_role[RoleType::B].umlWidget)
//        return m_role[RoleType::B].m_WidgetRegion;
//    return Error;
//}

/**
 * Returns the number of lines there are on the given region for
 * either widget A or B of the association.
 */
int AssociationWidget::getRegionCount(AssociationWidget::Region region, Uml::RoleType::Enum role)
{
    if(region == Error)
        return 0;
    int widgetCount = 0;
    AssociationWidgetList list = m_scene->associationList();
    foreach ( AssociationWidget* assocwidget, list ) {
        //don't count this association
        if (assocwidget == this)
            continue;
        const WidgetRole& otherA = assocwidget->m_role[RoleType::A];
        const WidgetRole& otherB = assocwidget->m_role[RoleType::B];
        const UMLWidget *a = otherA.umlWidget;
        const UMLWidget *b = otherB.umlWidget;
        /*
        //don't count associations to self if both of their end points are on the same region
        //they are different and placement won't interfere with them
        if( a == b && otherA.m_WidgetRegion == otherB.m_WidgetRegion )
                continue;
         */
        if (m_role[role].umlWidget == a && region == otherA.m_WidgetRegion)
            widgetCount++;
        else if (m_role[role].umlWidget == b && region == otherB.m_WidgetRegion)
            widgetCount++;
    }//end foreach
    return widgetCount;
}

UMLScenePoint AssociationWidget::findIntercept(const UMLSceneRect &rect, const UMLScenePoint &point)
{
    Region region = findPointRegion(rect, point.x(), point.y());
    /*
    const char *regionStr[] = { "Error",
        "West", "North", "East", "South",
        "NorthWest", "NorthEast", "SouthEast", "SouthWest",
        "Center"
    };
    DEBUG(DBG_SRC) << "findPointRegion(rect(" << rect.x() << "," << rect.y()
          << "," << rect.width() << "," << rect.height() << "), p("
          << point.x() << "," << point.y() << ")) = " << regionStr[region];
     */
    // Move some regions to the standard ones.
    switch (region) {
    case NorthWest:
        region = North;
        break;
    case NorthEast:
        region = East;
        break;
    case SouthEast:
        region = South;
        break;
    case SouthWest:
    case Center:
        region = West;
        break;
    default:
        break;
    }
    // The Qt coordinate system has (0,0) in the top left corner.
    // In order to go to the regular XY coordinate system with (0,0)
    // in the bottom left corner, we swap the X and Y axis.
    // That's why the following assignments look twisted.
    const int rectHalfWidth = rect.height() / 2;
    const int rectHalfHeight = rect.width() / 2;
    const int rectMidX = rect.y() + rectHalfWidth;
    const int rectMidY = rect.x() + rectHalfHeight;
    const int pX = point.y();
    const int pY = point.x();
    const int dX = rectMidX - pX;
    const int dY = rectMidY - pY;
    switch (region) {
    case West:
        region = South;
        break;
    case North:
        region = East;
        break;
    case East:
        region = North;
        break;
    case South:
        region = West;
        break;
    default:
        break;
    }
    // Now we have regular coordinates with the point (0,0) in the
    // bottom left corner.
    if (region == North || region == South) {
        int yoff = rectHalfHeight;
        if (region == North)
            yoff = -yoff;
        if (dX == 0) {
            return UMLScenePoint(rectMidY + yoff, rectMidX);  // swap back X and Y
        }
        if (dY == 0) {
            uError() << "usage error: " << "North/South (dY == 0)";
            return UMLScenePoint(0,0);
        }
        const float m = (float)dY / (float)dX;
        const float b = (float)pY - m * pX;
        const int inputY = rectMidY + yoff;
        const float outputX = ((float)inputY - b) / m;
        return UMLScenePoint(inputY, (int)outputX);  // swap back X and Y
    } else {
        int xoff = rectHalfWidth;
        if (region == East)
            xoff = -xoff;
        if (dY == 0)
            return UMLScenePoint(rectMidY, rectMidX + xoff);  // swap back X and Y
        if (dX == 0) {
            uError() << "usage error: " << "East/West (dX == 0)";
            return UMLScenePoint(0,0);
        }
        const float m = (float)dY / (float)dX;
        const float b = (float)pY - m * pX;
        const int inputX = rectMidX + xoff;
        const float outputY = m * (float)inputX + b;
        return UMLScenePoint((int)outputY, inputX);  // swap back X and Y
    }
}
/**
 * Given a rectangle and a point, findInterceptOnEdge computes the
 * connecting line between the middle point of the rectangle and
 * the point, and returns the intercept of this line with the
 * the edge of the rectangle identified by `region'.
 * When the region is North or South, the X value is returned (Y is
 * constant.)
 * When the region is East or West, the Y value is returned (X is
 * constant.)
 * @todo This is buggy. Try replacing by findIntercept()
 */
int AssociationWidget::findInterceptOnEdge(const UMLSceneRect &rect,
        AssociationWidget::Region region,
        const UMLScenePoint &point)
{
    // The Qt coordinate system has (0,0) in the top left corner.
    // In order to go to the regular XY coordinate system with (0,0)
    // in the bottom left corner, we swap the X and Y axis.
    // That's why the following assignments look twisted.
    const int rectHalfWidth = rect.height() / 2;
    const int rectHalfHeight = rect.width() / 2;
    const int rectMidX = rect.y() + rectHalfWidth;
    const int rectMidY = rect.x() + rectHalfHeight;
    const int dX = rectMidX - point.y();
    const int dY = rectMidY - point.x();
    switch (region) {
    case West:
        region = South;
        break;
    case North:
        region = West;
        break;
    case East:
        region = North;
        break;
    case South:
        region = East;
        break;
    default:
        break;
    }
    // Now we have regular coordinates with the point (0,0) in the
    // bottom left corner.
    if (region == North || region == South) {
        if (dX == 0)
            return rectMidY;
        // should be rectMidX, but we go back to Qt coord.sys.
        if (dY == 0) {
            uError() << "usage error: " << "North/South (dY == 0)";
            return -1;
        }
        const float m = (float)dY / (float)dX;
        float relativeX;
        if (region == North)
            relativeX = (float)rectHalfHeight / m;
        else
            relativeX = -(float)rectHalfHeight / m;
        return (rectMidY + (int)relativeX);
        // should be rectMidX, but we go back to Qt coord.sys.
    } else {
        if (dY == 0)
            return rectMidX;
        // should be rectMidY, but we go back to Qt coord.sys.
        if (dX == 0) {
            uError() << "usage error: " << "East/West (dX == 0)";
            return -1;
        }
        const float m = (float)dY / (float)dX;
        float relativeY = m * (float)rectHalfWidth;
        if (region == West)
            relativeY = -relativeY;
        return (rectMidX + (int)relativeY);
        // should be rectMidY, but we go back to Qt coord.sys.
    }
}

/**
 * Auxiliary method for updateAssociations():
 * Put position into m_positions and assoc into m_ordered at the
 * correct index.
 * m_positions and m_ordered move in parallel and are sorted by
 * ascending position.
 */
void AssociationWidget::insertIntoLists(int position, const AssociationWidget* assoc)
{
    bool did_insertion = false;
    for (int index = 0; index < m_positions_len; ++index) {
        if (position < m_positions[index]) {
            for (int moveback = m_positions_len; moveback > index; moveback--)
                m_positions[moveback] = m_positions[moveback - 1];
            m_positions[index] = position;
            m_ordered.insert(index, const_cast<AssociationWidget*>(assoc));
            did_insertion = true;
            break;
        }
    }
    if (! did_insertion) {
        m_positions[m_positions_len] = position;
        m_ordered.append(const_cast<AssociationWidget*>(assoc));
    }
    m_positions_len++;
}

/**
 * Tells all the other view associations the new count for the
 * given widget on a certain region. And also what index they should be.
 */
void AssociationWidget::updateAssociations(int totalCount,
        AssociationWidget::Region region,
        Uml::RoleType::Enum role)
{
    if( region == Error )
        return;
    AssociationWidgetList list = m_scene->associationList();

    UMLWidget *ownWidget = m_role[role].umlWidget;
    m_positions_len = 0;
    m_ordered.clear();
    // we order the AssociationWidget list by region and x/y value
    foreach ( AssociationWidget* assocwidget, list ) {
        WidgetRole *roleA = &assocwidget->m_role[RoleType::A];
        WidgetRole *roleB = &assocwidget->m_role[RoleType::B];
        UMLWidget *wA = roleA->umlWidget;
        UMLWidget *wB = roleB->umlWidget;
        // Skip self associations.
        if (wA == wB)
            continue;
        // Now we must find out with which end the assocwidget connects
        // to the input widget (ownWidget).
        bool inWidgetARegion = ( ownWidget == wA &&
                                 region == roleA->m_WidgetRegion );
        bool inWidgetBRegion = ( ownWidget == wB &&
                                 region == roleB->m_WidgetRegion);
        if ( !inWidgetARegion && !inWidgetBRegion )
            continue;
        // Determine intercept position on the edge indicated by `region'.
        UMLWidget * otherWidget = (inWidgetARegion ? wB : wA);
        AssociationLine *linepath = assocwidget->associationLine();
        UMLScenePoint refpoint;
        if (assocwidget->linePathStartsAt(otherWidget))
            refpoint = linepath->point(linepath->count() - 2);
        else
            refpoint = linepath->point(1);
        // The point is authoritative if we're called for the second time
        // (i.e. role==B) or it is a waypoint on the line path.
        bool pointIsAuthoritative = (role == RoleType::B || linepath->count() > 2);
        if (! pointIsAuthoritative) {
            // If the point is not authoritative then we use the other
            // widget's center.
            refpoint.setX(otherWidget->x() + otherWidget->width() / 2);
            refpoint.setY(otherWidget->y() + otherWidget->height() / 2);
        }
        int intercept = findInterceptOnEdge(ownWidget->rect(), region, refpoint);
        if (intercept < 0) {
            DEBUG(DBG_SRC) << "updateAssociations: error from findInterceptOnEdge for"
            << " assocType=" << assocwidget->associationType()
            << " ownWidget=" << ownWidget->name()
            << " otherWidget=" << otherWidget->name();
            continue;
        }
        insertIntoLists(intercept, assocwidget);
    } // while ( (assocwidget = assoc_it.current()) )

    // we now have an ordered list and we only have to call updateRegionLineCount
    int index = 1;
    foreach (AssociationWidget* assocwidget , m_ordered ) {
        if (ownWidget == assocwidget->widgetForRole(RoleType::A)) {
            assocwidget->updateRegionLineCount(index++, totalCount, region, RoleType::A);
        } else if (ownWidget == assocwidget->widgetForRole(RoleType::B)) {
            assocwidget->updateRegionLineCount(index++, totalCount, region, RoleType::B);
        }
    } // for (assocwidget = ordered.first(); ...)
}

/**
 * Called to tell the association that another association has added
 * a line to the region of one of its widgets. The widget is identified
 * by its role (A or B).
 *
 * Called by @ref updateAssociations which is called by
 * @ref calculateEndingPoints when required.
 */
void AssociationWidget::updateRegionLineCount(int index, int totalCount,
        AssociationWidget::Region region,
        Uml::RoleType::Enum role)
{
    if( region == Error )
        return;
    // If the association is to self and the line ends are on the same region then
    // use a different calculation.
    if (m_role[RoleType::A].umlWidget == m_role[RoleType::B].umlWidget &&
            m_role[RoleType::A].m_WidgetRegion == m_role[RoleType::B].m_WidgetRegion) {
        UMLWidget * pWidget = m_role[RoleType::A].umlWidget;
        int x = pWidget->x();
        int y = pWidget->y();
        int wh = pWidget->height();
        int ww = pWidget->width();
        int size = m_associationLine->count();
        // See if above widget ok to place assoc.
        switch( m_role[RoleType::A].m_WidgetRegion ) {
        case North:
            m_associationLine->setPoint( 0, UMLScenePoint( x + ( ww / 4 ), y ) );
            m_associationLine->setPoint( size - 1, UMLScenePoint(x + ( ww * 3 / 4 ), y ) );
            break;

        case South:
            m_associationLine->setPoint( 0, UMLScenePoint( x + ( ww / 4 ), y + wh ) );
            m_associationLine->setPoint( size - 1, UMLScenePoint( x + ( ww * 3 / 4 ), y + wh ) );
            break;

        case East:
            m_associationLine->setPoint( 0, UMLScenePoint( x + ww, y + ( wh / 4 ) ) );
            m_associationLine->setPoint( size - 1, UMLScenePoint( x + ww, y + ( wh * 3 / 4 ) ) );
            break;

        case West:
            m_associationLine->setPoint( 0, UMLScenePoint( x, y + ( wh / 4 ) ) );
            m_associationLine->setPoint( size - 1, UMLScenePoint( x, y + ( wh * 3 / 4 ) ) );
            break;
        default:
            break;
        }//end switch
        m_role[RoleType::A].m_OldCorner.setX( x );
        m_role[RoleType::A].m_OldCorner.setY( y );
        m_role[RoleType::B].m_OldCorner.setX( x );
        m_role[RoleType::B].m_OldCorner.setY( y );

        return;
    }

    WidgetRole& robj = m_role[role];
    UMLWidget * pWidget = robj.umlWidget;

    robj.m_nIndex = index;
    robj.m_nTotalCount = totalCount;
    int x = pWidget->x();
    int y = pWidget->y();
    robj.m_OldCorner.setX(x);
    robj.m_OldCorner.setY(y);
    int ww = pWidget->width();
    int wh = pWidget->height();
    const bool angular = Settings::optionState().generalState.angularlines;
    int ch = 0;
    int cw = 0;
    if (angular) {
        uint nind = (role == RoleType::A ? 1 : m_associationLine->count() - 2);
        UMLScenePoint neighbour = m_associationLine->point(nind);
        if (neighbour.x() < x)
            cw = 0;
        else if (neighbour.x() > x + ww)
            cw = 0 + ww;
        else
            cw = neighbour.x() - x;
        if (neighbour.y() < y)
            ch = 0;
        else if (neighbour.y() > y + wh)
            ch = 0 + wh;
        else
            ch = neighbour.y() - y;
    } else {
        ch = wh * index / totalCount;
        cw = ww * index / totalCount;
    }

    int snapX = m_scene->snappedX(x + cw);
    int snapY = m_scene->snappedY(y + ch);

    UMLScenePoint pt;
    if (angular) {
        pt = UMLScenePoint(snapX, snapY);
    } else {
        switch(region) {
            case West:
                pt.setX(x);
                pt.setY(snapY);
                break;
            case North:
                pt.setX(snapX);
                pt.setY(y);
                break;
            case East:
                pt.setX(x + ww);
                pt.setY(snapY);
                break;
            case South:
                pt.setX(snapX);
                pt.setY(y + wh);
                break;
            case Center:
                pt.setX(x + ww / 2);
                pt.setY(y + wh / 2);
                break;
            default:
                break;
        }
    }
    if (role == RoleType::A)
        m_associationLine->setPoint( 0, pt );
    else {
        m_associationLine->setPoint( m_associationLine->count() - 1, pt );
        AssociationLine::Region r = ( region == South || region == North ) ?
                             AssociationLine::TopBottom : AssociationLine::LeftRight;
        m_associationLine->setDockRegion( r );
    }
}

/**
 * Returns the state of whether the widget is selected.
 *
 * @return  Returns the state of whether the widget is selected.
 */
bool AssociationWidget::isSelected() const
{
    return m_selected;
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select   The state of whether the widget is selected.
 */
void AssociationWidget::setSelected(bool _select /* = true */)
{
    m_selected = _select;
    if( m_nameWidget)
        m_nameWidget->setSelected( _select );
    if( m_role[RoleType::A].roleWidget )
        m_role[RoleType::A].roleWidget->setSelected( _select );
    if( m_role[RoleType::B].roleWidget )
        m_role[RoleType::B].roleWidget->setSelected( _select );
    if( m_role[RoleType::A].multiplicityWidget )
        m_role[RoleType::A].multiplicityWidget->setSelected( _select );
    if( m_role[RoleType::B].multiplicityWidget )
        m_role[RoleType::B].multiplicityWidget->setSelected( _select );
    if( m_role[RoleType::A].changeabilityWidget)
        m_role[RoleType::A].changeabilityWidget->setSelected( _select );
    if( m_role[RoleType::B].changeabilityWidget)
        m_role[RoleType::B].changeabilityWidget->setSelected( _select );

    // Why call the following ? It makes sense only if there is  a long operation going on.
    qApp->processEvents();
    //Update the docwindow for this association.
    // This is done last because each of the above setSelected calls
    // overwrites the docwindow, but we want the main association doc
    // to win.
    if( _select ) {
        if( m_scene->selectedCount() == 0 )
                m_scene->showDocumentation( this, false );
    } else
        m_scene->updateDocumentation( true );
    qApp->processEvents();

    m_associationLine->setSelected( _select );
    if (! _select) {
        // For now, if _select is true we don't make the assoc class line
        // selected. But that's certainly open for discussion.
        // At any rate, we need to deselect the assoc class line
        // if _select is false.
        selectAssocClassLine(false);
    }
}

/**
 * Returns true if the given point is on the connecting line to
 * the association class. Returns false if there is no association
 * class attached, or if the given point is not on the connecting
 * line.
 */
bool AssociationWidget::onAssocClassLine(const UMLScenePoint &point)
{
    if (m_pAssocClassLine == NULL)
        return false;
    UMLSceneItemList list = m_scene->collisions(point);
    UMLSceneItemList::iterator end(list.end());
    for (UMLSceneItemList::iterator item_it(list.begin()); item_it != end; ++item_it) {
        if (*item_it == m_pAssocClassLine)
            return true;
    }
    return false;
}

/**
 * Returns true if the given point is on the Association.
 */
bool AssociationWidget::onAssociation(const UMLScenePoint & point)
{
    if (m_associationLine->closestPointIndex(point) != -1)
        return true;
    return onAssocClassLine(point);
}

/**
 * This slot is entered when an event has occurred on the views display,
 * most likely a mouse event. Before it sends out that mouse event all
 * children should make sure that they don't have a menu active or there
 * could be more than one popup menu displayed.
 */
void AssociationWidget::slotRemovePopupMenu()
{
    if (m_pMenu) {
        if (m_nameWidget) {
            m_nameWidget->slotRemovePopupMenu();
        }
        disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

/**
 * Handles any signals that tells everyone not to be selected.
 */
void AssociationWidget::slotClearAllSelected()
{
    setSelected( false );
}

/**
 * Moves all the mid points (all expcept start /end ) by the given amount.
 */
void AssociationWidget::moveMidPointsBy( int x, int y )
{
    int pos = m_associationLine->count() - 1;
    for (int i = 1; i < (int)pos; ++i) {
        UMLScenePoint p = m_associationLine->point( i );
        int newX = p.x() + x;
        int newY = p.y() + y;
        newX = m_scene->snappedX( newX );
        newY = m_scene->snappedY( newY );
        p.setX( newX );
        p.setY( newY );
        m_associationLine->setPoint( i, p );
    }
}

/**
 * Moves the entire association by the given offset.
 */
void AssociationWidget::moveEntireAssoc( int x, int y )
{
    //TODO: ADD SUPPORT FOR ASSOC. ON SEQ. DIAGRAMS WHEN NOTES BACK IN.
    moveMidPointsBy( x, y );
    calculateEndingPoints();
    calculateNameTextSegment();
    resetTextPositions();
}

/**
 * Returns the bounding rectangle of all segments of the association.
 */
QRectF AssociationWidget::boundingRect() const
{
    QRectF rectangle;

    /* we also want the end points connected to the other widget */
    int pos = m_associationLine->count();

    /* the lines have the width of the pen */
    uint pen_width = m_associationLine->pen().width();

    if (pen_width == 0)
        pen_width = 1; // width must be at least 1

    /* go through all points on the linepath */
    for (int i = 0; i < pos; ++i)
    {
        UMLScenePoint p = m_associationLine->point(i);

        /* the first point is our starting point */
        if (i == 0) {
            rectangle.setRect(p.x(), p.y(), 0, 0);
            continue;
        }

        if (p.x() < rectangle.x())
            rectangle.setX(p.x());
        if (p.y() < rectangle.y())
            rectangle.setY(p.y());
        if (p.x() > rectangle.x() + rectangle.width()) {
            int newX = p.x() - rectangle.x() + pen_width;
            rectangle.setWidth(abs(newX));
        }
        if (p.y() > rectangle.y() + rectangle.height()) {
            int newY = p.y() - rectangle.y() + pen_width;
            rectangle.setHeight(abs(newY));
        }
    }
    return rectangle;
}

/**
 * Connected to UMLClassifier::attributeRemoved() or UMLEntity::constraintRemoved()
 * in case this AssociationWidget is linked to a clasifier list item
 * ( an attribute or a foreign key constraint )
 *
 * @param obj               The UMLClassifierListItem removed.
 */
void AssociationWidget::slotClassifierListItemRemoved(UMLClassifierListItem* obj)
{
    if (obj != m_umlObject) {
        DEBUG(DBG_SRC) << "obj=" << obj << ": m_umlObject=" << m_umlObject;
        return;
    }
    m_umlObject = NULL;
    m_scene->removeAssoc(this);
}

/**
 * Connected to UMLObject::modified() in case this
 * AssociationWidget is linked to a classifer's attribute type.
 */
void AssociationWidget::slotAttributeChanged()
{
    UMLAttribute *attr = attribute();
    if (attr == NULL) {
        uError() << "attribute() returns NULL";
        return;
    }
    setVisibility(attr->visibility(), RoleType::B);
    setRoleName(attr->name(), RoleType::B);
}


/**
 * Sets the Association line index for the given role.
 */
void AssociationWidget::setIndex(int index, Uml::RoleType::Enum role)
{
    m_role[role].m_nIndex = index;
}

/**
 * Returns the Association line index for the given role.
 */
int AssociationWidget::getIndex(Uml::RoleType::Enum role) const
{
    return m_role[role].m_nIndex;
}

void AssociationWidget::clipSize()
{
    if( m_nameWidget )
        m_nameWidget->clipSize();

    if( m_role[RoleType::A].multiplicityWidget )
        m_role[RoleType::A].multiplicityWidget->clipSize();

    if( m_role[RoleType::B].multiplicityWidget )
        m_role[RoleType::B].multiplicityWidget->clipSize();

    if( m_role[RoleType::A].roleWidget )
        m_role[RoleType::A].roleWidget->clipSize();

    if( m_role[RoleType::B].roleWidget )
        m_role[RoleType::B].roleWidget->clipSize();

    if( m_role[RoleType::A].changeabilityWidget )
        m_role[RoleType::A].changeabilityWidget->clipSize();

    if( m_role[RoleType::B].changeabilityWidget )
        m_role[RoleType::B].changeabilityWidget->clipSize();

    if (m_associationClass)
        m_associationClass->clipSize();
}

ListPopupMenu* AssociationWidget::setupPopupMenu(ListPopupMenu *menu, const QPointF &p)
{
    Q_UNUSED(menu)
    //work out the type of menu we want
    //work out if the association allows rolenames, multiplicity, etc
    //also must be within a certain distance to be a multiplicity menu
    ListPopupMenu::MenuType menuType = ListPopupMenu::mt_Undefined;
    const int DISTANCE = 40;//must be within this many pixels for it to be a multi menu
    const UMLScenePoint lpStart = m_associationLine->point(0);
    const UMLScenePoint lpEnd = m_associationLine->point(m_associationLine->count() - 1);
    const int startXDiff = lpStart.x() - p.x();
    const int startYDiff = lpStart.y() - p.y();
    const int endXDiff = lpEnd.x() - p.x();
    const int endYDiff = lpEnd.y() - p.y();
    const float lengthMAP = sqrt( double(startXDiff * startXDiff + startYDiff * startYDiff) );
    const float lengthMBP = sqrt( double(endXDiff * endXDiff + endYDiff * endYDiff) );
    const Uml::AssociationType::Enum type = associationType();
    //allow multiplicity
    if( AssocRules::allowMultiplicity( type, widgetForRole(RoleType::A)->baseType() ) ) {
        if(lengthMAP < DISTANCE)
            menuType =  ListPopupMenu::mt_MultiA;
        else if(lengthMBP < DISTANCE)
            menuType = ListPopupMenu::mt_MultiB;
    }
    if( menuType == ListPopupMenu::mt_Undefined ) {
        if (type == AssociationType::Anchor || onAssocClassLine(p))
            menuType = ListPopupMenu::mt_Anchor;
        else if (isCollaboration())
            menuType = ListPopupMenu::mt_Collaboration_Message;
        else if (association() == NULL)
            menuType = ListPopupMenu::mt_AttributeAssociation;
        else if (AssocRules::allowRole(type))
            menuType = ListPopupMenu::mt_FullAssociation;
        else
            menuType = ListPopupMenu::mt_Association_Selected;
    }
    m_pMenu = new ListPopupMenu(m_scene->activeView(), menuType);
    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
    if (isCollaboration())
        m_nameWidget->setupPopupMenu(m_pMenu);
    return m_pMenu;
}

/**
 * Initialize attributes of this class at construction time.
 */
void AssociationWidget::init()
{
    // pointers to floating text widgets objects owned by this association
    m_nameWidget = 0;
    m_role[RoleType::A].changeabilityWidget = 0;
    m_role[RoleType::B].changeabilityWidget = 0;
    m_role[RoleType::A].multiplicityWidget = 0;
    m_role[RoleType::B].multiplicityWidget = 0;
    m_role[RoleType::A].roleWidget = 0;
    m_role[RoleType::B].roleWidget = 0;
    m_role[RoleType::A].umlWidget = 0;
    m_role[RoleType::B].umlWidget = 0;

    // associationwidget attributes
    m_role[RoleType::A].m_WidgetRegion = Error;
    m_role[RoleType::B].m_WidgetRegion = Error;
    m_role[RoleType::A].m_nIndex = 0;
    m_role[RoleType::B].m_nIndex = 0;
    m_role[RoleType::A].m_nTotalCount = 0;
    m_role[RoleType::B].m_nTotalCount = 0;
    m_role[RoleType::A].visibility = Uml::Visibility::Public;
    m_role[RoleType::B].visibility = Uml::Visibility::Public;
    m_role[RoleType::A].changeability = Uml::Changeability::Changeable;
    m_role[RoleType::B].changeability = Uml::Changeability::Changeable;
    m_positions_len = 0;
    m_activated = false;
    m_unNameLineSegment = 0;
    m_pMenu = 0;
    m_selected = false;
    m_nMovingPoint = -1;
    m_nLinePathSegmentIndex = -1;
    m_associationLine = new AssociationLine;
    m_associationLine->setAssociation( this );
    m_associationClass = NULL;
    m_pAssocClassLine = NULL;
    m_pAssocClassLineSel0 = m_pAssocClassLineSel1 = NULL;

    // Initialize local members.
    // These are only used if we don't have a UMLAssociation attached.
    m_associationType = Uml::AssociationType::Association;
    m_umldoc = UMLApp::app()->document();

    connect(m_scene, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
    connect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));
}

/**
 * Saves this widget to the "assocwidget" XMI element.
 */
void AssociationWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement assocElement = qDoc.createElement("assocwidget");

    WidgetBase::saveToXMI(qDoc, assocElement);
    if (m_umlObject) {
        assocElement.setAttribute("xmi.id", ID2STR(m_umlObject->id()));
    }
    assocElement.setAttribute("type", associationType());
    if (!association()) {
        assocElement.setAttribute("visibilityA", visibility(RoleType::A));
        assocElement.setAttribute("visibilityB", visibility(RoleType::B));
        assocElement.setAttribute("changeabilityA", changeability(RoleType::A));
        assocElement.setAttribute("changeabilityB", changeability(RoleType::B));
        if (m_umlObject == NULL) {
            assocElement.setAttribute("roleAdoc", roleDocumentation(RoleType::A));
            assocElement.setAttribute("roleBdoc", roleDocumentation(RoleType::B));
            assocElement.setAttribute("documentation", documentation());
        }
    }
    assocElement.setAttribute("widgetaid", ID2STR(widgetIDForRole(RoleType::A)));
    assocElement.setAttribute("widgetbid", ID2STR(widgetIDForRole(RoleType::B)));
    assocElement.setAttribute("indexa", m_role[RoleType::A].m_nIndex);
    assocElement.setAttribute("indexb", m_role[RoleType::B].m_nIndex);
    assocElement.setAttribute("totalcounta", m_role[RoleType::A].m_nTotalCount);
    assocElement.setAttribute("totalcountb", m_role[RoleType::B].m_nTotalCount);
    m_associationLine->saveToXMI(qDoc, assocElement);

    if (m_nameWidget) {
        m_nameWidget->saveToXMI(qDoc, assocElement);
    }

    if (multiplicityWidget(RoleType::A)) {
        multiplicityWidget(RoleType::A)->saveToXMI(qDoc, assocElement);
    }

    if (multiplicityWidget(RoleType::B)) {
        multiplicityWidget(RoleType::B)->saveToXMI(qDoc, assocElement);
    }

    if (roleWidget(RoleType::A)) {
        roleWidget(RoleType::A)->saveToXMI(qDoc, assocElement);
    }

    if (roleWidget(RoleType::B)) {
        roleWidget(RoleType::B)->saveToXMI(qDoc, assocElement);
    }

    if (changeabilityWidget(RoleType::A)) {
        changeabilityWidget(RoleType::A)->saveToXMI(qDoc, assocElement);
    }

    if (changeabilityWidget(RoleType::B)) {
        changeabilityWidget(RoleType::B)->saveToXMI(qDoc, assocElement);
    }

    if (m_associationClass) {
        QString acid = ID2STR(m_associationClass->id());
        assocElement.setAttribute("assocclass", acid);
        assocElement.setAttribute("aclsegindex", m_nLinePathSegmentIndex);
    }

    qElement.appendChild(assocElement);
}

/**
 * Uses the supplied widgetList for resolving
 * the role A and role B widgets. (The other loadFromXMI() queries
 * the UMLView for these widgets.)
 * Required for clipboard operations.
 */
bool AssociationWidget::loadFromXMI(QDomElement& qElement,
                                    const UMLWidgetList& widgets,
                                    const MessageWidgetList* messages)
{
    if (!WidgetBase::loadFromXMI(qElement)) {
        return false;
    }

    // load child widgets first
    QString widgetaid = qElement.attribute("widgetaid", "-1");
    QString widgetbid = qElement.attribute("widgetbid", "-1");
    Uml::ID::Type aId = STR2ID(widgetaid);
    Uml::ID::Type bId = STR2ID(widgetbid);
    UMLWidget *pWidgetA = Widget_Utils::findWidget(aId, widgets, messages);
    if (!pWidgetA) {
        uError() << "cannot find widget for roleA id " << ID2STR(aId);
        return false;
    }
    UMLWidget *pWidgetB = Widget_Utils::findWidget(bId, widgets, messages);
    if (!pWidgetB) {
        uError() << "cannot find widget for roleB id " << ID2STR(bId);
        return false;
    }
    setWidgetForRole(pWidgetA, RoleType::A);
    setWidgetForRole(pWidgetB, RoleType::B);

    QString type = qElement.attribute("type", "-1");
    Uml::AssociationType::Enum aType = Uml::AssociationType::fromInt(type.toInt());

    QString id = qElement.attribute("xmi.id", "-1");
    bool oldStyleLoad = false;
    if (id == "-1") {
        // xmi.id not present, ergo either a pure widget association,
        // or old (pre-1.2) style:
        // Everything is loaded from the AssociationWidget.
        // UMLAssociation may or may not be saved - if it is, it's a dummy.
        // Create the UMLAssociation if both roles are UML objects;
        // else load the info locally.

        if (Uml::AssociationType::hasUMLRepresentation(aType)) {
            // lack of an association in our widget AND presence of
            // both uml objects for each role clearly identifies this
            // as reading in an old-school file. Note it as such, and
            // create, and add, the UMLAssociation for this widget.
            // Remove this special code when backwards compatibility
            // with older files isn't important anymore. -b.t.
            UMLObject* umlRoleA = pWidgetA->umlObject();
            UMLObject* umlRoleB = pWidgetB->umlObject();
            if (!m_umlObject && umlRoleA && umlRoleB)
            {
                oldStyleLoad = true; // flag for further special config below
                if (aType == AssociationType::Aggregation || aType == AssociationType::Composition) {
                    uWarning()<<" Old Style save file? swapping roles on association widget"<<this;
                    // We have to swap the A and B widgets to compensate
                    // for the long standing bug in AssociationLine of drawing
                    // the diamond at the wrong end which was fixed
                    // just before the 1.2 release.
                    // The logic here is that the user has understood
                    // that the diamond belongs at the SOURCE end of the
                    // the association (i.e. at the container, not at the
                    // contained), and has compensated for this anomaly
                    // by drawing the aggregations/compositions from
                    // target to source.
                    UMLWidget *tmpWidget = pWidgetA;
                    pWidgetA = pWidgetB;
                    pWidgetB = tmpWidget;
                    setWidgetForRole(pWidgetA, RoleType::A);
                    setWidgetForRole(pWidgetB, RoleType::B);
                    umlRoleA = pWidgetA->umlObject();
                    umlRoleB = pWidgetB->umlObject();
                }

                setUMLAssociation(m_umldoc->createUMLAssociation(umlRoleA, umlRoleB, aType));
            }
        }

        setDocumentation(qElement.attribute("documentation", ""));
        setRoleDocumentation(qElement.attribute("roleAdoc", ""), RoleType::A);
        setRoleDocumentation(qElement.attribute("roleBdoc", ""), RoleType::B);

        // visibility defaults to Public if it cant set it here..
        QString visibilityA = qElement.attribute("visibilityA", "0");
        int vis = visibilityA.toInt();
        if (vis >= 200) {  // bkwd compat.
            vis -= 200;
        }
        setVisibility((Uml::Visibility::Enum)vis, RoleType::A);

        QString visibilityB = qElement.attribute("visibilityB", "0");
        vis = visibilityB.toInt();
        if (vis >= 200) { // bkwd compat.
            vis -= 200;
        }
        setVisibility((Uml::Visibility::Enum)vis, RoleType::B);

        // Changeability defaults to "Changeable" if it cant set it here..
        QString changeabilityA = qElement.attribute( "changeabilityA", "0");
        if (changeabilityA.toInt() > 0)
            setChangeability(Uml::Changeability::fromInt(changeabilityA.toInt()), RoleType::A);

        QString changeabilityB = qElement.attribute( "changeabilityB", "0");
        if (changeabilityB.toInt() > 0)
            setChangeability(Uml::Changeability::fromInt(changeabilityB.toInt()), RoleType::B);

    } else {

        // we should disconnect any prior association (can this happen??)
        if (m_umlObject && m_umlObject->baseType() == UMLObject::ot_Association)
        {
            UMLAssociation *umla = association();
            umla->disconnect(this);
            umla->nrof_parent_widgets--;
        }

        // New style: The xmi.id is a reference to the UMLAssociation.
        // If the UMLObject is not found right now, we try again later
        // during the type resolution pass - see activate().
        m_nId = STR2ID(id);
        UMLObject *myObj = m_umldoc->findObjectById(m_nId);
        if (myObj) {
            const UMLObject::ObjectType ot = myObj->baseType();
            if (ot != UMLObject::ot_Association) {
                setUMLObject(myObj);
            } else {
                UMLAssociation * myAssoc = static_cast<UMLAssociation*>(myObj);
                setUMLAssociation(myAssoc);
                if (type == "-1")
                    aType = myAssoc->getAssocType();
            }
        }
    }

    setAssociationType(aType);

    QString indexa = qElement.attribute( "indexa", "0" );
    QString indexb = qElement.attribute( "indexb", "0" );
    QString totalcounta = qElement.attribute( "totalcounta", "0" );
    QString totalcountb = qElement.attribute( "totalcountb", "0" );
    m_role[RoleType::A].m_nIndex = indexa.toInt();
    m_role[RoleType::B].m_nIndex = indexb.toInt();
    m_role[RoleType::A].m_nTotalCount = totalcounta.toInt();
    m_role[RoleType::B].m_nTotalCount = totalcountb.toInt();

    QString assocclassid = qElement.attribute("assocclass", "");
    if (! assocclassid.isEmpty()) {
        Uml::ID::Type acid = STR2ID(assocclassid);
        UMLWidget *w = Widget_Utils::findWidget(acid, widgets);
        if (w) {
            m_associationClass = static_cast<ClassifierWidget*>(w);
            m_associationClass->setClassAssociationWidget(this);
            // Preparation of the assoc class line is done in activate()
            QString aclsegindex = qElement.attribute("aclsegindex", "0");
            m_nLinePathSegmentIndex = aclsegindex.toInt();
        } else {
            uError() << "cannot find assocclass " << assocclassid;
        }
    }

    //now load child elements
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    while(!element.isNull()) {
        QString tag = element.tagName();
        if( tag == "linepath" ) {
            if( !m_associationLine->loadFromXMI( element ) )
                return false;
            else {
                // set up 'old' corner from first point in line
                // as IF this ISNT done, then the subsequent call to
                // widgetMoved will inadvertantly think we have made a
                // big move in the position of the association when we haven't.
                UMLScenePoint p = m_associationLine->point(0);
                m_role[RoleType::A].m_OldCorner.setX(p.x());
                m_role[RoleType::A].m_OldCorner.setY(p.y());
            }
        } else if (tag == "floatingtext" ||
                   tag == "UML:FloatingTextWidget") {  // for bkwd compatibility
            QString r = element.attribute( "role", "-1");
            if( r == "-1" )
                return false;
            Uml::TextRole::Enum role = Uml::TextRole::fromInt(r.toInt());
            FloatingTextWidget *ft = new FloatingTextWidget(m_scene, role, "", Uml::ID::Reserved);
            if( ! ft->loadFromXMI(element) ) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete ft;
                node = element.nextSibling();
                element = node.toElement();
                continue;
            }
            // always need this
            ft->setLink(this);

            switch( role ) {
            case Uml::TextRole::MultiA:
                m_role[RoleType::A].multiplicityWidget = ft;
                if(oldStyleLoad)
                    setMultiplicity(m_role[RoleType::A].multiplicityWidget->text(), RoleType::A);
                break;

            case Uml::TextRole::MultiB:
                m_role[RoleType::B].multiplicityWidget = ft;
                if(oldStyleLoad)
                    setMultiplicity(m_role[RoleType::B].multiplicityWidget->text(), RoleType::B);
                break;

            case Uml::TextRole::ChangeA:
                m_role[RoleType::A].changeabilityWidget = ft;
                break;

            case Uml::TextRole::ChangeB:
                m_role[RoleType::B].changeabilityWidget = ft;
                break;

            case Uml::TextRole::Name:
                m_nameWidget = ft;
                if(oldStyleLoad)
                    setName(m_nameWidget->text());
                break;

            case Uml::TextRole::Coll_Message:
            case Uml::TextRole::Coll_Message_Self:
                m_nameWidget = ft;
                ft->setLink(this);
                ft->setActivated();
                if(FloatingTextWidget::isTextValid(ft->text()))
                    ft->show();
                else
                    ft->hide();
                break;

            case Uml::TextRole::RoleAName:
                m_role[RoleType::A].roleWidget = ft;
                setRoleName( ft->text(), RoleType::A );
                break;
            case Uml::TextRole::RoleBName:
                m_role[RoleType::B].roleWidget = ft;
                setRoleName( ft->text(), RoleType::B );
                break;
            default:
                DEBUG(DBG_SRC) << "unexpected FloatingTextWidget (TextRole::Enum " << role << ")";
                delete ft;
                break;
            }
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    return true;
}

/**
 * Queries the UMLView for resolving the role A and role B widgets.
 * ....
 */
bool AssociationWidget::loadFromXMI(QDomElement& qElement)
{
    UMLScene *scene = umlScene();
    if (scene) {
        const UMLWidgetList& widgetList = scene->widgetList();
        const MessageWidgetList& messageList = scene->messageList();
        return loadFromXMI(qElement, widgetList, &messageList);
    }
    else {
        DEBUG(DBG_SRC) << "This isn't on UMLScene yet, so can neither fetch"
            "messages nor widgets on umlscene";
        return false;
    }
}

#include "associationwidget.moc"
