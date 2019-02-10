/*
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <stdio.h>

#include "AddGroup.h"

#include <QCommandLineParser>
#include <QTextStream>

#include "cli/Utils.h"
#include "core/Database.h"
#include "core/Entry.h"
#include "core/Group.h"

AddGroup::AddGroup()
{
    name = QString("add_group");
    description = QObject::tr("Add a new group to a database.");
}

AddGroup::~AddGroup()
{
}

int AddGroup::execute(const QStringList& arguments)
{

    QTextStream inputTextStream(stdin, QIODevice::ReadOnly);
    QTextStream outputTextStream(stdout, QIODevice::WriteOnly);

    QCommandLineParser parser;
    parser.setApplicationDescription(this->description);
    parser.addPositionalArgument("database", QObject::tr("Path of the database."));

    QCommandLineOption keyFile(QStringList() << "k"
                                             << "key-file",
                               QObject::tr("Key file of the database."),
                               QObject::tr("path"));
    parser.addOption(keyFile);

    parser.addPositionalArgument("group", QObject::tr("Path of the group to add."));
    parser.process(arguments);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 2) {
        outputTextStream << parser.helpText().replace("keepassxc-cli", "keepassxc-cli add");
        return EXIT_FAILURE;
    }

    QString databasePath = args.at(0);
    QString groupPath = args.at(1);

    Database* db = Database::unlockFromStdin(databasePath, parser.value(keyFile));
    if (db == nullptr) {
        return EXIT_FAILURE;
    }

    Group* group = db->rootGroup()->addGroupWithPath(groupPath);
    if (!group) {
        qCritical("Could not create group with path %s.", qPrintable(groupPath));
        return EXIT_FAILURE;
    }

    QString errorMessage = db->saveToFile(databasePath);
    if (!errorMessage.isEmpty()) {
        qCritical("Writing the database failed %s.", qPrintable(errorMessage));
        return EXIT_FAILURE;
    }

    outputTextStream << "Successfully added group " << group->name() << "." << endl;
    return EXIT_SUCCESS;
}
