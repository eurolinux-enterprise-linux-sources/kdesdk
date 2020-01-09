/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License js published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003       Alexander Blum  <blum@kewbee.de>             *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "jswriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "operation.h"
#include "umldoc.h"

#include <kdebug.h>

#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

JSWriter::JSWriter()
{
}

JSWriter::~JSWriter()
{
}

void JSWriter::writeClass(UMLClassifier *c)
{
    if (!c)
    {
        uDebug()<<"Cannot write class of NULL concept!";
        return;
    }

    QString classname = cleanName(c->getName());
    QString fileName = c->getName().toLower();

    //find an appropriate name for our file
    fileName = findFileName(c,".js");
    if (fileName.isEmpty())
    {
        emit codeGenerated(c, false);
        return;
    }

    QFile filejs;
    if (!openFile(filejs, fileName))
    {
        emit codeGenerated(c, false);
        return;
    }
    QTextStream js(&filejs);

    //////////////////////////////
    //Start generating the code!!
    /////////////////////////////

    //try to find a heading file (license, coments, etc)
    QString str;
    str = getHeadingFile(".js");
    if (!str.isEmpty())
    {
        str.replace(QRegExp("%filename%"),fileName);
        str.replace(QRegExp("%filepath%"),filejs.fileName());
        js << str << m_endl;
    }

    //write includes
    UMLPackageList includes;
    findObjectsRelated(c,includes);
    foreach (UMLPackage* conc,  includes ) {
        QString headerName = findFileName(conc, ".js");
        if ( !headerName.isEmpty() )
        {
            js << "#include \"" << headerName << "\"" << m_endl;
        }
    }
    js << m_endl;

    //Write class Documentation if there is somthing or if force option
    if (forceDoc() || !c->getDoc().isEmpty())
    {
        js << m_endl << "/**" << m_endl;
        js << "  * class " << classname << m_endl;
        js << formatDoc(c->getDoc(),"  * ");
        js << "  */" << m_endl << m_endl;
    }

    //check if class is abstract and / or has abstract methods
    if (c->getAbstract() && !hasAbstractOps(c))
        js << "/******************************* Abstract Class ****************************" << m_endl << "  "
        << classname << " does not have any pure virtual methods, but its author" << m_endl
        << "  defined it as an abstract class, so you should not use it directly." << m_endl
        << "  Inherit from it instead and create only objects from the derived classes" << m_endl
        << "*****************************************************************************/" << m_endl << m_endl;

    js << classname << " = function ()" << m_endl;
    js << "{" << m_endl;
    js << m_indentation << "this._init ();" << m_endl;
    js << "}" << m_endl;
    js << m_endl;

    UMLClassifierList superclasses = c->getSuperClasses();
    foreach (UMLClassifier *obj , superclasses ) {
        js << classname << ".prototype = new " << cleanName(obj->getName()) << " ();" << m_endl;
    }

    js << m_endl;

    if (! c->isInterface()) {
        UMLAttributeList atl = c->getAttributeList();

        js << "/**" << m_endl;
        QString temp = "_init sets all " + classname + " attributes to their default value."
                       " Make sure to call this method within your class constructor";
        js << formatDoc(temp, " * ");
        js << " */" << m_endl;
        js << classname << ".prototype._init = function ()" << m_endl;
        js << "{" << m_endl;
        foreach (UMLAttribute *at, atl ) {
            if (forceDoc() || !at->getDoc().isEmpty())
            {
                js << m_indentation << "/**" << m_endl
                << formatDoc(at->getDoc(), m_indentation + " * ")
                << m_indentation << " */" << m_endl;
            }
            if (!at->getInitialValue().isEmpty())
            {
                js << m_indentation << "this.m_" << cleanName(at->getName()) << " = " << at->getInitialValue() << ";" << m_endl;
            }
            else
            {
                js << m_indentation << "this.m_" << cleanName(at->getName()) << " = \"\";" << m_endl;
            }
        }
    }

    //associations
    UMLAssociationList aggregations = c->getAggregations();
    if (forceSections() || !aggregations.isEmpty())
    {
        js << m_endl << m_indentation << "/**Aggregations: */" << m_endl;
        writeAssociation(classname, aggregations , js );

    }
    UMLAssociationList compositions = c->getCompositions();
    if (forceSections() || !compositions.isEmpty())
    {
        js << m_endl << m_indentation << "/**Compositions: */" << m_endl;
        writeAssociation(classname, compositions , js );

    }
    js << m_endl;
    js << "}" << m_endl;
    js << m_endl;

    //operations
    UMLOperationList ops(c->getOpList());
    writeOperations(classname, &ops, js);

    js << m_endl;

    //finish file

    //close files and notfiy we are done
    filejs.close();
    emit codeGenerated(c, true);
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void JSWriter::writeAssociation(QString& classname, UMLAssociationList& assocList , QTextStream &js)
{
    foreach (UMLAssociation *a , assocList ) {
        // association side
        Uml::Role_Type role = (a->getObject(Uml::A)->getName() == classname ? Uml::B : Uml::A);

        QString roleName(cleanName(a->getRoleName(role)));

        if (!roleName.isEmpty()) {

            // association doc
            if (forceDoc() || !a->getDoc().isEmpty())
            {
                js << m_indentation << "/**" << m_endl
                   << formatDoc(a->getDoc(), m_indentation + " * ")
                   << m_indentation << " */" << m_endl;
            }

            // role doc
            if (forceDoc() || !a->getRoleDoc(role).isEmpty())
            {
                js << m_indentation << "/**" << m_endl
                   << formatDoc(a->getRoleDoc(role), m_indentation + " * ")
                   << m_indentation << " */" << m_endl;
            }

            bool okCvt;
            int nMulti = a->getMulti(role).toInt(&okCvt,10);
            bool isNotMulti = a->getMulti(role).isEmpty() || (okCvt && nMulti == 1);

            QString typeName(cleanName(a->getObject(role)->getName()));

            if (isNotMulti)
                js << m_indentation << "this.m_" << roleName << " = new " << typeName << "();" << m_endl;
            else
                js << m_indentation << "this.m_" << roleName << " = new Array();" << m_endl;

            // role visibility
        }
    }
}

void JSWriter::writeOperations(QString classname, UMLOperationList *opList, QTextStream &js)
{
    foreach (UMLOperation* op ,  *opList )
    {
        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->getDoc().isEmpty();
        foreach (UMLAttribute* at , atl )
            writeDoc |= !at->getDoc().isEmpty();

        if( writeDoc )  //write method documentation
        {
            js << "/**" << m_endl << formatDoc(op->getDoc()," * ");

            foreach (UMLAttribute* at , atl )  //write parameter documentation
            {
                if(forceDoc() || !at->getDoc().isEmpty())
                {
                    js << " * @param " + cleanName(at->getName())<<m_endl;
                    js << formatDoc(at->getDoc(),"    *      ");
                }
            }//end for : write parameter documentation
            js << " */" << m_endl;
        }//end if : write method documentation

        js << classname << ".prototype." << cleanName(op->getName()) << " = function " << "(";

        int i = atl.count();
        int j=0;
        foreach (UMLAttribute* at , atl ) {
            js << cleanName(at->getName())
            << (!(at->getInitialValue().isEmpty()) ? (QString(" = ")+at->getInitialValue()) : QString(""))
            << ((j < i-1)?", ":"");
            j++;
        }
        js << ")" << m_endl << "{" << m_endl;
        QString sourceCode = op->getSourceCode();
        if (sourceCode.isEmpty()) {
            js << m_indentation << m_endl;
        }
        else {
            js << formatSourceCode(sourceCode, m_indentation);
        }
        js << "}" << m_endl;
        js << m_endl << m_endl;
    }//end for
}

Uml::Programming_Language JSWriter::getLanguage()
{
    return Uml::pl_JavaScript;
}

const QStringList JSWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "break"
        << "case"
        << "const"
        << "continue"
        << "default"
        << "else"
        << "false"
        << "for"
        << "function"
        << "if"
        << "in"
        << "new"
        << "return"
        << "switch"
        << "this"
        << "true"
        << "var"
        << "while"
        << "with";
    }

    return keywords;
}

#include "jswriter.moc"
