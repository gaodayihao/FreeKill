// SPDX-License-Identifier: GPL-3.0-or-later

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include "shell.h"
#include "packman.h"
#include "server.h"
#include "serverplayer.h"
#include "util.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>

static void sigintHandler(int) {
  fprintf(stderr, "\n");
  rl_reset_line_state();
  rl_replace_line("", 0);
  rl_crlf();
  rl_redisplay();
}

void Shell::helpCommand(QStringList &) {
  qInfo("Frequently used commands:");
#define HELP_MSG(a, b) \
  qInfo((a), Color((b), fkShell::Cyan).toUtf8().constData());

  HELP_MSG("%s: Display this help message.", "help");
  HELP_MSG("%s: Shut down the server.", "quit");
  HELP_MSG("%s: List all online players.", "lsplayer");
  HELP_MSG("%s: List all running rooms.", "lsroom");
  HELP_MSG("%s: Kick a player by his id.", "kick");
  HELP_MSG("%s: Broadcast message.", "msg");
  qInfo();
  qInfo("===== Package commands =====");
  HELP_MSG("%s: Install a new package from <url>.", "install");
  HELP_MSG("%s: Remove a package.", "remove");
  HELP_MSG("%s: List all packages.", "lspkg");
  HELP_MSG("%s: Enable a package.", "enable");
  HELP_MSG("%s: Disable a package.", "disable");
  HELP_MSG("%s: Upgrade a package.", "upgrade");
  qInfo("For more commands, check the documentation.");

#undef HELP_MSG
}

void Shell::lspCommand(QStringList &) {
  if (ServerInstance->players.size() == 0) {
    qInfo("No online player.");
    return;
  }
  qInfo("Current %lld online player(s) are:", ServerInstance->players.size());
  foreach (auto player, ServerInstance->players) {
    qInfo() << player->getId() << "," << player->getScreenName();
  }
}

void Shell::lsrCommand(QStringList &) {
  if (ServerInstance->rooms.size() == 0) {
    qInfo("No running room.");
    return;
  }
  qInfo("Current %lld running rooms are:", ServerInstance->rooms.size());
  foreach (auto room, ServerInstance->rooms) {
    qInfo() << room->getId() << "," << room->getName();
  }
}

void Shell::installCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'install' command need a URL to install.");
    return;
  }

  auto url = list[0];
  Pacman->downloadNewPack(url);
}

void Shell::removeCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'remove' command need a package name to remove.");
    return;
  }

  auto pack = list[0];
  Pacman->removePack(pack);
}

void Shell::upgradeCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'upgrade' command need a package name to upgrade.");
    return;
  }

  auto pack = list[0];
  Pacman->upgradePack(pack);
}

void Shell::enableCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'enable' command need a package name to enable.");
    return;
  }

  auto pack = list[0];
  Pacman->enablePack(pack);
}

void Shell::disableCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'disable' command need a package name to disable.");
    return;
  }

  auto pack = list[0];
  Pacman->disablePack(pack);
}

void Shell::lspkgCommand(QStringList &) {
  auto arr = QJsonDocument::fromJson(Pacman->listPackages().toUtf8()).array();
  qInfo("Name\tVersion\t\tEnabled");
  qInfo("------------------------------");
  foreach (auto a, arr) {
    auto obj = a.toObject();
    auto hash = obj["hash"].toString();
    qInfo() << obj["name"].toString().toUtf8().constData() << "\t"
            << hash.first(8).toUtf8().constData() << "\t"
            << obj["enabled"].toString().toUtf8().constData();
  }
}

void Shell::kickCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'kick' command needs a player id.");
    return;
  }

  auto pid = list[0];
  bool ok;
  int id = pid.toInt(&ok);
  if (!ok)
    return;

  auto p = ServerInstance->findPlayer(id);
  if (p) {
    p->kicked();
  }
}

void Shell::msgCommand(QStringList &list) {
  if (list.isEmpty()) {
    qWarning("The 'msg' command needs message body.");
    return;
  }

  auto msg = list.join(' ');
  ServerInstance->broadcast("ServerMessage", msg);
}

Shell::Shell() {
  setObjectName("Shell");
  signal(SIGINT, sigintHandler);

  static QHash<QString, void (Shell::*)(QStringList &)> handlers;
  if (handlers.size() == 0) {
    handlers["help"] = &Shell::helpCommand;
    handlers["?"] = &Shell::helpCommand;
    handlers["lsplayer"] = &Shell::lspCommand;
    handlers["lsroom"] = &Shell::lsrCommand;
    handlers["install"] = &Shell::installCommand;
    handlers["remove"] = &Shell::removeCommand;
    handlers["upgrade"] = &Shell::upgradeCommand;
    handlers["lspkg"] = &Shell::lspkgCommand;
    handlers["enable"] = &Shell::enableCommand;
    handlers["disable"] = &Shell::disableCommand;
    handlers["kick"] = &Shell::kickCommand;
    handlers["msg"] = &Shell::msgCommand;
  }
  handler_map = handlers;
}

void Shell::run() {
  printf("\rFreeKill, Copyright (C) 2022-2023, GNU GPL'd, by Notify et al.\n");
  printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
  printf(
      "This is free software, and you are welcome to redistribute it under\n");
  printf("certain conditions; For more information visit "
         "http://www.gnu.org/licenses.\n\n");
  printf("This is server cli. Enter \"help\" for usage hints.\n");

  while (true) {
    char *bytes = readline("fk> ");
    if (!bytes || !strcmp(bytes, "quit")) {
      qInfo("Server is shutting down.");
      qApp->quit();
      return;
    }

    if (*bytes)
      add_history(bytes);

    auto command = QString(bytes);
    auto command_list = command.split(' ');
    auto func = handler_map[command_list.first()];
    if (!func) {
      auto bytes = command_list.first().toUtf8();
      qWarning("Unknown command \"%s\". Type \"help\" for hints.",
               bytes.constData());
    } else {
      command_list.removeFirst();
      (this->*func)(command_list);
    }

    free(bytes);
  }
}

#endif
